
#ifndef adiosOutputFeatures_H
#define adiosOutputFeatures_H

#include "adiosFeatures.H"

namespace Foam
{

struct adiosOutputFeatures
:
    public adiosFeatures
{
    virtual std::shared_ptr<adios2::IO>
    createIO(adios2::ADIOS* const) override;

    virtual std::shared_ptr<adios2::Engine>
    createEngine(adios2::IO* const, const fileName&) override;
};

}

#endif
