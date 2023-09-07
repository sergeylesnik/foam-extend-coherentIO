
#include "SliceReading.H"

#include "InputFeatures.H"
#include "FileSliceStream.H"


std::unique_ptr<Foam::SliceStream>
Foam::SliceReading::createStream()
{
    std::unique_ptr<Foam::StreamFeatures> file(new Foam::InputFeatures{});
    return std::unique_ptr<Foam::FileSliceStream>
    (
        new Foam::FileSliceStream{file}
    );
}


