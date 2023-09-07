
#include "InputFeatures.H"

#include "adios2.h"
#include "IO.h"
#include "Engine.h"

#include "SliceStreamRepo.H"

#include "fileName.H"


std::shared_ptr<adios2::IO>
Foam::InputFeatures::createIO(adios2::ADIOS* const corePtr)
{
    Foam::SliceStreamRepo* repo = Foam::SliceStreamRepo::instance();
    std::shared_ptr<adios2::IO> ioPtr{nullptr};
    repo->pull(ioPtr, "read");
    if (!ioPtr)
    {
        ioPtr = std::make_shared<adios2::IO>(corePtr->DeclareIO("read"));
        ioPtr->SetEngine("BP5");
        repo->push(ioPtr, "read");
    }
    return ioPtr;
}


std::shared_ptr<adios2::Engine>
Foam::InputFeatures::createEngine
(
    adios2::IO* const ioPtr,
    const Foam::fileName& path
)
{
    Foam::SliceStreamRepo* repo = Foam::SliceStreamRepo::instance();
    std::shared_ptr<adios2::Engine> enginePtr{nullptr};
    auto size = path.length();
    repo->pull( enginePtr, "read"+path( size ) );
    if ( !enginePtr )
    {
        enginePtr = std::make_shared<adios2::Engine>
                    (
                        ioPtr->Open( path, adios2::Mode::ReadRandomAccess )
                    );
        repo->push( enginePtr, "read"+path( size ) );
    }
    return enginePtr;
}

