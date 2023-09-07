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

template<class Type>
void Foam::IFCstream::reader<Type, Foam::fvsPatchField, Foam::surfaceMesh>::read
(
    IFCstream& ifs
)
{
    ifs.readNonProcessorBoundaryFields<Type>();

    // Take care internal and processor fields. Internal surface field in
    // coherent format includes processor boundaries. If it is uniform, the
    // processor patches need to be created with the same uniform entry.
    // Otherwise, the coherent internal field needs to be mapped to FOAM's
    // internal and processor fields.

    typedef typename pTraits<Type>::cmptType cmptType;
    const polyMesh& mesh = ifs.coherentMesh_.mesh();
    const polyBoundaryMesh& bm = mesh.boundaryMesh();

    // Internal field data
    UList<Type> internalData;

    // Field data in the coherent format holding internal and processor patch
    // fields
    List<Type> coherentData;

    ITstream& its = ifs.dict_.lookup("internalField");
    dictionary& bfDict = ifs.dict_.subDict("boundaryField");

    // Traverse the tokens of the internal field entry
    while (true)
    {
        if (its.eof())
        {
            FatalErrorInFunction
                << "Expected 'uniform' or compoundToken in " << its
                << nl << "    in file " << ifs.pathname_
                << abort(FatalError);
        }

        token currToken(its);

        if (currToken.isCompound()) // non-uniform
        {
            // Resize the compoundToken according to the mesh of the proc
            token::compound& compToken = currToken.compoundToken();
            compToken.resize(mesh.nInternalFaces());

            // Store the data pointer in a UList for convenience
            internalData = UList<Type>
            (
                reinterpret_cast<Type*>(compToken.data()),
                mesh.nInternalFaces()
            );

            // Current token index points to the token after the compound
            // ToDoIO Get rid of globalSize?
            const label globalSize = its[its.tokenIndex()++].labelToken();
            const string id = its[its.tokenIndex()++].stringToken();
            if (false)
            {
                Info<< globalSize;
            }  // Silence compiler warning

            // Internal surface field in coherent format includes processor
            // boundaries. Thus, find out the corresponding size.
            const label localSize =
                ifs.coherentFieldSize<fvsPatchField, surfaceMesh>();
            const globalIndex gi(localSize);
            const label elemOffset = gi.offset(Pstream::myProcNo());
            const label nElems = gi.localSize();
            const label nCmpts = compToken.nComponents();

            coherentData.resize(localSize);

            ifs.sliceStreamPtr_->access("fields", ifs.pathname_.path());
            ifs.sliceStreamPtr_->get
            (
                id,
                reinterpret_cast<cmptType*>(coherentData.data()),
                List<label>({nCmpts*elemOffset}),
                List<label>({nCmpts*nElems})
            );

            // Syncronizing IO engine ensures that the data is read from storage
            ifs.sliceStreamPtr_->bufferSync();

            break;
        }
        else if (currToken.isWord() && currToken.wordToken() == "uniform")
        {
            // Create processor patch fields with the same uniform entry as the
            // internal field.

            forAll(bm, i)
            {
                const polyPatch& patch = bm[i];
                const word& patchName = patch.name();

                // ToDoIO Make all the if(ppPatch) conditions consistent
                if (patch.type() == processorPolyPatch::typeName)
                {
                    dictionary dict;
                    dict.add("type", "processor");
                    dict.add("value", its);
                    bfDict.add(patchName, dict);
                }
            }

            // Closing the IO engine ensures that the data is read from storage
            ifs.sliceStreamPtr_->bufferSync();

            return;
        }
    }


    // The internal coherent field is non-uniform.

    // Create dictionary entries for the processor patch fields with the
    // correctly sized compound tokens.

    const label nAllPatches = bm.size();
    // ToDoIO Make it work with any type
    List<UList<Type> > procPatchData(nAllPatches);
    label nProcPatches = 0;

    forAll(bm, patchI)
    {
        const polyPatch& patch = bm[patchI];

        if (isA<processorPolyPatch>(patch))
        {
            const label patchSize = patch.size();

            tokenList entryTokens(2);
            entryTokens[0] = word("nonuniform");

            // Create a compound token for the patch, resize and store in a list
            autoPtr<token::compound> ctPtr =
                token::compound::New
                (
                    "List<" + word(pTraits<Type>::typeName) + '>',
                    patchSize
                );

            // Store the data pointer for the later mapping
            procPatchData[nProcPatches++] =
                UList<Type>
                (
                    reinterpret_cast<Type*>(ctPtr().data()),
                    patchSize
                );

            // autoPtr is invalid after calling ptr()
            entryTokens[1] = ctPtr.ptr();
            dictionary dict;
            dict.add("type", "processor");
            // Xfer needed here, calls the corresponding primitiveEntry
            // constructor
            dict.add("value", entryTokens.xfer());
            // No Xfer implemented for adding dictionary but copying is
            // accomplished by cloning the pointers of the hash table and
            // IDLList
            bfDict.add(patch.name(), dict);
        }
    }


    // Map from the coherent format to the internal and processor patch fields

    procPatchData.resize(nProcPatches);
    const label nNonProcPatches = nAllPatches - nProcPatches;

    // processorFaces
    const labelList& pf = ifs.coherentMesh_.internalFaceIDsFromBoundaries();

    // processorFacesPatchIds
    const labelList& pfpi = ifs.coherentMesh_.boundryIDsFromInternalFaces();

    labelList patchFaceI(nProcPatches, 0);
    label internalFaceI = 0;
    label pfI = 0;

    // Closing the IO engine ensures that the data is read from disk
    ifs.sliceStreamPtr_->bufferSync();

    if (pf.empty())
    {
        // Use explicit assign(). An assignment operator would trigger a
        // shallow copy by setting internalData data pointer to that of
        // coherentData. But the latter is destroyed after returning and the
        // data pointer would not be valid anymore.
        internalData.assign(coherentData);
    }
    else
    {
        forAll(coherentData, i)
        {
            if (pfI < pf.size() && i == pf[pfI])  // Processor field
            {
                const label patchI = pfpi[pfI++] - nNonProcPatches;
                procPatchData[patchI][patchFaceI[patchI]++] = coherentData[i];
            }
            else  // Internal field
            {
                internalData[internalFaceI++] = coherentData[i];
            }
        }
    }

    // In coherent format only the lower neighbour procs have the processor
    // faces and the corresponding fields. Get their values to the procs
    // above.

    // Send
    forAll(bm, patchI)
    {
        const polyPatch& patch = bm[patchI];

        if (isA<processorPolyPatch>(patch))
        {
            const processorPolyPatch& procPp =
                refCast<const processorPolyPatch>(patch);

            if (procPp.neighbProcNo() > procPp.myProcNo())
            {
                OPstream toProcNbr(Pstream::blocking, procPp.neighbProcNo());
                toProcNbr << procPatchData[patchI - nNonProcPatches];
            }
        }
    }

    // Receive
    forAll(bm, patchI)
    {
        const polyPatch& patch = bm[patchI];

        if (isA<processorPolyPatch>(patch))
        {
            const processorPolyPatch& procPp =
                refCast<const processorPolyPatch>(patch);

            if (procPp.neighbProcNo() < procPp.myProcNo())
            {
                const label patchDataI = patchI - nNonProcPatches;
                IPstream fromProcNbr(Pstream::blocking, procPp.neighbProcNo());
                fromProcNbr >> procPatchData[patchDataI];

                // Flip the sign since the faces are oriented in the opposite
                // direction
                forAll(procPatchData[patchDataI], faceI)
                {
                    procPatchData[patchDataI][faceI] *= -1;
                }
            }
        }
    }

    return;
}


