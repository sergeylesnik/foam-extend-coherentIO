
#include "FileSliceStream.H"
#include "StreamFeatures.H"

#include "SliceStreamImpl.H"

#include "foamString.H"


Foam::FileSliceStream::FileSliceStream
(
    std::unique_ptr<StreamFeatures>& fileFeatures
)
:
    sliceFile_{std::move(fileFeatures)}
{}


void Foam::FileSliceStream::v_access()
{
    Foam::SliceStreamRepo* repo = Foam::SliceStreamRepo::instance();
    ioPtr_ = sliceFile_->createIO(repo->pullADIOS());
    enginePtr_ = sliceFile_->createEngine(ioPtr_.get(), paths_.getPathName());
}


void Foam::FileSliceStream::v_flush()
{
    Foam::SliceStreamRepo* repo = Foam::SliceStreamRepo::instance();
    repo->close();
    v_access();
}
