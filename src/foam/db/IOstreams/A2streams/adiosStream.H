#ifndef adiosStream_H
#define adiosStream_H

#include "labelList.H"

#include "adiosRepo.H"

#include "adiosPaths.H"

#include "adiosWriting.H"
#include "adiosReading.H"

namespace Foam
{

// Determine whether mesh or field data I/O is triggered
std::string adiosStreamType(const std::string& id);

// Deprecated begin step of ADIOS2
template<class adiosFactory> void adiosBeginStep(const Foam::string&);

// Deprecated end step of ADIOS2
template<class adiosFactory> void adiosEndStep(const Foam::string&);

template<typename Container>
void adiosReadToContainer
(
    const Foam::string& type,
    const Foam::string& pathname,
    const Foam::string& blockId,
    Container& container,
    const labelList& start = {},
    const labelList& count = {}
);


class adiosStream
{
    virtual void v_open() = 0;

protected:

    // Forward declaration of bridge to implementation
    class Impl;

    // Bridge instance to implementation
    std::unique_ptr<Impl> pimpl_;

    // Instance on file name manager
    adiosPaths paths_{};

    // Type of I/O data; field or mesh
    Foam::string type_{};

    // Pointer to io instance
    std::shared_ptr<adios2::IO> ioPtr_{nullptr};

    // Pointer to engine instance
    std::shared_ptr<adios2::Engine> enginePtr_{nullptr};

    // Setter for bp file name and path
    void setPath(const Foam::string& type, const Foam::string& path = "");

public:

    // Default constructor
    adiosStream();

    // Destructor
    virtual ~adiosStream();

    // Open engine according to mesh or field data and path
    void open(const Foam::string& type, const Foam::string& path = "");

    // Reading local/global scalar array
    void transfer
    (
        const string& blockId,
        scalar* data,
        const labelList& start = {},
        const labelList& count = {}
    );

    // Reading local/global label array
    void transfer
    (
        const string& blockId,
        label* data,
        const labelList& start = {},
        const labelList& count = {}
    );

    // Reading local/global char array
    void transfer
    (
        const string& blockId,
        char* data,
        const labelList& start = {},
        const labelList& count = {}
    );

    label getBufferSize(const Foam::string& blockId, const scalar* const data);

    label getBufferSize(const Foam::string& blockId, const label* const data);

    label getBufferSize(const Foam::string& blockId, const char* const data);

    // Reading local/global array
    template<class ContainerType>
    typename std::enable_if<!std::is_const<ContainerType>::value, void>::type
    transfer
    (
        const string& blockId,
        ContainerType& data,
        const labelList& start = {},
        const labelList& count = {}
    );

    // Writing local/global scalar array
    void transfer
    (
        const Foam::string& blockId,
        const Foam::labelList& shape,
        const Foam::labelList& start,
        const Foam::labelList& count,
        const scalar* buf,
        const labelList& mapping = {},
        const bool masked = false
    );

    // Writing local/global label array
    void transfer
    (
        const Foam::string& blockId,
        const Foam::labelList& shape,
        const Foam::labelList& start,
        const Foam::labelList& count,
        const label* buf,
        const labelList& mapping = {},
        const bool masked = false
    );

    void transfer
    (
        const Foam::string& blockId,
        const Foam::labelList& shape,
        const Foam::labelList& start,
        const Foam::labelList& count,
        const char* buf,
        const labelList& mapping = {},
        const bool masked = false
    );

    void close();

};

}


// Reading local/global array
template<class ContainerType>
typename std::enable_if<!std::is_const<ContainerType>::value, void>::type
Foam::adiosStream::transfer
(
    const Foam::string& blockId,
    ContainerType& container,
    const Foam::labelList& start,
    const Foam::labelList& count
)
{
    if (start.empty() && count.empty())
    {
        auto size = getBufferSize(blockId, container.data());
        container.resize(size);
    }
    else
    {
        container.resize(count[0]);
    }
    transfer(blockId, container.data(), start, count);
}


template<typename Container>
void Foam::adiosReadToContainer
(
    const Foam::string& type,
    const Foam::string& pathname,
    const Foam::string& blockId,
    Container& container,
    const Foam::labelList& start,
    const Foam::labelList& count
)
{
    auto adiosStreamPtr = adiosReading{}.createStream();
    adiosStreamPtr->open(type, pathname);
    adiosStreamPtr->transfer(blockId, container, start, count);
    adiosStreamPtr->close();
}


#endif
