
#include "SliceWriting.H"

#include "OutputFeatures.H"
#include "FileSliceStream.H"


std::unique_ptr<Foam::SliceStream>
Foam::SliceWriting::createStream()
{
    std::unique_ptr<Foam::StreamFeatures> file(new Foam::OutputFeatures{});
    return std::unique_ptr<Foam::FileSliceStream>
    (
        new Foam::FileSliceStream{file}
    );
}


