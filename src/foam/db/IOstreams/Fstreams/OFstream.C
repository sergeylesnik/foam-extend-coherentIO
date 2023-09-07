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

#include "OFstream.H"
#include "OSspecific.H"
#include "OStringStream.H"
#include "UList.H"
#include "Pstream.H"
#include "gzstream.h"
#include "messageStream.H"
#include <iostream>
#include <memory>

#include "SliceStreamRepo.H"
#include "sliceWritePrimitives.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(Foam::OFstream, 0);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

Foam::OFstreamAllocator::OFstreamAllocator
(
    const fileName& pathname,
    ios_base::openmode mode,
    IOstream::streamFormat format,
    IOstream::compressionType compression
)
:
    ofPtr_(nullptr)
{
    if (pathname.empty())
    {
        if (OFstream::debug)
        {
            Info<< "OFstreamAllocator::OFstreamAllocator(const fileName&) : "
                   "cannot open null file " << endl;
        }
    }

    if (format == IOstream::COHERENT)
    {
        // The file pointer is a buffer pointer in this case.
        // The buffer is written to ADIOS in the destructor.
        ofPtr_ = new std::ostringstream();
    }
    else
    {
        if (compression == IOstream::COMPRESSED)
        {
            // get identically named uncompressed version out of the way
            if (isFile(pathname, false))
            {
                rm(pathname);
            }

            ofPtr_ = new ogzstream((pathname + ".gz").c_str(), mode);
        }
        else
        {
            // get identically named compressed version out of the way
            if (isFile(pathname + ".gz", false))
            {
                rm(pathname + ".gz");
            }

            ofPtr_ = new ofstream(pathname.c_str(), mode);
        }
    }
}


