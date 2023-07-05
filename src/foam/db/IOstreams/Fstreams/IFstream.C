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

#include "IFstream.H"
#include "IOstream.H"
#include "OSspecific.H"
#include "debug.H"
#include "gzstream.h"
#include "adiosFileStream.H"
#include "IStringStream.H"

#include <fstream>
bool readLocalString( std::string& buf,
                      const Foam::string strName ) {

    //TODO : Parallel implementation. Only master reading and distributing.
    std::ifstream inFile;
    inFile.open( "fields/" + strName );
    bool found = inFile.good();
    if ( found ) {
        inFile.seekg(0, std::ios::end);
        size_t size = inFile.tellg();
        buf.resize( size );
        inFile.seekg(0);
        inFile.read( &buf[0], size );
        inFile.close();
    }

    return found;
}

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(Foam::IFstream, 0);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

Foam::IFstreamAllocator::IFstreamAllocator
(
    const fileName& pathname,
    IOstream::streamFormat format
)
:
    ifPtr_(nullptr),
    bufStr_(),
    //adiosPtr_(nullptr),
    adiosStreamPtr_(nullptr),
    compression_(IOstream::UNCOMPRESSED)
{
    if (pathname.empty())
    {
        if (IFstream::debug)
        {
            Info<< "IFstreamAllocator::IFstreamAllocator(const fileName&) : "
                    "cannot open null file " << endl;
        }
    }

    if (format == IOstream::COHERENT)
    {
        ifPtr_ = new std::istringstream();

        if ( !adiosStreamPtr_ ) //|| !adiosPtr_ )
        {
            allocateAdios();
        }

        // Get state from ADIOS
        const bool dataFound =
            readLocalString(bufStr_, pathname.name()); //pathname.caseName(""));

        if (!dataFound)
        {
            // Invalidate stream if the variable is not found
            ifPtr_->setstate(std::ios::failbit);
        }
        else
        {
            // Assign the buffer of string bufStr_ to the buffer of the stream
            ifPtr_->rdbuf()->pubsetbuf(&bufStr_[0], bufStr_.size());
        }
    }
    else
    {
        ifPtr_ = new ifstream(pathname.c_str());
    }

    // If the file is compressed, decompress it before reading.
    if (!ifPtr_->good() && isFile(pathname + ".gz", false))
    {
        if (IFstream::debug)
        {
            Info<< "IFstreamAllocator::IFstreamAllocator(const fileName&) : "
                    "decompressing " << pathname + ".gz" << endl;
        }

        delete ifPtr_;

        ifPtr_ = new igzstream((pathname + ".gz").c_str());

        if (ifPtr_->good())
        {
            compression_ = IOstream::COMPRESSED;
        }
    }

}


Foam::IFstreamAllocator::~IFstreamAllocator()
{
    delete ifPtr_;
}


void Foam::IFstreamAllocator::allocateAdios()
{
    //adiosPtr_.reset(new adiosRead());
    adiosStreamPtr_ = adiosReading{}.createStream();
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::IFstream::IFstream
(
    const fileName& pathname,
    streamFormat format,
    versionNumber version
)
:
    IFstreamAllocator(pathname, format),
    ISstream
    (
        *ifPtr_,
        "IFstream.sourceFile_",
        format,
        version,
        IFstreamAllocator::compression_
    ),
    pathname_(pathname),
    tmpIssPtr_(nullptr)
{
    setClosed();

    setState(ifPtr_->rdstate());

    if (!good())
    {
        if (debug)
        {
            Info<< "IFstream::IFstream(const fileName&,"
                   "streamFormat=ASCII,"
                   "versionNumber=currentVersion) : "
                   "could not open file for input"
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

Foam::IFstream::~IFstream()
{
    if (tmpIssPtr_)
    {
        delete tmpIssPtr_;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

std::istream& Foam::IFstream::stdStream()
{
    if (!ifPtr_)
    {
        FatalErrorIn("IFstream::stdStream()")
            << "No stream allocated" << abort(FatalError);
    }
    return *ifPtr_;
}


// read binary block from second stream
Foam::Istream& Foam::IFstream::parread(parIOType* buf, const string& blockId)
{
    if (format() != COHERENT)
    {
        FatalIOErrorIn("ISstream::parread(parIOType*, std::streamsize)", *this)
            << "stream format not parallel"
            << exit(FatalIOError);
    }

    if ( !adiosStreamPtr_ ) //|| !adiosPtr_ )
    {
        allocateAdios();
    }

    //adiosPtr_->read(buf, blockId);
    //adiosStreamPtr_->transfer(blockId, buf);

    return *this;
}


const std::istream& Foam::IFstream::stdStream() const
{
    if (!ifPtr_)
    {
        FatalErrorIn("IFstream::stdStream() const")
            << "No stream allocated" << abort(FatalError);
    }
    return *ifPtr_;
}


void Foam::IFstream::print(Ostream& os) const
{
    // Print File data
    os  << "IFstream: ";
    ISstream::print(os);
}


Foam::Istream& Foam::IFstream::readToStringStream(string& id)
{
    if (tmpIssPtr_)
    {

    }
    readLocalString(tmpIssBuf_, id);
    tmpIssPtr_ = new IStringStream(tmpIssBuf_);
    return *tmpIssPtr_;
}


// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //

Foam::IFstream& Foam::IFstream::operator()() const
{
    if (!good())
    {
        // also checks .gz file
        if (isFile(pathname_, true))
        {
            check("IFstream::operator()");
            FatalIOError.exit();
        }
        else
        {
            FatalIOErrorIn("IFstream::operator()", *this)
                << "file " << pathname_ << " does not exist"
                << exit(FatalIOError);
        }
    }

    return const_cast<IFstream&>(*this);
}


// ************************************************************************* //
