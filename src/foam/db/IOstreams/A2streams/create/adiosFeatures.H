
#ifndef adiosFeatures_H
#define adiosFeatures_H

#include <memory>

namespace adios2
{
class ADIOS;
class IO;
class Engine;
}

namespace Foam
{

// Forward declaration
class fileName;

struct adiosFeatures
{
    virtual ~adiosFeatures() = default;

    virtual std::shared_ptr<adios2::IO>
    createIO(adios2::ADIOS* const) = 0;

    virtual std::shared_ptr<adios2::Engine>
    createEngine(adios2::IO* const, const fileName&) = 0;
};

}

#endif
