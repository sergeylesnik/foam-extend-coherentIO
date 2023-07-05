
#ifndef adiosReading_H
#define adiosReading_H

#include "adiosStreamCreator.H"

namespace Foam
{

// Forward declaration
class adiosStream;

struct adiosReading
:
    public adiosStreamCreator
{
    virtual std::unique_ptr<adiosStream> createStream() override;
};

}

#endif

