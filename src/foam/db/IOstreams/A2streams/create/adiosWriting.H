
#ifndef adiosWriting_H
#define adiosWriting_H

#include "adiosStreamCreator.H"

namespace Foam
{

// Forward declaration
class adiosStream;

struct adiosWriting
:
    public adiosStreamCreator
{
    virtual std::unique_ptr<adiosStream> createStream() override;
};

}

#endif

