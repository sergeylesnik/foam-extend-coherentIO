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

InClass
    Foam::surfaceFields

Description
    Foam::surfaceFields

SourceFiles
    surfaceFields.C

\*---------------------------------------------------------------------------*/

#ifndef surfaceFields_H
#define surfaceFields_H

#include "objectRegistry.H"
#include "GeometricFields.H"
#include "surfaceMesh.H"
#include "fvMesh.H"
#include "fvsPatchFields.H"
#include "surfaceFieldsFwd.H"
#include "calculatedFvsPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
class OFCstream<Type, fvsPatchField, surfaceMesh>
:
public OFCstreamBase
{

    // Private data

        //- Internal field of the coherent format with processor patch fields
        //  mapped into it
        List<Type> consolidatedData_;


    // Private member functions

        void combineCoherentInternal();


public:

    //- Constructor
    OFCstream
    (
        const fileName& pathname,
        const objectRegistry& registry,
        ios_base::openmode mode = ios_base::out|ios_base::trunc,
        IOstream::streamFormat format = IOstream::ASCII,
        IOstream::versionNumber version = IOstream::currentVersion,
        IOstream::compressionType compression = IOstream::UNCOMPRESSED
    );


    //- Destructor
    virtual ~OFCstream();
};

template<>
label IFCstream::coherentFieldSize<fvsPatchField, surfaceMesh>();

template<>
dictionary& IFCstream::readToDict<fvsPatchField, surfaceMesh>
(
    const word& fieldTypeName
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "surfaceFieldsI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
