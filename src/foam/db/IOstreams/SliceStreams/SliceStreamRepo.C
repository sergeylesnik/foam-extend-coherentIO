
#include "SliceStreamRepo.H"

#include "adios2.h"

#include "Pstream.H"
#include "foamString.H"

Foam::SliceStreamRepo* Foam::SliceStreamRepo::repoInstance_ = nullptr;

struct Foam::SliceStreamRepo::Impl
{

    using ADIOS_uPtr = std::unique_ptr<adios2::ADIOS>;
    using IO_map_uPtr = std::unique_ptr<Foam::SliceStreamRepo::IO_map>;
    using Engine_map_uPtr = std::unique_ptr<Foam::SliceStreamRepo::Engine_map>;


    // Default constructor
    Impl()
    :
        adiosPtr_{nullptr},
        ioMap_{new Foam::SliceStreamRepo::IO_map()},
        engineMap_{new Foam::SliceStreamRepo::Engine_map()}
    {
        if (!adiosPtr_)
        {
            if (Pstream::parRun())
            {
                adiosPtr_.reset
                          (
                              new adios2::ADIOS
                                  (
                                      "system/config.xml",
                                      MPI_COMM_WORLD
                                  )
                          );
            }
            else
            {
                adiosPtr_.reset(new adios2::ADIOS("system/config.xml"));
            }
        }
    }


    ADIOS_uPtr adiosPtr_{};

    IO_map_uPtr ioMap_{};

    Engine_map_uPtr engineMap_{};
};


Foam::SliceStreamRepo::SliceStreamRepo()
:
    pimpl_{new Foam::SliceStreamRepo::Impl{}},
    boundaryCounter_{0}
{}


Foam::SliceStreamRepo::~SliceStreamRepo() = default;


Foam::SliceStreamRepo* Foam::SliceStreamRepo::instance()
{
    if (!repoInstance_)
    {
        repoInstance_ = new Foam::SliceStreamRepo();
    }
    return repoInstance_;
}


adios2::ADIOS*
Foam::SliceStreamRepo::pullADIOS()
{
    return pimpl_->adiosPtr_.get();
}


Foam::SliceStreamRepo::IO_map*
Foam::SliceStreamRepo::get(const std::shared_ptr<adios2::IO>&)
{
    return pimpl_->ioMap_.get();
}


Foam::SliceStreamRepo::Engine_map*
Foam::SliceStreamRepo::get(const std::shared_ptr<adios2::Engine>&)
{
    return pimpl_->engineMap_.get();
}


void Foam::SliceStreamRepo::push(const Foam::label& input)
{
    boundaryCounter_ = input;
}


void Foam::SliceStreamRepo::open(const bool atScale)
{
    for (const auto& enginePair: *(pimpl_->engineMap_))
    {
        if (*(enginePair.second))
        {
            if (enginePair.second->OpenMode() != adios2::Mode::ReadRandomAccess)
            {
                enginePair.second->BeginStep();
            }
        }
    }
}

void Foam::SliceStreamRepo::close(const bool atScale)
{
    for (const auto& enginePair: *(pimpl_->engineMap_))
    {
        if (*(enginePair.second))
        {
            if (enginePair.second->OpenMode() != adios2::Mode::ReadRandomAccess)
            {
                enginePair.second->EndStep();
            }
            if (!atScale)
            {
                enginePair.second->Close();
            }
        }
    }

    if (!atScale)
    {
        pimpl_->engineMap_->clear();
    }
}

void Foam::SliceStreamRepo::clear()
{
    close();
    pimpl_->adiosPtr_->FlushAll();
    for (const auto& ioPair: *(pimpl_->ioMap_))
    {
        ioPair.second->RemoveAllVariables();
    }
}
