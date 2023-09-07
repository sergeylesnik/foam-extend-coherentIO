
#include "Offsets.H"

#include "globalIndex.H"


Foam::Offsets::Offsets(label value, bool reduce)
{
    set(value, reduce);
}


void Foam::Offsets::set(Foam::label value, bool reduce)
{
    Foam::globalIndex offsets(value, reduce);

    // Guarantee monotonically increasing values
    offset_pairs_[0].second = offsets.offset(1);
    for (label proc = 1; proc < Foam::Pstream::nProcs(); ++proc)
    {
        offset_pairs_[proc].second = offsets.offset(proc+1);
        if (offset_pairs_[proc].second < offset_pairs_[proc-1].second)
        {
            offset_pairs_[proc].second = offset_pairs_[proc-1].second;
        }
    }

    // Translate offsets through lower bound -- first in pair
    for (auto it = offset_pairs_.begin(); it < offset_pairs_.end()-1; ++it)
    {
        (it+1)->first = it->second;
    }
}


Foam::label Foam::Offsets::lowerBound(Foam::label myProcNo) const
{
    return offset_pairs_[myProcNo].first;
}


Foam::label Foam::Offsets::upperBound(Foam::label myProcNo) const
{
    return offset_pairs_[myProcNo].second;
}


Foam::label Foam::Offsets::count(Foam::label myProcNo) const
{
    return upperBound(myProcNo) - lowerBound(myProcNo);
}


Foam::label Foam::Offsets::size() const
{
    return count(Pstream::myProcNo());
}


Foam::Offsets::iterator Foam::Offsets::begin()
{
    return offset_pairs_.begin();
}


Foam::Offsets::iterator Foam::Offsets::end()
{
    return offset_pairs_.end();
}


std::pair<Foam::label, Foam::label>
Foam::Offsets::operator[](label i) const
{
    return offset_pairs_[i];
}

