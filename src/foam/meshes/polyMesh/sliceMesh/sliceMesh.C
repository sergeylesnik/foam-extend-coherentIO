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

#include "sliceMesh.H"
#include "sliceMeshHelper.H"
#include "nonblockConsensus.H"

#include "processorPolyPatch.H"

#include "DataComponent.H"
#include "OffsetStrategies.H"
#include "IndexComponent.H"
#include "FieldComponent.H"
#include "SliceDecorator.H"

#include <numeric>
#include <cmath>
#include <functional> // std::bind, std::placeholders


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(Foam::sliceMesh, 0);

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::sliceMesh::readMesh(const fileName& pathname)
{
    using InitStrategyPtr = std::unique_ptr<InitStrategy>;
    using InitIndexComp = InitFromADIOS<labelList>;
    using PartitionIndexComp = NaivePartitioningFromADIOS<labelList>;

    Foam::IndexComponent meshSlice{};
    if (Pstream::parRun())
    {
        std::unique_ptr<InitIndexComp> init_partitionStarts
        (
            new InitIndexComp("mesh", pathname, "partitionStarts")
        );
        if (init_partitionStarts->size() == Pstream::nProcs()+1)
        {
            meshSlice.add
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
            meshSlice.node("partitionStarts")->add
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
            meshSlice.add("mesh", "ownerStarts", std::move(init_ownerStarts));
        }

    }
    else
    {
        InitStrategyPtr init_ownerStarts
        (
            new InitIndexComp("mesh", pathname, "ownerStarts")
        );
        meshSlice.add("mesh", "ownerStarts", std::move(init_ownerStarts));
    }

    InitStrategyPtr init_cellOffsets(new InitOffsets(true));
    meshSlice.node("ownerStarts")->add
    (
        "offsets",
        "cellOffsets",
        std::move(init_cellOffsets),
        nullptr,
        Foam::offset_by_size_minus_one
    );
    meshSlice.decorate<Foam::SliceDecorator>("cellOffsets");

    InitStrategyPtr init_neighbours
    (
        new InitIndexComp("mesh", pathname, "neighbours")
    );
    meshSlice.node("ownerStarts")->add
    (
        "mesh",
        "neighbours",
        std::move(init_neighbours),
        Foam::start_from_front,
        Foam::count_from_front
    );

    InitStrategyPtr init_faceOffsets(new InitOffsets(true));
    meshSlice.node("neighbours")->add
    (
        "offsets",
        "faceOffsets",
        std::move(init_faceOffsets),
        nullptr,
        Foam::count_from_size
    );

    InitStrategyPtr init_internalFaceOffsets(new InitOffsets(true));
    meshSlice.node("neighbours")->add
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
    meshSlice.node("ownerStarts")->add
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
    meshSlice.node("faceStarts")->add
    (
        "mesh",
        "faces",
        std::move(init_faces),
        Foam::start_from_front,
        Foam::count_from_front
    );

    InitStrategyPtr init_pointOffsets(new InitOffsets(false));
    meshSlice.node("faces")->add
    (
        "offsets",
        "pointOffsets",
        std::move(init_pointOffsets),
        Foam::offset_by_max_plus_one,
        nullptr
    );
    meshSlice.decorate<Foam::SliceDecorator>("pointOffsets");

    InitStrategyPtr init_points
    (
        new InitPrimitivesFromADIOS<pointField>("mesh", pathname, "points")
    );
    meshSlice.node("pointOffsets")->add<FieldComponent<pointField>>
    (
        "mesh",
        "points",
        std::move(init_points),
        Foam::start_from_front,
        Foam::count_from_front
    );

    meshSlice.initialize();

    std::vector<Foam::label> ownerStarts;
    std::vector<Foam::label> faceStarts;
    std::vector<Foam::label> linearizedFaces;
    meshSlice.node("ownerStarts")->extract(ownerStarts);
    meshSlice.node("neighbours")->extract(globalNeighbours_);
    meshSlice.node("faceStarts")->extract(faceStarts);
    meshSlice.node("faces")->extract(linearizedFaces);
    meshSlice.node("points")->extract(allPoints_);
    localOwner_ = serializeOwner(ownerStarts);
    globalFaces_ = deserializeFaces(faceStarts, linearizedFaces);

    numBoundaries_ = *std::min_element
                      (
                          globalNeighbours_.begin(),
                          globalNeighbours_.end()
                      );
    numBoundaries_ = Foam::decodeSlicePatchId( numBoundaries_ ) + 1;
    Foam::reduce(numBoundaries_, maxOp<label>());
    boundaryGlobalIndex_.resize(numBoundaries_);

    for (Foam::label patchi = 0; patchi<numBoundaries_; ++patchi)
    {
        auto slicePatchId = Foam::encodeSlicePatchId(patchi);
        InitStrategyPtr init_boundaryFaceOffsets(new InitOffsets(true));
        meshSlice.node("neighbours")
                 ->add
                 (
                     "offsets",
                     "boundaryFaceOffsets" + std::to_string(patchi),
                     std::move(init_boundaryFaceOffsets),
                     nullptr,
                     Foam::count_eq(slicePatchId)
                 );
    }

    meshSlice.initialize();

    meshSlice.node("cellOffsets")->extract(cellOffsets_);
    meshSlice.node("faceOffsets")->extract(faceOffsets_);
    meshSlice.node("internalFaceOffsets")
             ->extract(internalSurfaceFieldOffsets_);
    for (Foam::label patchi = 0; patchi<numBoundaries_; ++patchi)
    {
        Foam::Offsets patchOffsets;
        meshSlice.node("boundaryFaceOffsets" + std::to_string(patchi))
                 ->extract(patchOffsets);
        boundarySurfacePatchOffsets_.push_back(patchOffsets);
    }
    meshSlice.node("pointOffsets")->extract(pointOffsets_);
    meshSlice.node("cellOffsets")->extract(cellSlice_);
    meshSlice.node("pointOffsets")->extract(pointSlice_);

    if (Pstream::parRun())
    {
        initializeSurfaceFieldMappings();
        commSlicePatches();
        commSharedPoints();
        renumberFaces();
    }

    sliceablePermutation_ = slicePermutation(globalNeighbours_);
}


