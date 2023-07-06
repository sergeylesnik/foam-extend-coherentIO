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
#include "adiosFileStream.H"
#include "IStringStream.H"
#include "fileName.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(IFCstream, 0);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

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

void Foam::IFCstream::readCompoundTokenData(dictionary& dict, const label size)
{
    forAllIter(IDLList<entry>, dict, pIter)
    {
        entry& pEntry = pIter();

        if (pEntry.isDict())
        {
            readCompoundTokenData(pEntry.dict(), size);
        }

        if (debug)
        {
            Pout<< "Reading primitive entry " << nl << tab << pEntry << endl;
        }

        ITstream& is = pEntry.stream();
        readCompoundTokenData(is, size);
    }
}


void Foam::IFCstream::readCompoundTokenData
(
    ITstream& is,
    const label localSize
)
{
    // Traverse the tokens. If the current token is a compound token, resize
    // the compound token with the correct size. The id is stored in the next
    // token.
    while (!is.eof())
    {
        token currToken(is);

        // Resize the compoundTokens according to mesh of the proc and read the
        // corresponding slice of the data.
        if (currToken.isCompound())
        {
            token::compound& compToken = currToken.compoundToken();

            // Current token index points to the token after the compound
            // ToDoIO Get rid of globalSize?
            const label coherentStartI = is.tokenIndex();
            const label globalSize = is[coherentStartI].labelToken();
            const string id = is[coherentStartI + 1].stringToken();

            // Delete the coherent format tokens by resizing the tokenList
            is.resize(coherentStartI);

            // ToDoIO Get proper offsets directly from sliceableMesh_
            // elemOffset has to be int64
            //
            // By now:
            // // Internal field
            // const label elemOffset = sliceableMesh_.cellOffsets().front();
            // const label nElems = sliceableMesh_.cellOffsets().size();
            // const label nCmpts = compToken.nComponents();
            //
            // // Patches - doesn't work (set doReduce flag?)
            // Offsets patchOffsets(patch.size());
            // const label elemOffset = patchOffsets.front();
            // const label nElems = patchOffsets.size();

            const globalIndex gi(localSize);
            const label elemOffset = gi.offset(Pstream::myProcNo());
            const label nElems = gi.localSize();
            const label nCmpts = compToken.nComponents();

            compToken.resize(nElems);

            if (debug)
            {
                Pout
                    << "void Foam::IFCstream::readCompoundTokenData" << nl
                    << "(" << nl
                    << "    ITstream& is," << nl
                    << "    const label localSize" << nl
                    << ")" << nl
                    << "Reading compoundToken" << nl
                    << "    ITstream name = " << is.name() << nl
                    << "    ITstream globalSize = " << globalSize << nl
                    << "    ITstream id = " << id << nl
                    << "    elemOffset = " << elemOffset << nl
                    << "    nElems = " << nElems << nl
                    << "    compoundToken nComponents = " << nCmpts << nl
                    << "    ITstream tokens info:"
                    << endl;

                forAll(is, i)
                {
                    Pout<< tab << tab << is[i].info() << endl;
                }
            }

            // ToDoIO Provide a better interface from adiosStream for reading
            // of fields.
            adiosStreamPtr_->open("fields", pathname_.path());
            adiosStreamPtr_->transfer
            (
                id,
                reinterpret_cast<scalar*>(compToken.data()),
                List<label>({nCmpts*elemOffset}),
                List<label>({nCmpts*nElems})
            );
        }
    }
}


void Foam::IFCstream::readNonProcessorBoundaryFields()
{
    const polyMesh& mesh = sliceableMesh_.mesh();
    const polyBoundaryMesh& bm = mesh.boundaryMesh();
    dictionary& bfDict = dict_.subDict("boundaryField");

    forAll(bm, i)
    {
        const polyPatch& patch = bm[i];
        if (patch.type() != processorPolyPatch::typeName)
        {
            dictionary& patchDict = bfDict.subDict(patch.name());
            readCompoundTokenData(patchDict, patch.size());
        }
    }
}


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
    sliceableMesh_
    (
        const_cast<sliceMesh&>
        (
            registry.lookupObject<sliceMesh>(sliceMesh::typeName)
        )
    ),
    tmpIssPtr_(nullptr),
    adiosStreamPtr_(adiosReading{}.createStream())
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
