
#ifndef InputFeatures_H
#define InputFeatures_H

#include "StreamFeatures.H"

namespace Foam
{

struct InputFeatures
:
    public StreamFeatures
{
    virtual std::shared_ptr<adios2::IO>
    createIO( adios2::ADIOS* const ) override;

    virtual std::shared_ptr<adios2::Engine>
    createEngine( adios2::IO* const, const Foam::fileName& ) override;
};

}

#endif