void Foam::sendSliceFaces
(
    std::pair<Foam::label, Foam::label> sendPair,
    Foam::Offsets& cellOffsets,
    Foam::Offsets& pointOffsets,
    Foam::labelList& globalNeighbours,
    Foam::faceList& globalFaces,
    Foam::pointField& allPoints_,
    std::vector<Foam::sliceProcPatch>& sliceProcPatches,
    Foam::label numBoundaries
)
{
    Foam::label myProcNo = Pstream::myProcNo();
    Foam::label partition = sendPair.first;
    Foam::OPstream toPartition(Pstream::blocking, partition, 0, 0);

    Foam::Slice slice(partition, cellOffsets);
    Foam::sliceProcPatch procPatch(slice, globalNeighbours, numBoundaries);

    // Face Stuff
    auto sendFaces = procPatch.extractFaces(globalFaces);
    toPartition << sendFaces;

    // Owner Stuff
    auto sendNeighbours = procPatch.extractFaces(globalNeighbours);
    toPartition << sendNeighbours;

    // Point Stuff
    procPatch.determinePointIDs(sendFaces, pointOffsets.lowerBound(myProcNo));
    auto sendPoints = procPatch.extractPoints(allPoints_);
    toPartition << sendPoints;

    // procBoundary Stuff: Track face swapping indices for processor boundaries;
    procPatch.encodePatch(globalNeighbours);
    sliceProcPatches.push_back(procPatch);
}


