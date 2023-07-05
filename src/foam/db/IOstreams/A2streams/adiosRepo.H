#ifndef adiosRepo_H
#define adiosRepo_H

#include "label.H"

#include <map>
#include <memory>

// Forward declaration
namespace adios2
{
class ADIOS;
class IO;
class Engine;
}

namespace Foam
{

// Forward declaration
class string;


class adiosRepo
{
    // Singelton instance
    static adiosRepo* repoInstance_;

    // Private default constructor in singelton
    adiosRepo();

    using IO_map = std::map<Foam::string, std::shared_ptr<adios2::IO>>;
    using Engine_map = std::map<Foam::string, std::shared_ptr<adios2::Engine>>;

    // Private members

    // Forward declaration of bridge to ADIOS2 dependencies
    class Impl;

    // Bridge instance to ADIOS2 implementation
    std::unique_ptr<Impl> pimpl_;

    label boundaryCounter_{0};

    // Private methods
    IO_map* get(const std::shared_ptr<adios2::IO>&);

    Engine_map* get(const std::shared_ptr<adios2::Engine>&);

public:

    // Getter to singelton instance
    static adiosRepo* instance();

    // Destructor
    ~adiosRepo();

    // Deleted copy constructor
    adiosRepo(adiosRepo& other) = delete;

    // Deleted copy assignment operator
    adiosRepo& operator=(const adiosRepo& other) = delete;

    // Getter for the ADIOS instance
    adios2::ADIOS* pullADIOS();

    // Pull of an ADIOS specific feature (IO, Engine, adiosBuffer)
    template<typename FeatureType>
    void pull(std::shared_ptr<FeatureType>&, const Foam::string&);

    // Push of an ADIOS specific feature (IO, Engine, adiosBuffer)
    template<typename FeatureType>
    void push(const std::shared_ptr<FeatureType>&, const Foam::string&);

    // Setter to boundaryCounter_
    // TODO: Not the responsibility of adiosRepo. Can this be removed?
    void push(const label&);

    // Removal of an ADIOS specific feature (IO, Engine, adiosBuffer)
    template<typename FeatureType>
    void remove(const std::shared_ptr<FeatureType>&, const Foam::string&);

    // Closing all engines and clear the engine map
    void close();

    void clear();

};

}

#include "adiosRepoI.H"

#endif

