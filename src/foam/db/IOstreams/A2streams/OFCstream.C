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

#include "OFCstream.H"
#include "dictionaryEntry.H"
#include "formattingEntry.H"

#include "adiosStream.H"
#include "processorPolyPatch.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

// defineTypeNameAndDebug(Foam::OFCstreamBase, 0);

// * * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

void Foam::OFCstreamBase::gatherFieldDataEntries
(
    dictionary& dict,
    DynamicList<fieldDataEntry*>& fieldDataEntries
)
{
    forAllIter(IDLList<entry>, dict, iter)
    {
        entry& e = iter();

        if (isA<fieldDataEntry>(e))
        {
            fieldDataEntry* fde = dynamic_cast<fieldDataEntry*>(&e);
            fieldDataEntries.append(fde);
        }
        else if (e.isDict())
        {
            string name = e.name();
            gatherFieldDataEntries(e.dict(), fieldDataEntries);
        }
    }
}


void Foam::OFCstreamBase::removeProcPatchesFromDict()
{
    const polyBoundaryMesh& bm = sliceableMesh_.mesh().boundaryMesh();
    dictionary& bfDict = dict_.subDict("boundaryField");

    forAll(bm, i)
    {
        const polyPatch& patch = bm[i];

        if (patch.type() == processorPolyPatch::typeName)
        {
            bfDict.remove(patch.name());
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::OFCstreamBase::OFCstreamBase
(
    const fileName& pathname,
    const objectRegistry& registry,
    ios_base::openmode mode,
    streamFormat format,
    versionNumber version,
    compressionType compression
)
:
    OFstream(pathname, mode, format, version, compression),
    pathname_(pathname),
    sliceableMesh_
    (
        const_cast<sliceMesh&>
        (
            registry.lookupObject<sliceMesh>(sliceMesh::typeName)
        )
    ),
    dict_(pathname.name()),
    currentSubDictPtr_(&dict_),
    currentKeyword_(),
    currentEntryI_(0)
{}


// * * * * * * * * * * * * * * * * Destructors * * * * * * * * * * * * * * * //

Foam::OFCstreamBase::~OFCstreamBase()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::string
Foam::OFCstreamBase::getBlockId() const
{
    return getGlobalId();
}


Foam::string
Foam::OFCstreamBase::getGlobalId() const
{
    const fileName id = OFstream::getBlockId();
    const wordList cmpts = id.components();
    fileName globalId;

    forAll(cmpts, i)
    {
        if (cmpts[i].find("processor") == std::string::npos)
        {
            globalId = globalId/cmpts[i];
        }
    }

    return globalId;
}


Foam::Ostream&
Foam::OFCstreamBase::writeKeyword(const keyType& kw)
{
    currentKeyword_ = kw;
    moveStreamBufferToDict();

    return *this;
}


Foam::Ostream&
Foam::OFCstreamBase::write(const token& t)
{
    if (t.isPunctuation())
    {
        if (t.pToken() == token::punctuationToken::END_STATEMENT)
        {
            ostream& buf = getStreamBuffer();
            std::ostringstream& oss = dynamic_cast<std::ostringstream&>(buf);
            const word str = oss.str();
            currentSubDictPtr_->add(currentKeyword_, str);
            oss.str(string());
        }
    }

    return *this;
}


Foam::Ostream&
Foam::OFCstreamBase::write(const word& str)
{
    if (token::compound::isCompound(str))
    {
        currentCompoundTokenName_ = str;

        return *this;
    }

    return OSstream::write(str);
}


Foam::word Foam::OFCstreamBase::incrBlock(const word name)
{
    // Save the data written to stream before
    moveStreamBufferToDict();

    const dictionary subDict(name);
    currentSubDictPtr_->add(name, subDict);
    currentSubDictPtr_ =
        const_cast<dictionary*>(currentSubDictPtr_->subDictPtr(name));

    return name;
}


void Foam::OFCstreamBase::decrBlock()
{
    // Save the data written to stream before
    moveStreamBufferToDict();

    currentSubDictPtr_ = &const_cast<dictionary&>(currentSubDictPtr_->parent());
}


Foam::Ostream&
Foam::OFCstreamBase::parwrite(uListProxyBase* uListProxyPtr)
{
    currentSubDictPtr_->add
    (
        new fieldDataEntry
        (
            currentKeyword_,
            currentCompoundTokenName_,
            uListProxyPtr
        )
    );

    return *this;
}


void Foam::OFCstreamBase::moveStreamBufferToDict()
{
    ostream& buf = getStreamBuffer();
    std::ostringstream& oss = dynamic_cast<std::ostringstream&>(buf);
    const string str = oss.str();
    if (!str.empty())
    {
        token t = str;
        t.type() = token::WORD;
        const word cei = "ASCII_F" + std::to_string(currentEntryI_);
        currentSubDictPtr_->add(new formattingEntry(cei, t));
        currentEntryI_++;
        oss.str(string());
    }
}


void Foam::OFCstreamBase::writeDict
(
    Ostream& os,
    const dictionary& dict,
    bool subDict
)
const
{
    if (subDict)
    {
        os << nl;
        os.indent();
        os << token::BEGIN_BLOCK;
        os.incrIndent();
        os << nl;
    }

    forAllConstIter(IDLList<entry>, dict, iter)
    {
        const entry& e = *iter;

        if (isA<dictionaryEntry>(e))
        {
            os.indent();
            os.write(e.keyword());
            writeDict(os, e.dict(), true);
        }
        else if (isA<primitiveEntry>(e))
        {
            const keyType& key = e.keyword();

            // ToDoIO Better identification of formatting entries?
            if (regExp("ASCII_F.*").match(key))
            {
                os << e;
            }
            else
            {
                // Write without new lines which are handled by
                // formattingEntries
                const primitiveEntry& pe = dynamicCast<const primitiveEntry>(e);
                os.writeKeyword(key);
                pe.write(os, true);
                // ToDoIO Let formattingEntry take care over semicolon?
                // os << token::END_STATEMENT;
            }
        }
        else  // fieldDataEntry
        {
            os << e;
        }

        // Check stream before going to next entry.
        if (!os.good())
        {
            WarningInFunction
                << "Can't write entry " << iter().keyword()
                << " for dictionary " << dict.name()
                << nl;
        }
    }

    if (subDict)
    {
        os.decrIndent();
        os.indent();
        os << token::END_BLOCK << nl;
    }
}


void Foam::OFCstreamBase::writeGlobalGeometricField()
{
    DynamicList<fieldDataEntry*> fieldDataEntries;
    gatherFieldDataEntries(dict_, fieldDataEntries);

    const label nFields = fieldDataEntries.size();
    List<fieldTag> globalUniformity(nFields);

    // Prepare list with field tags for global reduce
    forAll(fieldDataEntries, i)
    {
        globalUniformity[i] = fieldDataEntries[i]->tag();
    }

    globalUniformity =
        returnReduce(globalUniformity, fieldTag::uniformityCompareOp);

    forAll(fieldDataEntries, i)
    {
        fieldDataEntry& fde = *(fieldDataEntries[i]);
        fde.tag() = globalUniformity[i];

        // ToDoIO Check whether the fde field size equals the size of the
        // corresponding mesh entity on each rank. Allreduce this info using
        // the uniformityCompareOp infrastructure because the field size may
        // accidentally equal the size of the mesh entity. If it's not equal on
        // all ranks, do not agglomerate and write to ADIOS with prefixed
        // processorXX.
        if (!fde.uniform())
        {
            const label nElems = fde.uList().size(); //nElems();
            const label nCmpts = fde.uList().nComponents();

            const globalIndex bgi(nElems);
            const label nGlobalElems = bgi.size();
            const label elemOffset = bgi.offset(Pstream::myProcNo());

            // Write to engine
            auto adiosStreamPtr = Foam::adiosWriting{}.createStream();
            adiosStreamPtr->open("fields", pathname_.path());
            adiosStreamPtr->transfer
            (
                fde.id(),
                {nCmpts*nGlobalElems},
                {nCmpts*elemOffset},
                {nCmpts*nElems},
                reinterpret_cast<const scalar*>(fde.uList().cdata())
            );
            adiosStreamPtr->close();

            fde.nGlobalElems() = nGlobalElems;
        }
    }

    if (Pstream::master())
    {
        OFstream of
        (
            name(),
            ios_base::out|ios_base::trunc,
            streamFormat::ASCII
        );

        moveStreamBufferToDict();

        // ToDoIO keep formattingEntry?
        // dict_.write(of, false);
        writeDict(of, dict_, false);
    }
}

// ************************************************************************* //
