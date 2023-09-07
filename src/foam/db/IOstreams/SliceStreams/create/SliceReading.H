
#ifndef SliceReading_H
#define SliceReading_H

#include "SliceStreamCreator.H"

namespace Foam
{

// Forward declaration
class SliceStream;

struct SliceReading
:
    public SliceStreamCreator
{
    virtual std::unique_ptr<SliceStream> createStream() override;
};

}

#endif