void Foam::recvSliceFaces
(
    std::pair<Foam::label, Foam::label> recvPair,
    Foam::faceList& globalFaces,
    Foam::labelList& localOwner,
    Foam::Offsets& cellOffsets,
    Foam::Offsets& pointOffsets,
    Foam::Slice& cellSlice,
    Foam::Slice& pointSlice,
    Foam::pointField& allPoints_,
    std::vector<Foam::sliceProcPatch>& sliceProcPatches,
    Foam::labelList& globalNeighbours,
    Foam::label numBoundaries
)
{
    label partition = recvPair.first;
    label numberOfPartitionFaces = recvPair.second;
    IPstream fromPartition(Pstream::blocking, partition, 0, 0);

    // Face Communication
    faceList recvFaces(numberOfPartitionFaces);
    fromPartition >> recvFaces;
    Foam::appendTransformed
    (
        globalFaces,
        recvFaces,
        [](auto& face)
        {
            return face.reverseFace();
        }
    );

    // Owner Communication
    labelList recvOwner(numberOfPartitionFaces);
    fromPartition >> recvOwner;
    Foam::appendTransformed
    (
        localOwner,
        recvOwner,
        [&cellSlice](const auto& id)
        {
            return cellSlice.convert(id);
        }
    );

    // Identify points from slice/partition that associate with the received faces
    Foam::Slice recvPointSlice(partition, pointOffsets);
    auto pointIDs = Foam::pointSubset(recvFaces, recvPointSlice);
    // Append new point IDs to point mapping
    // TODO: Create proper state behaviour in Slice
    pointSlice.append(pointIDs);

    // Point Communication
    pointField recvPoints(pointIDs.size());
    fromPartition >> recvPoints;
    allPoints_.append(recvPoints);

    // ProcBoundary Stuff
    Foam::Slice recvCellSlice(partition, cellOffsets);
    sliceProcPatch procPatch(recvCellSlice, globalNeighbours, numBoundaries);
    procPatch.encodePatch(globalNeighbours, numberOfPartitionFaces);
    sliceProcPatches.push_back(procPatch);
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

Foam::faceList Foam::sliceMesh::deserializeFaces
(
    const std::vector<Foam::label>& faceStarts,
    const std::vector<Foam::label>& linearizedFaces
)
{
    Foam::faceList globalFaces(faceStarts.size() - 1);
    for (size_t i = 0; i<globalFaces.size(); ++i)
    {
        globalFaces[i].resize(faceStarts[i + 1] - faceStarts[i]);
        std::copy
        (
            linearizedFaces.begin() + (faceStarts[i] - faceStarts[0]),
            linearizedFaces.begin() + (faceStarts[i + 1] - faceStarts[0]),
            globalFaces[i].begin()
        );
    }
    return globalFaces;
}


Foam::labelList
Foam::sliceMesh::serializeOwner(const std::vector<Foam::label>& ownerStarts)
{
    Foam::label ownerCellID = 0;
    Foam::labelList localOwner;
    localOwner.setSize(ownerStarts.back() - ownerStarts.front());
    for (size_t i = 0; i<ownerStarts.size() - 1; ++i)
    {
        std::fill
        (
            localOwner.begin() + ownerStarts[i] - ownerStarts.front(),
            localOwner.begin() + ownerStarts[i + 1] - ownerStarts.front(),
            ownerCellID
        );
        ++ownerCellID;
    }
    return localOwner;
}


void Foam::sliceMesh::commSlicePatches()
{

    auto sendNumPartitionFaces = Foam::numFacesToExchange
                                 (
                                     cellOffsets_,
                                     pointOffsets_,
                                     globalNeighbours_
                                 );
    auto recvNumPartitionFaces = Foam::nonblockConsensus(sendNumPartitionFaces);

    for (const auto& sendPair: sendNumPartitionFaces)
    {
        sendSliceFaces
        (
            sendPair,
            cellOffsets_,
            pointOffsets_,
            globalNeighbours_,
            globalFaces_,
            allPoints_,
            slicePatches_,
            numBoundaries_
        );
    }

    for (const auto& recvPair: recvNumPartitionFaces)
    {
        recvSliceFaces
        (
            recvPair,
            globalFaces_,
            localOwner_,
            cellOffsets_,
            pointOffsets_,
            cellSlice_,
            pointSlice_,
            allPoints_,
            slicePatches_,
            globalNeighbours_,
            numBoundaries_
        );
    }
}


void Foam::sliceMesh::commSharedPoints()
{
    auto myProcNo = Pstream::myProcNo();
    //TODO: Refactor
    std::map<label, std::vector<label>> sendPointIDs{};
    auto missingPointIDs = Foam::missingPoints
                           (
                               globalFaces_,
                               pointSlice_
                           );
    for (label partition = 0; partition<myProcNo; ++partition)
    {
        Foam::Slice recvPointSlice(partition, pointOffsets_);
        auto globallySharedPoints = Foam::pointSubset
                                    (
                                        missingPointIDs,
                                        recvPointSlice
                                    );
        if (!globallySharedPoints.empty())
        {
            std::vector<label> tmp
                               (
                                   globallySharedPoints.begin(),
                                   globallySharedPoints.end()
                               );
            sendPointIDs[partition] = std::move(tmp);
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


void Foam::sliceMesh::renumberFaces()
{
    for (auto& face: globalFaces_)
    {
        std::transform
        (
            face.begin(),
            face.end(),
            face.begin(),
            [this](const auto& id)
            {
                return pointSlice_.convert(id);
            }
        );
    }
}


void Foam::sliceMesh::initializeSurfaceFieldMappings()
{
    // permute before processor boundaries have been identified
    auto permutation = slicePermutation(globalNeighbours_);
    Foam::labelList neighbours{};
    permutation.copyPolyNeighbours(neighbours);
    // Identify neighbouring processor and number of shared faces
    auto procPatchIDsAndSizes = Foam::numFacesToExchange
                                (
                                    cellOffsets_,
                                    pointOffsets_,
                                    neighbours
                                );
    Foam::label totalSize = std::accumulate
                            (
                                procPatchIDsAndSizes.begin(),
                                procPatchIDsAndSizes.end(),
                                0,
                                [](label size, const auto& pair)
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
    std::vector<Foam::sliceProcPatch> tmp{};
    for (const auto& procPatchIdAndSize: procPatchIDsAndSizes)
    {
        // Identify processor boundaries in neighbour list
        Foam::Slice slice(procPatchIdAndSize.first, cellOffsets_);
        Foam::sliceProcPatch owningProcPatch(slice, neighbours, numBoundaries_);
        tmp.push_back(owningProcPatch);
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

Foam::sliceMesh::sliceMesh(const Foam::polyMesh& pm)
:
    MeshObject<polyMesh, sliceMesh>(pm)
{
    Foam::fileName path = pm.pointsInstance()/pm.meshDir();
    readMesh(path);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::labelList Foam::sliceMesh::polyNeighbours()
{
    // TODO: copy poly neighbours into labelList
    std::vector<Foam::label> neighbours{};
    sliceablePermutation_.copyPolyNeighbours(neighbours);
    cellSlice_.convert(neighbours);
    labelList polyNeighbours(neighbours.size());
    std::copy
    (
        std::begin(neighbours),
        std::end(neighbours),
        std::begin(polyNeighbours)
    );
    return polyNeighbours;
}


Foam::labelList Foam::sliceMesh::polyOwner()
{
    auto owner = localOwner_;
    sliceablePermutation_.mapToPoly(owner);
    return owner;
}


Foam::faceList Foam::sliceMesh::polyFaces()
{
    auto faces = globalFaces_;
    sliceablePermutation_.mapToPoly(faces);
    return faces;
}


Foam::pointField Foam::sliceMesh::polyPoints()
{
    return allPoints_;
}


std::vector<Foam::label> Foam::sliceMesh::polyPatches()
{
    std::vector<label> patches{};
    sliceablePermutation_.copyPolyPatches(patches);
    return patches;
}


std::vector<Foam::sliceProcPatch> Foam::sliceMesh::procPatches()
{
    return slicePatches_;
}


Foam::List<Foam::polyPatch*>
Foam::sliceMesh::polyPatches(polyBoundaryMesh& boundary)
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
            Foam::label sliceProcPatchId = slicePatches_[patchi].id();
            auto patchStartIt =
                std::find(patches.begin(), patches.end(), sliceProcPatchId);
            Foam::label patchStart =
                numInternalFaces + std::distance(patches.begin(), patchStartIt);
            Foam::label patchSize =
                std::count(patchStartIt, patches.end(), sliceProcPatchId);

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
Foam::sliceMesh::boundaryGlobalIndex(label patchId)
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
