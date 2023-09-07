/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.1
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "CoherentMesh.H"
#include "sliceMeshHelper.H"
#include "nonblockConsensus.H"

#include "processorPolyPatch.H"

#include "DataComponent.H"
#include "OffsetStrategies.H"
#include "FieldComponent.H"
#include "SliceDecorator.H"

#include <numeric>
#include <cmath>
#include <functional> // std::bind, std::placeholders

#include <chrono>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(Foam::CoherentMesh, 0);

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::CoherentMesh::readMesh(const fileName& pathname)
{
    using InitStrategyPtr = std::unique_ptr<InitStrategy>;
    using InitIndexComp = InitFromADIOS<labelList>;
    using PartitionIndexComp = NaivePartitioningFromADIOS<labelList>;

    IndexComponent coherenceTree{};
    if (Pstream::parRun())
    {
        std::unique_ptr<InitIndexComp> init_partitionStarts
        (
            new InitIndexComp("mesh", pathname, "partitionStarts")
        );
        if (init_partitionStarts->size() == Pstream::nProcs()+1)
        {
            coherenceTree.add
            (
                "mesh",
                "partitionStarts",
                std::move(init_partitionStarts),
                Foam::start_from_myProcNo,
                Foam::count_two
            );

            InitStrategyPtr init_ownerStarts
            (
                new InitIndexComp("mesh", pathname, "ownerStarts")
            );
            coherenceTree.node("partitionStarts")->add
            (
                "mesh",
                "ownerStarts",
                std::move(init_ownerStarts),
                Foam::start_from_front,
                Foam::count_from_front_plus_one
            );
        }
        else
        {
            InitStrategyPtr init_ownerStarts
            (
                new PartitionIndexComp("mesh", pathname, "ownerStarts")
            );
            coherenceTree.add("mesh", "ownerStarts", std::move(init_ownerStarts));
        }

    }
    else
    {
        InitStrategyPtr init_ownerStarts
        (
            new InitIndexComp("mesh", pathname, "ownerStarts")
        );
        coherenceTree.add("mesh", "ownerStarts", std::move(init_ownerStarts));
    }

    InitStrategyPtr init_cellOffsets(new InitOffsets(true));
    coherenceTree.node("ownerStarts")->add
    (
        "offsets",
        "cellOffsets",
        std::move(init_cellOffsets),
        nullptr,
        Foam::offset_by_size_minus_one
    );
    coherenceTree.decorate<Foam::SliceDecorator>("cellOffsets");

    InitStrategyPtr init_neighbours
    (
        new InitIndexComp("mesh", pathname, "neighbours")
    );
    coherenceTree.node("ownerStarts")->add
    (
        "mesh",
        "neighbours",
        std::move(init_neighbours),
        Foam::start_from_front,
        Foam::count_from_front
    );

    InitStrategyPtr init_faceOffsets(new InitOffsets(true));
    coherenceTree.node("neighbours")->add
    (
        "offsets",
        "faceOffsets",
        std::move(init_faceOffsets),
        nullptr,
        Foam::count_from_size
    );

    InitStrategyPtr init_internalFaceOffsets(new InitOffsets(true));
    coherenceTree.node("neighbours")->add
    (
        "offsets",
        "internalFaceOffsets",
        std::move(init_internalFaceOffsets),
        nullptr,
        Foam::count_geq(0)
    );

    InitStrategyPtr init_faceStarts
    (
        new InitIndexComp("mesh", pathname, "faceStarts")
    );
    coherenceTree.node("ownerStarts")->add
    (
        "mesh",
        "faceStarts",
        std::move(init_faceStarts),
        Foam::start_from_front,
        Foam::count_from_front_plus_one
    );

    InitStrategyPtr init_faces
    (
        new InitIndexComp("mesh", pathname, "faces")
    );
    coherenceTree.node("faceStarts")->add
    (
        "mesh",
        "faces",
        std::move(init_faces),
        Foam::start_from_front,
        Foam::count_from_front
    );

    InitStrategyPtr init_pointOffsets(new InitOffsets(false));
    coherenceTree.node("faces")->add
    (
        "offsets",
        "pointOffsets",
        std::move(init_pointOffsets),
        Foam::offset_by_max_plus_one,
        nullptr
    );
    coherenceTree.decorate<Foam::SliceDecorator>("pointOffsets");

    InitStrategyPtr init_points
    (
        new InitPrimitivesFromADIOS<pointField>("mesh", pathname, "points")
    );
    coherenceTree.node("pointOffsets")->add<FieldComponent<pointField>>
    (
        "mesh",
        "points",
        std::move(init_points),
        Foam::start_from_front,
        Foam::count_from_front
    );

    coherenceTree.initialize();

    std::vector<Foam::label> ownerStarts;
    coherenceTree.node("ownerStarts")->extract(ownerStarts);
    serializeOwner(ownerStarts); // TODO: has side-effects: filling member localOwner_
    ownerStarts.clear();

    std::vector<Foam::label> faceStarts;
    std::vector<Foam::label> linearizedFaces;
    coherenceTree.node("faceStarts")->extract(faceStarts);
    coherenceTree.node("faces")->extract(linearizedFaces);
    deserializeFaces(faceStarts, linearizedFaces); // TODO: has side-effects: filling member globalFaces_
    linearizedFaces.clear();
    faceStarts.clear();

    coherenceTree.node("points")->extract(allPoints_);

    numBoundaries_ = *std::min_element
                      (
                          coherenceTree.node("neighbours")->begin(),
                          coherenceTree.node("neighbours")->end()
                      );
    numBoundaries_ = Foam::decodeSlicePatchId( numBoundaries_ ) + 1;
    Foam::reduce(numBoundaries_, maxOp<label>());
    boundaryGlobalIndex_.resize(numBoundaries_);

    for (Foam::label patchi = 0; patchi<numBoundaries_; ++patchi)
    {
        auto slicePatchId = Foam::encodeSlicePatchId(patchi);
        InitStrategyPtr init_boundaryFaceOffsets(new InitOffsets(true));
        coherenceTree.node("neighbours")->add
        (
            "offsets",
            "boundaryFaceOffsets" + std::to_string(patchi),
            std::move(init_boundaryFaceOffsets),
            nullptr,
            Foam::count_eq(slicePatchId)
        );
    }

    coherenceTree.node("neighbours")->initialize();
    coherenceTree.node("neighbours")->extract(globalNeighbours_);

    coherenceTree.node("cellOffsets")->extract(cellOffsets_);
    coherenceTree.node("faceOffsets")->extract(faceOffsets_);
    coherenceTree.node("internalFaceOffsets")
        ->extract(internalSurfaceFieldOffsets_);
    for (Foam::label patchi = 0; patchi<numBoundaries_; ++patchi)
    {
        Foam::Offsets patchOffsets;
        coherenceTree.node("boundaryFaceOffsets" + std::to_string(patchi))
                 ->extract(patchOffsets);
        boundarySurfacePatchOffsets_.push_back(patchOffsets);
    }
    coherenceTree.node("pointOffsets")->extract(pointOffsets_);
    coherenceTree.node("cellOffsets")->extract(cellSlice_);
    coherenceTree.node("pointOffsets")->extract(pointSlice_);

    if (Pstream::parRun())
    {
        initializeSurfaceFieldMappings();
    auto start = std::chrono::steady_clock::now();
        commSlicePatches();
        commSharedPoints();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    double elapsed_time = static_cast<double>(elapsed_seconds.count());
    Foam::reduce(elapsed_time, maxOp<double>());
    Info << "elapsed time: " << elapsed_time << "s\n";
        renumberFaces();
    }

    splintedPermutation_ = FragmentPermutation(globalNeighbours_);
}


