
#ifndef OutputFeatures_H
#define OutputFeatures_H

#include "StreamFeatures.H"

namespace Foam
{

struct OutputFeatures
:
    public StreamFeatures
{
    virtual std::shared_ptr<adios2::IO>
    createIO(adios2::ADIOS* const) override;

    virtual std::shared_ptr<adios2::Engine>
    createEngine(adios2::IO* const, const fileName&) override;
};

}

#endif
