#ifndef SliceStreamCreator_H
#define SliceStreamCreator_H

#include <memory>

namespace Foam
{

// Forward declaration
class SliceStream;

struct SliceStreamCreator
{
    virtual ~SliceStreamCreator() = default;

    virtual std::unique_ptr<SliceStream> createStream() = 0;
};

}

#endif