void Foam::CoherentMesh::sendSliceFaces
(
    std::pair<Foam::label, Foam::label> sendPair
)
{
    Foam::label myProcNo = Pstream::myProcNo();
    Foam::label partition = sendPair.first;
    Foam::OPstream toPartition(Pstream::blocking, partition, 0, 0);

    Foam::Slice slice(partition, cellOffsets_);
    Foam::ProcessorPatch procPatch(slice, globalNeighbours_, numBoundaries_);

    // Face Stuff
    auto sendFaces = procPatch.extractFaces(globalFaces_);
    toPartition << sendFaces;
    procPatch.determinePointIDs(sendFaces, pointOffsets_.lowerBound(myProcNo));
    sendFaces.clear();

    // Owner Stuff
    auto sendNeighbours = procPatch.extractFaces(globalNeighbours_);
    toPartition << sendNeighbours;
    sendNeighbours.clear();

    // Point Stuff
    auto sendPoints = procPatch.extractPoints(allPoints_);
    toPartition << sendPoints;
    sendPoints.clear();

    // procBoundary Stuff: Track face swapping indices for processor boundaries;
    procPatch.encodePatch(globalNeighbours_);
    slicePatches_.push_back(procPatch);
}


void Foam::CoherentMesh::recvSliceFaces
(
    std::pair<Foam::label, Foam::label> recvPair
)
{
    label partition = recvPair.first;
    label numberOfPartitionFaces = recvPair.second;
    IPstream fromPartition(Pstream::blocking, partition, 0, 0);

    // Face Communication
    auto oldNumFaces = globalFaces_.size();
    globalFaces_.resize(oldNumFaces + numberOfPartitionFaces);
    SubList<face> recvFaces(globalFaces_, numberOfPartitionFaces, oldNumFaces);
    fromPartition >> recvFaces;
    std::transform
    (
        recvFaces.begin(),
        recvFaces.end(),
        recvFaces.begin(),
        [](Foam::face& input)
        {
            return input.reverseFace();
        }
    );

    // Owner Communication
    localOwner_.resize(oldNumFaces + numberOfPartitionFaces);
    SubList<label> recvOwner(localOwner_, numberOfPartitionFaces, oldNumFaces);
    fromPartition >> recvOwner;
    std::transform
    (
        recvOwner.begin(),
        recvOwner.end(),
        recvOwner.begin(),
        [this](const Foam::label& id)
        {
            return cellSlice_.convert(id);
        }
    );

    // Identify points from slice/partition that associate with the received faces
    Foam::Slice recvPointSlice(partition, pointOffsets_);

    std::set<Foam::label> pointIDs{};
    Foam::subset
    (
        globalFaces_.end() - numberOfPartitionFaces,
        globalFaces_.end(),
        std::inserter(pointIDs, pointIDs.end()),
        recvPointSlice
    );
    // Append new point IDs to point mapping
    // TODO: Create proper state behaviour in Slice
    pointSlice_.append(pointIDs);

    // Point Communication
    auto oldNumPoints = allPoints_.size();
    allPoints_.resize(oldNumPoints + pointIDs.size());
    SubField<point> recvPoints(allPoints_, pointIDs.size(), oldNumPoints);
    fromPartition >> recvPoints;

    // ProcBoundary Stuff
    Foam::Slice recvCellSlice(partition, cellOffsets_);
    ProcessorPatch procPatch(recvCellSlice, globalNeighbours_, numBoundaries_);
    procPatch.encodePatch(globalNeighbours_, numberOfPartitionFaces);
    slicePatches_.push_back(procPatch);
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::CoherentMesh::deserializeFaces
(
    const std::vector<Foam::label>& faceStarts,
    const std::vector<Foam::label>& linearizedFaces
)
{
    globalFaces_.setSize(faceStarts.size() - 1);
    for (size_t i = 0; i<globalFaces_.size(); ++i)
    {
        globalFaces_[i].resize(faceStarts[i + 1] - faceStarts[i]);
        std::move
        (
            linearizedFaces.begin() + (faceStarts[i] - faceStarts[0]),
            linearizedFaces.begin() + (faceStarts[i + 1] - faceStarts[0]),
            globalFaces_[i].begin()
        );
    }
}


void Foam::CoherentMesh::serializeOwner
(
    const std::vector<Foam::label>& ownerStarts
)
{
    Foam::label ownerCellID = 0;
    localOwner_.setSize(ownerStarts.back() - ownerStarts.front());
    for (size_t i = 0; i<ownerStarts.size() - 1; ++i)
    {
        std::fill
        (
            localOwner_.begin() + ownerStarts[i] - ownerStarts.front(),
            localOwner_.begin() + ownerStarts[i + 1] - ownerStarts.front(),
            ownerCellID
        );
        ++ownerCellID;
    }
}


void Foam::CoherentMesh::commSlicePatches()
{
    Foam::label myProcNo = Pstream::myProcNo();
    Foam::label nProcs = Pstream::nProcs();
    //TODO : move to tree (decoration)
    std::vector<Slice> slices(nProcs - (myProcNo+1));
    Foam::label partition = myProcNo;
    std::transform
    (
        cellOffsets_.begin() + myProcNo + 1,
        cellOffsets_.end(),
        std::back_inserter(slices),
        [&partition](const std::pair<Foam::label, Foam::label> offset) mutable
        {
            ++partition;
            return Foam::Slice(partition, offset.first, offset.second);
        }
    );

    std::map<Foam::label, Foam::label> sendNumPartitionFaces{};
    for (auto& slice : slices)
    {
        std::vector<Foam::label> sharedIDs{};
        Foam::subset
        (
            globalNeighbours_.begin(),
            globalNeighbours_.end(),
            std::back_inserter(sharedIDs),
            slice
        );
        if (!sharedIDs.empty())
        {
            sendNumPartitionFaces[slice.partition()] = sharedIDs.size();
        }
    }

    auto recvNumPartitionFaces = Foam::nonblockConsensus(sendNumPartitionFaces);

    slicePatches_.clear();
    for (const auto& sendPair: sendNumPartitionFaces)
    {
        sendSliceFaces(sendPair);
    }

    for (const auto& recvPair: recvNumPartitionFaces)
    {
        recvSliceFaces(recvPair);
    }
}


void Foam::CoherentMesh::commSharedPoints()
{
    auto myProcNo = Pstream::myProcNo();
    std::set<label> missingPointIDs{};
    Foam::subset
    (
        globalFaces_.begin(),
        globalFaces_.end(),
        std::inserter(missingPointIDs, missingPointIDs.end()),
        [this] (const Foam::label& id)
        {
            return !pointSlice_.exist(id);
        }
    );

    //TODO : move to tree (decoration)
    std::vector<Slice> slices(myProcNo);
    Foam::label partition = -1;
    std::transform
    (
        pointOffsets_.begin(),
        pointOffsets_.begin() + myProcNo,
        std::back_inserter(slices),
        [&partition](const std::pair<Foam::label, Foam::label> offset) mutable
        {
            ++partition;
            return Foam::Slice(partition, offset.first, offset.second);
        }
    );

    std::map<Foam::label, std::vector<Foam::label>> sendPointIDs{};
    for (auto& slice : slices)
    {
        std::vector<Foam::label> sharedIDs{};
        Foam::subset
        (
            missingPointIDs.begin(),
            missingPointIDs.end(),
            std::back_inserter(sharedIDs),
            slice
        );
        if (!sharedIDs.empty())
        {
            sendPointIDs[slice.partition()] = sharedIDs;
        }
    }

    auto recvPointIDs = Foam::nonblockConsensus(sendPointIDs, MPI_LONG);

    for (const auto& commPair: recvPointIDs)
    {
        auto partition = commPair.first;
        auto sharedPoints = commPair.second;
        pointSlice_.convert(sharedPoints);
        pointField pointBuf = Foam::extractor(allPoints_, sharedPoints);
        OPstream::write
        (
            Pstream::blocking,
            partition,
            reinterpret_cast<const char*>(pointBuf.data()),
            pointBuf.size() * 3 * sizeof(scalar)
        );
    }

    for (const auto& commPair: sendPointIDs)
    {
        auto partition = commPair.first;
        auto sharedPoints = commPair.second;
        label count = sharedPoints.size();
        pointField pointBuf;
        pointBuf.resize(count);
        IPstream::read
        (
            Pstream::blocking,
            partition,
            reinterpret_cast<char*>( pointBuf.data()),
            count * 3 * sizeof(scalar)
        );
        for (const auto& point: pointBuf)
        {
            allPoints_.append(point);
        }
        pointSlice_.append(sharedPoints);
    }
}


void Foam::CoherentMesh::renumberFaces()
{
    for (auto& face: globalFaces_)
    {
        std::transform
        (
            face.begin(),
            face.end(),
            face.begin(),
            [this](const Foam::label& id)
            {
                return pointSlice_.convert(id);
            }
        );
    }
}


void Foam::CoherentMesh::initializeSurfaceFieldMappings()
{
    // permute before processor boundaries have been identified
    auto permutation = FragmentPermutation(globalNeighbours_);
    Foam::labelList neighbours{};
    permutation.retrieveNeighbours(neighbours);

    // Identify neighbouring processor and number of shared faces
    Foam::label myProcNo = Pstream::myProcNo();
    Foam::label nProcs = Pstream::nProcs();
    // TODO : move to tree (decoration)
    std::vector<Slice> slices(nProcs - (myProcNo+1));
    Foam::label partition = myProcNo;
    std::transform
    (
        cellOffsets_.begin() + myProcNo + 1,
        cellOffsets_.end(),
        std::back_inserter(slices),
        [&partition](const std::pair<Foam::label, Foam::label> offset) mutable
        {
            ++partition;
            return Foam::Slice(partition, offset.first, offset.second);
        }
    );

    std::map<Foam::label, Foam::label> procPatchIDsAndSizes{};
    for (auto& slice : slices)
    {
        std::vector<Foam::label> sharedIDs{};
        Foam::subset
        (
            neighbours.begin(),
            neighbours.end(),
            std::back_inserter(sharedIDs),
            slice
        );
        if (!sharedIDs.empty())
        {
            procPatchIDsAndSizes[slice.partition()] = sharedIDs.size();
        }
    }

    using pair_type = std::pair<Foam::label, Foam::label>;
    Foam::label totalSize = std::accumulate
                            (
                                procPatchIDsAndSizes.begin(),
                                procPatchIDsAndSizes.end(),
                                0,
                                [](label size, pair_type pair)
                                {
                                    return std::move(size) + pair.second;
                                }
                            );
    // Resize container for internal face and boundary indices
    internalFaceIDs_.resize(totalSize);
    procBoundaryIDs_.resize(totalSize);

    // Loop and fill internal face and boundary indices
    auto internalFaceIDsIter = internalFaceIDs_.begin();
    auto procBoundaryIDsIter = procBoundaryIDs_.begin();
    slicePatches_.reserve(procPatchIDsAndSizes.size());
    for (const auto& procPatchIdAndSize: procPatchIDsAndSizes)
    {
        // Identify processor boundaries in neighbour list
        Foam::Slice slice(procPatchIdAndSize.first, cellOffsets_);
        Foam::ProcessorPatch owningProcPatch(slice, neighbours, numBoundaries_);
        slicePatches_.push_back(owningProcPatch);
        // Copy the index to internal face list
        std::copy
        (
            owningProcPatch.begin(),
            owningProcPatch.end(),
            internalFaceIDsIter
        );
        internalFaceIDsIter += owningProcPatch.size();
        // Copy the boundary index to the corresponding faces
        std::fill_n
        (
            procBoundaryIDsIter,
            owningProcPatch.size(),
            decodeSlicePatchId(owningProcPatch.id())
        );
        procBoundaryIDsIter += owningProcPatch.size();
    }
    // Sort the face indices to the internal list and
    // permute the processor boundary indices accordingly
    auto sortedPermutation = permutationOfSorted(internalFaceIDs_);
    applyPermutation(internalFaceIDs_, sortedPermutation);
    applyPermutation(procBoundaryIDs_, sortedPermutation);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::CoherentMesh::CoherentMesh(const Foam::polyMesh& pm)
:
    MeshObject<polyMesh, CoherentMesh>(pm)
{
    Foam::fileName path = pm.pointsInstance()/pm.meshDir();
    readMesh(path);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::CoherentMesh::polyNeighbours(Foam::labelList& neighbours)
{
    splintedPermutation_.retrieveNeighbours(neighbours);
    cellSlice_.convert(neighbours);
}


void Foam::CoherentMesh::polyOwner(Foam::labelList& owner)
{
    owner = localOwner_;
    splintedPermutation_.permute(owner);
}


void Foam::CoherentMesh::polyFaces(Foam::faceList& faces)
{
    faces = globalFaces_;
    splintedPermutation_.permute(faces);
}


void Foam::CoherentMesh::polyPoints(Foam::pointField& points)
{
    points = allPoints_;
}


std::vector<Foam::label> Foam::CoherentMesh::polyPatches()
{
    std::vector<label> patches{};
    splintedPermutation_.retrievePatches(patches);
    return patches;
}


std::vector<Foam::ProcessorPatch> Foam::CoherentMesh::procPatches()
{
    return slicePatches_;
}


Foam::List<Foam::polyPatch*>
Foam::CoherentMesh::polyPatches(polyBoundaryMesh& boundary)
{
    // Retrieve list of patch IDs for boundary patch faces
    std::vector<label> patches = polyPatches();
    Foam::label numInternalFaces = globalNeighbours_.size() - patches.size();

    // Read (on master) and distribute ASCII entries for boundaries
    PtrList<entry> patchEntries{};
    if (Pstream::master())
    {
        Istream& is = boundary.readStream("polyBoundaryMesh");
        patchEntries = PtrList<entry>(is);
    }
    Pstream::scatter(patchEntries);

    // Start filling modelled/physical boundary patches
    Foam::List<Foam::polyPatch*> boundaryPatches
    (
        patchEntries.size() +
        slicePatches_.size(),
        reinterpret_cast<Foam::polyPatch*>(0)
    );
    Foam::label nthPatch = 0;
    Foam::label nextPatchStart = numInternalFaces;
    for (label patchi = 0; patchi<patchEntries.size(); ++patchi)
    {
        // Determine local start and size of boundary patches
        Foam::label patchStart{nextPatchStart};
        Foam::label patchSize{0};
        Foam::label slicePatchId = Foam::encodeSlicePatchId(patchi);
        auto patchStartIt =
            std::find(patches.begin(), patches.end(), slicePatchId);
        if (patchStartIt != patches.end())
        {
            patchStart =
                numInternalFaces + std::distance(patches.begin(), patchStartIt);
            patchSize = std::count(patchStartIt, patches.end(), slicePatchId);
        }

        // Replace new values into patch entries and create the polyPatch.
        patchEntries[nthPatch].dict().set("startFace", patchStart);
        patchEntries[nthPatch].dict().set("nFaces", patchSize);
        boundaryPatches[nthPatch] =
            Foam::polyPatch::New
            (
                patchEntries[nthPatch].keyword(),
                patchEntries[nthPatch].dict(),
                nthPatch,
                boundary
            ).ptr();

        nextPatchStart = patchStart + patchSize;
        ++nthPatch;
    }

    if (Pstream::parRun())
    {
        // Fill processor boundary patches
        for (label patchi = 0; patchi<slicePatches_.size(); ++patchi)
        {
            // Determine start and size of processor boundary patch
            Foam::label ProcessorPatchId = slicePatches_[patchi].id();
            auto patchStartIt =
                std::find(patches.begin(), patches.end(), ProcessorPatchId);
            Foam::label patchStart =
                numInternalFaces + std::distance(patches.begin(), patchStartIt);
            Foam::label patchSize =
                std::count(patchStartIt, patches.end(), ProcessorPatchId);

            // Create and store the processorPolyPatch
            boundaryPatches[nthPatch] =
                new Foam::processorPolyPatch
                (
                    slicePatches_[patchi].name(),
                    patchSize,
                    patchStart,
                    nthPatch,
                    boundary,
                    Pstream::myProcNo(),
                    slicePatches_[patchi].partner()
                );

            ++nthPatch;
        }
    }

    return boundaryPatches;
}


const Foam::globalIndex&
Foam::CoherentMesh::boundaryGlobalIndex(label patchId)
{
    if (!boundaryGlobalIndex_.set(patchId))
    {
        boundaryGlobalIndex_.set
                             (
                                 patchId,
                                 new globalIndex
                                 (
                                     mesh().boundaryMesh()[patchId].size()
                                 )
                             );
    }

    return boundaryGlobalIndex_[patchId];
}

// ************************************************************************* //