template<class Type>
void Foam::OFCstream<Type, Foam::fvsPatchField, Foam::surfaceMesh>::
combineCoherentInternal()
{

    const Offsets& iso = this->coherentMesh_.internalSurfaceFieldOffsets();
    const label localCombinedSize = iso.count(Pstream::myProcNo());

    this->consolidatedData_.resize(localCombinedSize);

    // Internal
    const fieldDataEntry* internalFieldDataEntryPtr =
        dynamic_cast<const fieldDataEntry*>
        (
            this->dict_.lookupEntryPtr("internalField", false, false)
        );

    if (!internalFieldDataEntryPtr)
    {
        FatalErrorInFunction
            << "Entry 'internalField' not found in dictionary "
            << dict_.name()
            << abort(FatalError);
    }

    const UList<Type>& internalData =
        dynamic_cast<const UList<Type>&>(internalFieldDataEntryPtr->uList());


    // Processor patches
    const polyBoundaryMesh& bm = coherentMesh_.mesh().boundaryMesh();
    const label nPatches = bm.size();
    dictionary& bfDict = dict_.subDict("boundaryField");
    List<UList<Type> > patchData(nPatches);
    List<const fieldDataEntry*> procPatchFDEPtrs(nPatches);

    label ppI = 0;
    forAll(bm, i)
    {
        const polyPatch& patch = bm[i];
        const word& patchName = patch.name();

        if (patch.type() == processorPolyPatch::typeName)
        {
            const dictionary& ppDict = bfDict.subDict(patchName);
            procPatchFDEPtrs[ppI] =
                dynamic_cast<const fieldDataEntry*>
                (
                    ppDict.lookupEntryPtr("value", false, false)
                );
            patchData[ppI] =
                dynamic_cast<const UList<Type>&>
                (
                    procPatchFDEPtrs[ppI]->uList()
                );
            ppI++;
        }
    }

    const label nProcPatches = ppI;
    patchData.resize(nProcPatches);
    procPatchFDEPtrs.resize(nProcPatches);
    const label nNonProcPatches = nPatches - nProcPatches;

    // processorFaces
    const labelList& pf = this->coherentMesh_.internalFaceIDsFromBoundaries();
    const label nProcFaces = pf.size();

    // processorFacesPatchIds
    const labelList& pfpi = this->coherentMesh_.boundryIDsFromInternalFaces();

    label internalFaceI = 0;
    label pfI = 0;
    labelList patchFaceI(nProcPatches, 0);
    if (pf.empty())
    {
        this->consolidatedData_ = internalData;
    }
    else
    {
        for (label i = 0; i < this->consolidatedData_.size(); i++)
        {
            if (pfI < nProcFaces && i == pf[pfI])  // Processor field
            {
                const label patchI = pfpi[pfI++] - nNonProcPatches;
                this->consolidatedData_[i] =
                    patchData[patchI][patchFaceI[patchI]++];
            }
            else  // Internal field
            {
                this->consolidatedData_[i] = internalData[internalFaceI++];
            }
        }
    }

    // Create new entry for the consolidated internalField
    fieldDataEntry* coherentInternal =
    new fieldDataEntry
    (
        "internalField",
        internalFieldDataEntryPtr->compoundTokenName(),
        new UListProxy<Type>(consolidatedData_)
    );

    // Set the new internalField in the dictionary replacing the old one
    this->dict_.set(coherentInternal);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::OFCstream<Type, Foam::fvsPatchField, Foam::surfaceMesh>::OFCstream
(
    const fileName& pathname,
    const objectRegistry& registry,
    ios_base::openmode mode,
    IOstreamOption streamOpt
)
:
    OFCstreamBase(pathname, registry, mode, streamOpt)
{}


// * * * * * * * * * * * * * * * * Destructors * * * * * * * * * * * * * * * //

template<class Type>
Foam::OFCstream<Type, Foam::fvsPatchField, Foam::surfaceMesh>::~OFCstream()
{
    combineCoherentInternal();
    this->removeProcPatchesFromDict();
    this->writeGlobalGeometricField();
}


// ************************************************************************* //
