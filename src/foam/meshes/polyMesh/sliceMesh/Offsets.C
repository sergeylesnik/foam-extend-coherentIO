
#include "Offsets.H"


void Foam::Offsets::do_reduce()
{
    label offset = 0;
    forAll(offsets_, procI)
    {
        label oldOffset = offset;
        offset += offsets_[procI];

        if (offset<oldOffset)
        {
            FatalErrorIn("Foam::Offsets")
            << "Overflow : sum of sizes " << offsets_
            << " exceeds capability of label (" << labelMax
            << "). Please recompile with larger datatype for label."
            << exit(FatalError);
        }
        offsets_[procI] = offset;
    }
}


Foam::Offsets::Offsets(label value, bool parRun, bool reduce)
{
    set(value, reduce);
}


void Foam::Offsets::set(Foam::label value, bool reduce)
{
    offsets_[Foam::Pstream::myProcNo()] = value;
    Foam::Pstream::gatherList(offsets_);
    Foam::Pstream::scatterList(offsets_);

    if (reduce) { do_reduce(); }

    // If processor does not own corresponding
    // mesh entities. Problem first arised with points.
    for (label proc = 1; proc<Foam::Pstream::nProcs(); ++proc)
    {
        if (offsets_[proc]<offsets_[proc - 1])
        {
            offsets_[proc] = offsets_[proc - 1];
        }
    }
}


Foam::label Foam::Offsets::lowerBound(Foam::label myProcNo) const
{
    return (myProcNo>0) ? offsets_[myProcNo - 1] : 0;
}


Foam::label Foam::Offsets::upperBound(Foam::label myProcNo) const
{
    return offsets_[myProcNo];
}


Foam::label Foam::Offsets::count(Foam::label myProcNo) const
{
    return upperBound(myProcNo) - lowerBound(myProcNo);
}


Foam::label Foam::Offsets::front() const
{
    return lowerBound(Pstream::myProcNo());
}


Foam::label Foam::Offsets::back() const
{
    return upperBound(Pstream::myProcNo());
}


Foam::label Foam::Offsets::size() const
{
    return count(Pstream::myProcNo());
}
