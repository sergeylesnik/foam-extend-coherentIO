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
void Foam::OFCstream<Type, Foam::fvsPatchField, Foam::surfaceMesh>::
combineCoherentInternal()
{

    const Offsets& iso = this->sliceableMesh_.internalSurfaceFieldOffsets();
    const label localCombinedSize = iso.count(Pstream::myProcNo());

    this->consolidatedData_.resize(localCombinedSize);

    // Internal
    const fieldDataEntry* internalFieldDataEntryPtr =
        dynamic_cast<const fieldDataEntry*>
        (
            this->dict_.lookupEntryPtr("internalField", false, false)
        );

    const UList<Type>& internalData =
        dynamic_cast<const UList<Type>&>(internalFieldDataEntryPtr->uList());


    // Processor patches
    const polyBoundaryMesh& bm = sliceableMesh_.mesh().boundaryMesh();
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
    const labelList& pf = this->sliceableMesh_.internalFaceIDsFromBoundaries();
    const label nProcFaces = pf.size();

    // processorFacesPatchIds
    const labelList& pfpi = this->sliceableMesh_.boundryIDsFromInternalFaces();

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
    IOstream::streamFormat format,
    IOstream::versionNumber version,
    IOstream::compressionType compression
)
:
    OFCstreamBase
    (
        pathname,
        registry,
        mode,
        format,
        version,
        compression
    )
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
