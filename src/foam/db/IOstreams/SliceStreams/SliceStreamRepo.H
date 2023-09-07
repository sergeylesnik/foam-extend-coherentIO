#ifndef SliceStreamRepo_H
#define SliceStreamRepo_H

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


class SliceStreamRepo
{
    // Singelton instance
    static SliceStreamRepo* repoInstance_;

    // Private default constructor in singelton
    SliceStreamRepo();

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
    static SliceStreamRepo* instance();

    // Destructor
    ~SliceStreamRepo();

    // Deleted copy constructor
    SliceStreamRepo(SliceStreamRepo& other) = delete;

    // Deleted copy assignment operator
    SliceStreamRepo& operator=(const SliceStreamRepo& other) = delete;

    // Getter for the ADIOS instance
    adios2::ADIOS* pullADIOS();

    // Pull of an ADIOS specific feature (IO, Engine, SliceBuffer)
    template<typename FeatureType>
    void pull(std::shared_ptr<FeatureType>&, const Foam::string&);

    // Push of an ADIOS specific feature (IO, Engine, SliceBuffer)
    template<typename FeatureType>
    void push(const std::shared_ptr<FeatureType>&, const Foam::string&);

    // Setter to boundaryCounter_
    // TODO: Not the responsibility of SliceStreamRepo. Can this be removed?
    void push(const label&);

    // Removal of an ADIOS specific feature (IO, Engine, SliceBuffer)
    template<typename FeatureType>
    void remove(const std::shared_ptr<FeatureType>&, const Foam::string&);

    // Initiating engines with Engine::BeginStep
    void open(const bool atScale = false);

    // Closing all engines and clear the engine map
    void close(const bool atScale = false);

    void clear();

};

}

#include "SliceStreamRepoI.H"

#endif

