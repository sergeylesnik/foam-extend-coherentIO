
#ifndef SliceWriting_H
#define SliceWriting_H

#include "SliceStreamCreator.H"

namespace Foam
{

// Forward declaration
class SliceStream;

struct SliceWriting
:
    public SliceStreamCreator
{
    virtual std::unique_ptr<SliceStream> createStream() override;
};

}

#endif

