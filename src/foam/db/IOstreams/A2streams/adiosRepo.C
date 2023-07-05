
#include "adiosRepo.H"

#include "adios2.h"

#include "Pstream.H"
#include "foamString.H"

Foam::adiosRepo*
Foam::adiosRepo::repoInstance_ = nullptr;

Foam::adiosRepo::adiosRepo()
:
    pimpl_{std::make_unique<Foam::adiosRepo::Impl>()},
    boundaryCounter_{0}
{}


struct Foam::adiosRepo::Impl
{

    using ADIOS_uPtr = std::unique_ptr<adios2::ADIOS>;
    using IO_map_uPtr = std::unique_ptr<Foam::adiosRepo::IO_map>;
    using Engine_map_uPtr = std::unique_ptr<Foam::adiosRepo::Engine_map>;


    // Default constructor
    Impl()
    :
        adiosPtr_{nullptr},
        ioMap_{std::make_unique<Foam::adiosRepo::IO_map>()},
        engineMap_{std::make_unique<Foam::adiosRepo::Engine_map>()}
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


Foam::adiosRepo::~adiosRepo() = default;


Foam::adiosRepo* Foam::adiosRepo::instance()
{
    if (!repoInstance_)
    {
        repoInstance_ = new Foam::adiosRepo();
    }
    return repoInstance_;
}


adios2::ADIOS*
Foam::adiosRepo::pullADIOS()
{
    return pimpl_->adiosPtr_.get();
}


Foam::adiosRepo::IO_map*
Foam::adiosRepo::get(const std::shared_ptr<adios2::IO>&)
{
    return pimpl_->ioMap_.get();
}


Foam::adiosRepo::Engine_map*
Foam::adiosRepo::get(const std::shared_ptr<adios2::Engine>&)
{
    return pimpl_->engineMap_.get();
}


void Foam::adiosRepo::push(const Foam::label& input)
{
    boundaryCounter_ = input;
}


void Foam::adiosRepo::close()
{
    for (const auto& enginePair: *(pimpl_->engineMap_))
    {
        if (*(enginePair.second))
        {
            if (enginePair.second->OpenMode() != adios2::Mode::ReadRandomAccess)
            {
                enginePair.second->EndStep();
            }
            enginePair.second->Close();
        }
    }
    pimpl_->engineMap_->clear();
}

void Foam::adiosRepo::clear()
{
    close();
    pimpl_->adiosPtr_->FlushAll();
    //pimpl_->adiosPtr_->RemoveAllIOs();
    //pimpl_->ioMap_->clear();
    for (const auto& ioPair: *(pimpl_->ioMap_))
    {
        ioPair.second->RemoveAllVariables();
    }
}
