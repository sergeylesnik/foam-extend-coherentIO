
#ifndef adiosInputFeatures_H
#define adiosInputFeatures_H

#include "adiosFeatures.H"

namespace Foam
{

struct adiosInputFeatures
:
    public adiosFeatures
{
    virtual std::shared_ptr<adios2::IO>
    createIO( adios2::ADIOS* const ) override;

    virtual std::shared_ptr<adios2::Engine>
    createEngine( adios2::IO* const, const Foam::fileName& ) override;
};

}

#endif
