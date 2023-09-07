
#include "Slice.H"

#include "Offsets.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::Slice::Slice
(
    const Foam::label& partition,
    const Foam::Offsets& offsets
)
:
    Slice
    (
        partition,
        offsets.lowerBound(partition),
        offsets.upperBound(partition)
    )
{}


Foam::Slice::Slice
(
    const Foam::label& bottom,
    const Foam::label& top
)
:
    Slice(-1, bottom, top)
{}


Foam::Slice::Slice
(
    const Foam::label& partition,
    const Foam::label& bottom,
    const Foam::label& top
)
:
    partition_{partition},
    bottom_{bottom},
    top_{top},
    mapping_{std::make_shared<Foam::sliceMap>(top_-bottom_)}
{}

// * * * * * * * * * * * * Private Member Functions * * * * * * * * * * * * //

Foam::label Foam::Slice::shift(const Foam::label& id) const
{
    return id - bottom_;
}

// * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * * //

Foam::label Foam::Slice::partition() const
{
    return partition_;
}


bool Foam::Slice::operator()(const Foam::label& id) const
{
    return (bottom_<=id && id<top_);
}


bool Foam::Slice::exist(const Foam::label& id) const
{
    return (mapping_->exist(id)) || (this->operator()(id));
}


Foam::label Foam::Slice::convert(const Foam::label& id) const
{
    return this->operator()(id) ? shift(id) : mapping_->operator[](id);
}

// ************************************************************************* //
