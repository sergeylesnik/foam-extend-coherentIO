
#include "OutputFeatures.H"

#include "adios2.h"
#include "IO.h"
#include "Engine.h"

#include "SliceStreamRepo.H"

#include "fileName.H"


std::shared_ptr<adios2::IO>
Foam::OutputFeatures::createIO(adios2::ADIOS* const corePtr)
{
    SliceStreamRepo* repo = Foam::SliceStreamRepo::instance();
    std::shared_ptr<adios2::IO> ioPtr{nullptr};
    repo->pull(ioPtr, "write");
    if (!ioPtr)
    {
        ioPtr = std::make_shared<adios2::IO>(corePtr->DeclareIO("write"));
        ioPtr->SetEngine("BP5");
        repo->push(ioPtr, "write");
    }
    return ioPtr;
}


std::shared_ptr<adios2::Engine>
Foam::OutputFeatures::createEngine
(
    adios2::IO* const ioPtr,
    const Foam::fileName& path
)
{
    SliceStreamRepo* repo = Foam::SliceStreamRepo::instance();
    std::shared_ptr<adios2::Engine> enginePtr{nullptr};
    auto size = path.length();
    repo->pull(enginePtr, "write" + path(size));
    if (!enginePtr)
    {
        enginePtr = std::make_shared<adios2::Engine>
                    (
                        ioPtr->Open(path, adios2::Mode::Append)
                    );
        enginePtr->BeginStep();
        repo->push(enginePtr, "write" + path(size));
    }
    return enginePtr;
}

