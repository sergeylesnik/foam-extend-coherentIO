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

Class
    Foam::IFCstream

Description
    Input from file stream and ADIOS for the coherent format. The stream builds
    a dictionary and alters its entries afterwards, filling the compound tokens
    (i.e. lists) and creating processorPatchFields since these are not present
    in the coherent format. GeometricField constructor obtains the ready-to-use
    dictionary.

SourceFiles
    IFCstream.C

\*---------------------------------------------------------------------------*/

#ifndef IFCstream_H
#define IFCstream_H

#include "ISstream.H"
#include "fileName.H"
#include "className.H"
#include "adiosFileStream.H"
#include "gzstream.h"
#include "sliceMesh.H"
#include "processorPolyPatch.H"

#include <fstream>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class adiosStream;

/*---------------------------------------------------------------------------*\
                      Class IFCstreamAllocator Declaration
\*---------------------------------------------------------------------------*/

//- A std::istream with ability to handle compressed files
class IFCstreamAllocator
{
    friend class IFCstream;

    // Private data

        istream* ifPtr_;

        //- String serves as buffer for file contents which are read only on
        //  master
        string bufStr_;

        //- Compression, e.g. ".gz"
        IOstream::compressionType compression_;


    // Constructors

        //- Construct from pathname
        IFCstreamAllocator
        (
            const fileName& pathname,
            const IOstream::streamFormat format
        );


    //- Destructor
    ~IFCstreamAllocator();

    // Member functions

        //- Allocate ADIOS object if not already present
        void allocateAdios();
};


/*---------------------------------------------------------------------------*\
                          Class IFCstream Declaration
\*---------------------------------------------------------------------------*/

class IFCstream
:
    public IFCstreamAllocator,
    public ISstream
{
    // Private data

        fileName pathname_;
        sliceMesh& sliceableMesh_;
        Istream* tmpIssPtr_;
        string tmpIssBuf_;

        //- Field dictionary populated by stream
        dictionary dict_;

        //- Pointer to the IO engine
        std::unique_ptr<adiosStream> adiosStreamPtr_;

    // Private Member Functions

        //- Add field of type processorPolyPatch to dictionary. To be
        //  specialized by PatchField and GeoMesh types
        template<template<class> class PatchField, class GeoMesh>
        void addProcessorPatchField
        (
            dictionary& bfDict,
            const word& patchName,
            const word& fieldTypeName
        );

        //- Get the field size from the corresponding mesh entity. To be
        //  specialized by PatchField and GeoMesh types
        template<template<class> class PatchField, class GeoMesh>
        label coherentFieldSize();

        // Read

            //- Find (recursively) all compound tokens in dictionary and
            //  populate them with data
            void readCompoundTokenData(dictionary& dict, const label size);

            //- Find all compound tokens in ITstream and populate them with
            //  data
            void readCompoundTokenData(ITstream& is, const label localSize);

            //- Read field data of all boundary patches that are not of
            //  processor type
            void readNonProcessorBoundaryFields();


protected:

    // Protected Member Functions

        //- Read a word token, identify and handle compound tokens if
        //  discovered
        virtual void readWordToken(token&);


public:

    // Static data members

        //- Runtime type information
        TypeName("IFCstream");


    // Constructors

        //- Construct from pathname and registry
        IFCstream
        (
            const fileName& pathname,
            const objectRegistry& registry,
            streamFormat format = COHERENT,
            versionNumber version = currentVersion
        );


    //- Destructor
    ~IFCstream();


    // Public Member Functions

        // Access

            //- Return the name of the stream
            const fileName& name() const
            {
                return pathname_;
            }

            //- Return non-const access to the name of the stream
            fileName& name()
            {
                return pathname_;
            }


        // Read functions

            //- Read to dictionary and add the missing subFields (e.g.
            //  processor patch fields).
            template<template<class> class PatchField, class GeoMesh>
            dictionary& readToDict(const word& fieldTypeName);


        // STL stream

            //- Access to underlying std::istream
            virtual istream& stdStream();

            //- Const access to underlying std::istream
            virtual const istream& stdStream() const;


        // Print

            //- Print description of IOstream to Ostream
            virtual void print(Ostream&) const;

            // ToDoIO Function used for reading of non-contiguous lists.
            // As is, it works only in serial. Reconsider.
            virtual Istream& readToStringStream(string&);


    // Member operators

        //- Return a non-const reference to const IFCstream
        //  Needed for read-constructors where the stream argument is temporary:
        //  e.g. thing thisThing(IFCstream("thingFileName")());
        IFCstream& operator()() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "IFCstreamI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
