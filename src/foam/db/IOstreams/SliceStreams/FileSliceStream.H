#ifndef FileSliceStream_H
#define FileSliceStream_H

#include "SliceStream.H"

namespace Foam
{

// Forward declarations
class string;
class StreamFeatures;

class FileSliceStream
:
    public SliceStream
{

    std::unique_ptr<StreamFeatures> sliceFile_;

    virtual void v_access() final;

    virtual void v_flush() final;

public:

    explicit FileSliceStream(std::unique_ptr<StreamFeatures>&);

};

}

#endif
