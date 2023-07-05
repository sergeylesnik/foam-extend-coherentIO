
#include "sliceProcPatch.H"

#include "Offsets.H"
#include "slicePermutation.H"

#include <set>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

Foam::label Foam::sliceProcPatch::instanceCount_ = 0;

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::sliceProcPatch::sliceProcPatch
(
    const Foam::Slice& slice,
    const Foam::labelList& neighbours,
    const Foam::label& numBoundaries
)
:
    numNonProcPatches_{numBoundaries},
    slice_{slice},
    localFaceIDs_
    {
        labelList
        (
            std::count_if
            (
                neighbours.begin(),
                neighbours.end(),
                slice
            )
        )
    },
    procBoundaryName_
    {
        word("procBoundary") +
        Foam::name(Pstream::myProcNo()) +
        word("to") +
        Foam::name(slice_.partition())
    }
{
    determineFaceIDs(neighbours);
    ++instanceCount_;
    id_ = Foam::encodeSlicePatchId((numNonProcPatches_ - 1) + instanceCount_);
}


// Copy constructor
Foam::sliceProcPatch::sliceProcPatch(const sliceProcPatch& other)
:
    id_{other.id_},
    slice_{other.slice_},
    localFaceIDs_{other.localFaceIDs_.begin(), other.localFaceIDs_.end()},
    localPointIDs_{other.localPointIDs_.begin(), other.localPointIDs_.end()},
    procBoundaryName_{other.procBoundaryName_}
{
    ++instanceCount_;
}


//TODO: Must not be copyable because of instanceCount_ consider move semantics only
Foam::sliceProcPatch&
Foam::sliceProcPatch::operator=(Foam::sliceProcPatch const& other)
{
    Foam::sliceProcPatch tmp(other);
    swap(tmp);
    return *this;
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * //

Foam::sliceProcPatch::~sliceProcPatch()
{
    --instanceCount_;
}

// * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * * //

void Foam::sliceProcPatch::swap(Foam::sliceProcPatch& other) noexcept
{
    using std::swap;
    swap( id_, other.id_ );
    swap( instanceCount_, other.instanceCount_ );
    swap( slice_, other.slice_ );
    swap( localFaceIDs_, other.localFaceIDs_ );
    swap( localPointIDs_, other.localPointIDs_ );
    swap( procBoundaryName_, other.procBoundaryName_ );
}


Foam::label Foam::sliceProcPatch::id()
{
    return id_;
}


Foam::word Foam::sliceProcPatch::name()
{
    return procBoundaryName_;
}


Foam::label Foam::sliceProcPatch::partner()
{
    return slice_.partition();
}


void Foam::sliceProcPatch::determineFaceIDs(const Foam::labelList& neighbours)
{
    auto faceIdIter = localFaceIDs_.begin();
    for
    (
        auto neighboursIter = std::begin(neighbours);
        neighboursIter != std::end(neighbours);
        ++neighboursIter
    )
    {
        if (slice_(*neighboursIter))
        {
            *faceIdIter = std::distance(neighbours.begin(), neighboursIter);
            ++faceIdIter;
        }
    }
}


void Foam::sliceProcPatch::determinePointIDs
(
    const Foam::faceList& faces,
    const Foam::label& bottomPointId
)
{
    auto pred = [bottomPointId](const auto& id)
                {
                    return bottomPointId<=id;
                };
    auto pointIDs = Foam::pointSubset(faces, pred);
    localPointIDs_.resize(pointIDs.size());
    std::transform
    (
        pointIDs.begin(),
        pointIDs.end(),
        localPointIDs_.begin(),
        [&bottomPointId](const auto& id)
        {
            return id - bottomPointId;
        }
    );
}


void Foam::sliceProcPatch::appendOwner
(
    Foam::labelList& owner,
    Foam::labelList& recvOwner
)
{
    std::transform
    (
        recvOwner.begin(),
        recvOwner.end(),
        recvOwner.begin(),
        [this](const auto& id)
        {
            return slice_.convert(id);
        }
    );
    owner.append(recvOwner);
}


void Foam::sliceProcPatch::encodePatch(Foam::labelList& neighbours)
{
    std::transform
    (
        std::begin(neighbours),
        std::end(neighbours),
        std::begin(neighbours),
        [this](const auto& cellId)
        {
            return slice_(cellId) ? id_ : cellId;
        }
    );
}


void Foam::sliceProcPatch::encodePatch
(
    std::vector<Foam::label>& neighbours,
    const Foam::label& numPatchFaces
)
{
    std::fill_n(std::back_inserter(neighbours), numPatchFaces, id_);
}


void Foam::sliceProcPatch::encodePatch
(
    Foam::labelList& neighbours,
    const Foam::label& numPatchFaces
)
{
    auto curr_size = neighbours.size();
    neighbours.resize(curr_size + numPatchFaces);
    std::fill_n(neighbours.begin() + curr_size, numPatchFaces, id_);
}


Foam::pointField
Foam::sliceProcPatch::extractPoints(const Foam::pointField& input)
{
    Foam::pointField output;
    output.resize(localPointIDs_.size());
    std::transform
    (
        std::begin(localPointIDs_),
        std::end(localPointIDs_),
        std::begin(output),
        [&input](const auto& id)
        {
            return input[id];
        }
    );
    return output;
}


void Foam::swap(Foam::sliceProcPatch& a, Foam::sliceProcPatch& b) noexcept
{
    a.swap( b );
}


// ************************************************************************* //
