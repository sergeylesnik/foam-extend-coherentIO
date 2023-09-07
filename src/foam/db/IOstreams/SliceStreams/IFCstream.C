/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.1
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "IFCstream.H"
#include "IOstream.H"
#include "OSspecific.H"
#include "debug.H"
#include "gzstream.h"
#include "IStringStream.H"
#include "fileName.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(IFCstream, 0);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// ToDoIO Remove the allocator. Since ifPtr_ is always std::istringstream all
// the functionality may be moved to the IFCstream constructor
Foam::IFCstreamAllocator::IFCstreamAllocator
(
    const fileName& pathname,
    IOstream::streamFormat format
)
:
    ifPtr_(nullptr),
    bufStr_(),
    compression_(IOstream::UNCOMPRESSED)
{
    if (pathname.empty())
    {
        if (IFCstream::debug)
        {
            InfoInFunction
                << "cannot open null file " << endl;
        }
    }

    if (format != IOstream::COHERENT)
    {
        FatalErrorInFunction
            << "IO Format is not COHERENT"
            << exit(FatalError);
    }

    ifPtr_ = new std::istringstream();

    // The metadata is read only on master and then broadcasted to slaves.
    if (Pstream::master())
    {
        if (IFCstream::debug)
        {
            InfoInFunction
                << "Reading ASCII file " << pathname << endl;
        }

        std::istream* iPtr = new std::ifstream(pathname.c_str());

        // If the file is compressed, decompress it before reading.
        if (!iPtr->good() && isFile(pathname + ".gz", false))
        {
            delete iPtr;

            iPtr = new igzstream((pathname + ".gz").c_str());

            if (iPtr->good())
            {
                compression_ = IOstream::COMPRESSED;
            }
        }

        // Read to buffer
        if (iPtr->good())
        {
            iPtr->seekg(0, std::ios::end);
            size_t size = iPtr->tellg();
            bufStr_.resize(size);
            iPtr->seekg(0);
            iPtr->read(&bufStr_[0], size);
        }

        if (!iPtr->good())
        {
            // Invalidate stream if the variable is not found
            ifPtr_->setstate(std::ios::failbit);
        }
    }

    Pstream::scatter(bufStr_);

    // Assign the buffer of string bufStr_ to the buffer of the stream
    ifPtr_->rdbuf()->pubsetbuf(&bufStr_[0], bufStr_.size());
}


Foam::IFCstreamAllocator::~IFCstreamAllocator()
{
    delete ifPtr_;
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //
// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void Foam::IFCstream::readWordToken(token& t)
{
    word* wPtr = new word;

    if (read(*wPtr).bad())
    {
        delete wPtr;
        t.setBad();
    }
    else if (token::compound::isCompound(*wPtr))
    {
        if (debug)
        {
            Pout<< "Start constructing coherent compound token " << *wPtr
                << endl;
        }

        t = token::compound::New(*wPtr, 0).ptr();

        if (debug)
        {
            Pout<< "End constructing coherent compound token " << *wPtr << endl;
        }

        delete wPtr;
    }
    else
    {
        t = wPtr;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::IFCstream::IFCstream
(
    const fileName& pathname,
    const objectRegistry& registry,
    streamFormat format,
    versionNumber version
)
:
    IFCstreamAllocator(pathname, format),
    ISstream
    (
        *ifPtr_,
        "IFCstream.sourceFile_",
        format,
        version,
        IFCstreamAllocator::compression_
    ),
    pathname_(pathname),
    coherentMesh_
    (
        const_cast<CoherentMesh&>
        (
            registry.lookupObject<CoherentMesh>(CoherentMesh::typeName)
        )
    ),
    tmpIssPtr_(nullptr),
    sliceStreamPtr_(SliceReading{}.createStream())
{
    setClosed();

    setState(ifPtr_->rdstate());

    if (!good())
    {
        if (debug)
        {
            InfoInFunction
                << "Could not open file for input"
                << endl << info() << endl;
        }

        setBad();
    }
    else
    {
        setOpened();
    }

    lineNumber_ = 1;
}


// * * * * * * * * * * * * * * * * Destructors * * * * * * * * * * * * * * * //

Foam::IFCstream::~IFCstream()
{
    if (debug)
    {
        InfoInFunction
            << "Destructing stream" << endl;
    }

    if (tmpIssPtr_)
    {
        delete tmpIssPtr_;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

std::istream& Foam::IFCstream::stdStream()
{
    if (!ifPtr_)
    {
        FatalErrorIn("IFCstream::stdStream()")
            << "No stream allocated" << abort(FatalError);
    }
    return *ifPtr_;
}


const std::istream& Foam::IFCstream::stdStream() const
{
    if (!ifPtr_)
    {
        FatalErrorIn("IFCstream::stdStream() const")
            << "No stream allocated" << abort(FatalError);
    }
    return *ifPtr_;
}


void Foam::IFCstream::print(Ostream& os) const
{
    // Print File data
    os  << "IFCstream: ";
    ISstream::print(os);
}


Foam::Istream& Foam::IFCstream::readToStringStream(string& id)
{
    if (tmpIssPtr_)
    {

    }
    tmpIssPtr_ = new IStringStream(tmpIssBuf_);
    return *tmpIssPtr_;
}


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //

Foam::IFCstream&
Foam::IFCstream::operator()() const
{
    if (!good())
    {
        // also checks .gz file
        if (isFile(pathname_, true))
        {
            check("IFCstream::operator()");
            FatalIOError.exit();
        }
        else
        {
            FatalIOErrorIn("IFCstream::operator()", *this)
                << "file " << pathname_ << " does not exist"
                << exit(FatalIOError);
        }
    }

    return const_cast<IFCstream&>(*this);
}


// ************************************************************************* //
