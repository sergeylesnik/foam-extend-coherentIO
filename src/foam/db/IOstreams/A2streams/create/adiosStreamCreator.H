#ifndef adiosStreamCreator_H
#define adiosStreamCreator_H

#include <memory>

namespace Foam
{

// Forward declaration
class adiosStream;

struct adiosStreamCreator
{
    virtual ~adiosStreamCreator() = default;

    virtual std::unique_ptr<adiosStream> createStream() = 0;
};

}

#endif
