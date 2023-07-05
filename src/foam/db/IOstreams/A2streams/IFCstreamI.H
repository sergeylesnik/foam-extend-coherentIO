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


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<template<class> class PatchField, class GeoMesh>
Foam::dictionary& Foam::IFCstream::readToDict(const word& fieldTypeName)
{
    // Fill the dictionary with the stream
    dict_.read(*this);

    ITstream& is = dict_.lookup("internalField");
    readCompoundTokenData(is, coherentFieldSize<PatchField, GeoMesh>());

    readNonProcessorBoundaryFields();

    dictionary& bfDict = dict_.subDict("boundaryField");

    forAll(sliceableMesh_.mesh().boundaryMesh(), i)
    {
        const polyPatch& patch = sliceableMesh_.mesh().boundaryMesh()[i];
        const word& patchName = patch.name();

        if (patch.type() == processorPolyPatch::typeName)
        {
            addProcessorPatchField<PatchField, GeoMesh>
            (
                bfDict, patchName, fieldTypeName
            );
        }
    }

    // Closing the IO engine ensures that the data is read from storage
    adiosStreamPtr_->close();
    return dict_;
}


// ************************************************************************* //
