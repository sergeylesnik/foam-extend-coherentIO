
#include "SliceStream.H"

#include "SliceStreamImpl.H"

// * * * * * * * * * * * * * * * * Constructor  * * * * * * * * * * * * * * //

Foam::SliceStream::SliceStream()
:
    pimpl_{new Impl()},
    paths_{},
    type_{},
    ioPtr_{nullptr},
    enginePtr_{nullptr}
{}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * //

Foam::SliceStream::~SliceStream() = default;


std::string Foam::SliceStreamType(const std::string& id)
{
    std::string type = "fields";
    if (id.find("polyMesh") != std::string::npos
        ||
        id.find("region") != std::string::npos)
    {
        type = "mesh";
    }
    return type;
}

// * * * * * * * * * * * * * Private Member Functions * * * * * * * * * * * //

void
Foam::SliceStream::setPath(const Foam::string& type, const Foam::string& path)
{
    if (type == "mesh")
    {
        paths_.setPathName(paths_.meshPathname(path));
    }
    else if (type == "fields")
    {
        paths_.setPathName(paths_.dataPathname(path));
    }
}

// * * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * //

void Foam::SliceStream::access(const Foam::string& type, const Foam::string& path)
{
    type_ = type;
    setPath(type, path);
    v_access();
}


void Foam::SliceStream::bufferSync()
{
    if (enginePtr_)
    {
        if
        (
            enginePtr_->OpenMode() == adios2::Mode::Read
         || enginePtr_->OpenMode() == adios2::Mode::ReadRandomAccess
        )
        {
            enginePtr_->PerformGets();
        }
        else
        {
            enginePtr_->PerformPuts();
        }
    }
}


Foam::label Foam::SliceStream::getBufferSize
(
    const Foam::string& blockId,
    const Foam::scalar* const data
)
{
    return pimpl_->readingBuffer<Foam::variableBuffer<Foam::scalar> >
                   (
                       ioPtr_.get(),
                       enginePtr_.get(),
                       blockId
                   );
}


Foam::label Foam::SliceStream::getBufferSize
(
    const Foam::string& blockId,
    const Foam::label* const data
)
{
    return pimpl_->readingBuffer<Foam::variableBuffer<Foam::label> >
                   (
                       ioPtr_.get(),
                       enginePtr_.get(),
                       blockId
                   );
}


Foam::label Foam::SliceStream::getBufferSize
(
    const Foam::string& blockId,
    const char* const data
)
{
    return pimpl_->readingBuffer<Foam::variableBuffer<char> >
                   (
                       ioPtr_.get(),
                       enginePtr_.get(),
                       blockId
                   );
}


// Reading
void Foam::SliceStream::get
(
    const Foam::string& blockId,
    scalar* data,
    const Foam::labelList& start,
    const Foam::labelList& count
)
{
    pimpl_->get
            (
                ioPtr_.get(),
                enginePtr_.get(),
                blockId,
                data,
                start,
                count
            );
}


void Foam::SliceStream::get
(
    const Foam::string& blockId,
    label* data,
    const Foam::labelList& start,
    const Foam::labelList& count
)
{
    pimpl_->get
            (
                ioPtr_.get(),
                enginePtr_.get(),
                blockId,
                data,
                start,
                count
            );
}


void Foam::SliceStream::get
(
    const Foam::string& blockId,
    char* data,
    const Foam::labelList& start,
    const Foam::labelList& count
)
{
    pimpl_->get
            (
                ioPtr_.get(),
                enginePtr_.get(),
                blockId,
                data,
                start,
                count
            );
}


// Writing
void Foam::SliceStream::put
(
    const Foam::string& blockId,
    const Foam::labelList& shape,
    const Foam::labelList& start,
    const Foam::labelList& count,
    const scalar* data,
    const Foam::labelList& mapping,
    const bool masked
)
{
    pimpl_->put
            (
                ioPtr_.get(),
                enginePtr_.get(),
                blockId,
                shape,
                start,
                count,
                data,
                mapping,
                masked
            );
}


void Foam::SliceStream::put
(
    const Foam::string& blockId,
    const Foam::labelList& shape,
    const Foam::labelList& start,
    const Foam::labelList& count,
    const label* data,
    const Foam::labelList& mapping,
    const bool masked
)
{
    pimpl_->put
            (
                ioPtr_.get(),
                enginePtr_.get(),
                blockId,
                shape,
                start,
                count,
                data,
                mapping,
                masked
            );
}


void Foam::SliceStream::put
(
    const Foam::string& blockId,
    const Foam::labelList& shape,
    const Foam::labelList& start,
    const Foam::labelList& count,
    const char* data,
    const Foam::labelList& mapping,
    const bool masked
)
{
    pimpl_->put
            (
                ioPtr_.get(),
                enginePtr_.get(),
                blockId,
                shape,
                start,
                count,
                data,
                mapping,
                masked
            );
}


void Foam::SliceStream::flush()
{
    v_flush();
}

// ************************************************************************* //
