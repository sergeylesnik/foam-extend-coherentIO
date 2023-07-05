
#include "Slice.H"

#include "Offsets.H"

// * * * * * * * * * * * * * * Free Functions * * * * * * * * * * * * * //

std::set<Foam::label> Foam::missingPoints
(
    const Foam::faceList& faces,
    const Foam::Slice& slice
)
{
    auto pointsInFaces = Foam::pointSubset(faces);
    std::set<label> missingPointIDs{};
    std::copy_if
    (
        std::begin(pointsInFaces),
        std::end(pointsInFaces),
        std::inserter(missingPointIDs, missingPointIDs.end()),
        [&slice](const label& id)
        {
            return slice.exist(id);
        }
    );
    return missingPointIDs;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::Slice::Slice
(
    const Foam::label& partition,
    const Foam::Offsets& offsets
)
:
    partition_{partition},
    bottom_{offsets.lowerBound(partition)},
    top_{offsets.upperBound(partition)},
    mapping_{std::make_shared<Foam::sliceMap>(offsets.count(partition))}
{}

// * * * * * * * * * * * * Private Member Functions * * * * * * * * * * * * //

Foam::label Foam::Slice::shift(const Foam::label& id) const
{
    return id - bottom_;
}

// * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * * //

Foam::label Foam::Slice::partition()
{
    return partition_;
}


bool Foam::Slice::operator()(const Foam::label& id) const
{
    return (bottom_<=id && id<top_);
}


bool Foam::Slice::exist(const Foam::label& id) const
{
    return !(mapping_->exist(id)) && !(this->operator()(id));
}


Foam::label Foam::Slice::convert(const Foam::label& id) const
{
    return this->operator()(id) ? shift(id) : mapping_->operator[](id);
}

// ************************************************************************* //