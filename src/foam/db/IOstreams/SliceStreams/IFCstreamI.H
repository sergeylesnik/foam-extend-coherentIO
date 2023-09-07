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

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<template<class> class PatchField, class GeoMesh>
void Foam::IFCstream::addProcessorPatchField
(
    dictionary& bfDict,
    const word& patchName,
    const word& fieldTypeName
)
{
    NotImplemented;
}


template<template<class> class PatchField, class GeoMesh>
Foam::label Foam::IFCstream::coherentFieldSize()
{
    NotImplemented;

    return 0;
}


template<class Type>
void Foam::IFCstream::readCompoundTokenData(dictionary& dict, const label size)
{
    forAllIter(IDLList<entry>, dict, pIter)
    {
        entry& pEntry = pIter();

        if (pEntry.isDict())
        {
            readCompoundTokenData<Type>(pEntry.dict(), size);
        }

        if (debug)
        {
            Pout<< "Reading primitive entry " << nl << tab << pEntry << endl;
        }

        ITstream& is = pEntry.stream();
        readCompoundTokenData<Type>(is, size);
    }
}


template<class Type>
void Foam::IFCstream::readCompoundTokenData
(
    ITstream& is,
    const label localSize
)
{
    typedef typename pTraits<Type>::cmptType cmptType;

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

            // ToDoIO Get proper offsets directly from coherentMesh_
            // elemOffset has to be int64
            //
            // By now:
            // // Internal field
            // const label elemOffset = coherentMesh_.cellOffsets().front();
            // const label nElems = coherentMesh_.cellOffsets().size();
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

            // ToDoIO Provide a better interface from SliceStream for reading
            // of fields.
            sliceStreamPtr_->access("fields", pathname_.path());
            sliceStreamPtr_->get
            (
                id,
                reinterpret_cast<cmptType*>(compToken.data()),
                List<label>({nCmpts*elemOffset}),
                List<label>({nCmpts*nElems})
            );
        }
    }
}


template<class Type>
void Foam::IFCstream::readNonProcessorBoundaryFields()
{
    const polyMesh& mesh = coherentMesh_.mesh();
    const polyBoundaryMesh& bm = mesh.boundaryMesh();
    dictionary& bfDict = dict_.subDict("boundaryField");

    forAll(bm, i)
    {
        const polyPatch& patch = bm[i];
        if (patch.type() != processorPolyPatch::typeName)
        {
            dictionary& patchDict = bfDict.subDict(patch.name());
            readCompoundTokenData<Type>(patchDict, patch.size());
        }
    }
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::IFCstream::reader<Type, PatchField, GeoMesh>::read
(
    IFCstream& ifs
)
{
    Info<< "Non-special" << endl;
    ITstream& its = ifs.dict_.lookup("internalField");
    ifs.readCompoundTokenData<Type>
    (
        its,
        ifs.coherentFieldSize<PatchField, GeoMesh>()
    );

    ifs.readNonProcessorBoundaryFields<Type>();

    dictionary& bfDict = ifs.dict_.subDict("boundaryField");

    forAll(ifs.coherentMesh_.mesh().boundaryMesh(), i)
    {
        const polyPatch& patch = ifs.coherentMesh_.mesh().boundaryMesh()[i];
        const word& patchName = patch.name();

        if (patch.type() == processorPolyPatch::typeName)
        {
            ifs.addProcessorPatchField<PatchField, GeoMesh>
            (
                bfDict, patchName, pTraits<Type>::typeName
            );
        }
    }

    // Ensure that the data is read from storage
    ifs.sliceStreamPtr_->bufferSync();

    return;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type, template<class> class PatchField, class GeoMesh>
Foam::dictionary& Foam::IFCstream::readToDict()
{
    // Fill the dictionary with the stream
    dict_.read(*this);

    reader<Type, PatchField, GeoMesh>::read(*this);

    return dict_;
}


// ************************************************************************* //
