#ifndef adiosFileStream_H
#define adiosFileStream_H

#include "adiosStream.H"

namespace Foam
{

// Forward declarations
class string;
class adiosFeatures;

class adiosFileStream
:
    public adiosStream
{

    std::unique_ptr<adiosFeatures> adiosFile_;

    virtual void v_open() final;

public:

    explicit adiosFileStream(std::unique_ptr<adiosFeatures>&);

};

}

#endif
