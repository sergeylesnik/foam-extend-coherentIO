
#include "sliceMap.H"

// * * * * * * * * * * * * * * * * Constructor  * * * * * * * * * * * * * * //

Foam::sliceMap::sliceMap(const Foam::label& numNativeEntities)
:
    mapping_{},
    numNativeEntities_{numNativeEntities}
{}

// * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * * //

Foam::label Foam::sliceMap::operator[](const Foam::label& id)
{
    return mapping_[id];
}

bool Foam::sliceMap::exist(const Foam::label& id)
{
    return mapping_.count(id) == 1;
};

// ************************************************************************* //