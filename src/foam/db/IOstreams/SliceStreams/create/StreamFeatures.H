
#ifndef StreamFeatures_H
#define StreamFeatures_H

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

struct StreamFeatures
{
    virtual ~StreamFeatures() = default;

    virtual std::shared_ptr<adios2::IO>
    createIO(adios2::ADIOS* const) = 0;

    virtual std::shared_ptr<adios2::Engine>
    createEngine(adios2::IO* const, const fileName&) = 0;
};

}

#endif