Foam::OFstreamAllocator::~OFstreamAllocator()
{
    delete ofPtr_;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::OFstream::OFstream
(
    const fileName& pathname,
    ios_base::openmode mode,
    IOstreamOption streamOpt
)
:
    OFstreamAllocator
    (
        pathname,
        mode,
        streamOpt.format(),
        streamOpt.compression()
    ),
    OSstream(*ofPtr_, "OFstream.sinkFile_", streamOpt),
    pathname_(pathname),
    blockNamesStack_(),
    tmpOssPtr_(nullptr)
{
    if (debug)
    {
        InfoInFunction
            << "Constructing a stream with format " << streamOpt.format()
            << Foam::endl;
    }
    setClosed();
    setState(ofPtr_->rdstate());

    if (!good())
    {
        if (debug)
        {
            Info<< "IFstream::IFstream(const fileName&,"
                   "streamFormat format=ASCII,"
                   "versionNumber version=currentVersion) : "
                   "could not open file for input\n"
                   "in stream " << info() << Foam::endl;
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
Foam::OFstream::~OFstream()
{
    if (isA<std::ostringstream>(*ofPtr_) && format() == IOstream::COHERENT)
    {
        std::ostringstream& os = dynamic_cast<std::ostringstream&>(*ofPtr_);

        if (!os.str().empty())
        {
            writeLocalString( getRelativeFileName(),
                              os.str().data(),
                              os.str().size() );
        }
    }

    if (tmpOssPtr_)
    {
        writeLocalString( getBlockId(),
                          tmpOssPtr_->str().data(),
                          tmpOssPtr_->str().size() );
        delete tmpOssPtr_;
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::string Foam::OFstream::getBlockId() const
{
    fileName id;
    bool firstEntry = true;

    forAllConstIter(SLList<word>, blockNamesStack_, iter)
    {
        if (firstEntry)
        {
            id = iter();
            firstEntry = false;
        }
        else
        {
            id = iter() + '/' + id;
        }
    }

    id = getRelativeFileName() + '/' + id;

    if(debug > 1)
    {
        Pout
            << "Block id = " << id << "; blockNamesStack_.size() = "
            << blockNamesStack_.size() << '\n';
    }

    return id;
}


std::ostream& Foam::OFstream::stdStream()
{
    if (!ofPtr_)
    {
        FatalErrorIn("OFstream::stdStream()")
            << "No stream allocated." << abort(FatalError);
    }
    return *ofPtr_;
}


const std::ostream& Foam::OFstream::stdStream() const
{
    if (!ofPtr_)
    {
        FatalErrorIn("OFstreamAllocator::stdStream() const")
            << "No stream allocated." << abort(FatalError);
    }
    return *ofPtr_;
}


void Foam::OFstream::print(Ostream& os) const
{
    os  << "    OFstream: ";
    OSstream::print(os);
}


Foam::word Foam::OFstream::incrBlock(const word name)
{
    blockNamesStack_.push(name);

    if(debug > 1)
    {
        Pout
            << "Add word to the block name LIFO stack: " << name << '\n';
    }

    OSstream::indent();
    OSstream::write(name);
    OSstream::write(nl);
    OSstream::indent();
    OSstream::write(char(token::BEGIN_BLOCK));
    OSstream::write(nl);
    OSstream::incrIndent();

    return name;
}


void Foam::OFstream::decrBlock()
{
    if (debug > 1)
    {
        Info<< "OFsteam::decrBlock" << '\n';
    }

    popBlockNamesStack();

    OSstream::decrIndent();
    OSstream::indent();
    OSstream::write(char(token::END_BLOCK));
}


Foam::Ostream& Foam::OFstream::writeKeyword(const keyType& kw)
{
    if (debug > 1)
    {
        Pout
            << "Add keyType to the block name LIFO stack: " << kw << "\n";
    }

    blockNamesStack_.push(kw);
    // Inform SliceStreamRepo that n'th boundary is being written.
    if( kw == "type" ) {
       ++boundaryCounter_;
       Foam::SliceStreamRepo* repo = Foam::SliceStreamRepo::instance();
       repo->push( boundaryCounter_ );
    }

    return this->Ostream::writeKeyword(kw);
}


void Foam::OFstream::popBlockNamesStack()
{
    if (blockNamesStack_.empty())
    {
        WarningInFunction
            << "Tried to pop the stack although its empty.\n";
    }
    else
    {
        if(debug > 1)
        {
            Pout
                << "Pop block name LIFO stack: "
                << blockNamesStack_.first() << '\n';
        }

        blockNamesStack_.pop();
    }
}


Foam::Ostream& Foam::OFstream::write
(
    const char* data,
    std::streamsize byteSize
)
{
    if (format() == COHERENT)
    {
        sliceWritePrimitives
        (
            "fields",
            "",
            this->getBlockId(),
            byteSize/sizeof(scalar),
            reinterpret_cast<const scalar*>(data)
        );
    }
    else
    {
        OSstream::write(data, byteSize);
    }

    return *this;
}


Foam::Ostream& Foam::OFstream::parwrite(std::unique_ptr<uListProxyBase> uListProxyPtr)
{
    if (format() != COHERENT)
    {
        FatalIOErrorIn("Ostream::parwrite(const parIOType*, const label)", *this)
            << "stream format not parallel"
            << abort(FatalIOError);
    }

    string blockId = getBlockId();

    return *this;
}


Foam::Ostream& Foam::OFstream::stringStream()
{
    if (format() != COHERENT)
    {
        FatalIOErrorIn("Ostream::stringStream()", *this)
            << " stream format not parallel"
            << abort(FatalIOError);
    }

    if (tmpOssPtr_)
    {
        FatalIOErrorIn("Ostream::stringStream()", *this)
            << " a string stream object already exists"
            << abort(FatalIOError);
    }

    tmpOssPtr_ = new OStringStream();
    return *tmpOssPtr_;
}


void Foam::OFstream::writeLocalString
(
    const Foam::fileName& varName,
    const Foam::string& str,
    const Foam::label size
)
{
    if (Foam::Pstream::master())
    {
        std::ofstream outFile;
        outFile.open
        (
            varName, ios_base::out|ios_base::trunc
        );
        outFile << str;
        outFile.close();
    }
}


// ************************************************************************* //
