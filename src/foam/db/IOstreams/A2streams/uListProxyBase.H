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
    Foam::uListProxyBase

Description
    Interface to UList<T> abstracting type T.

SourceFiles
    uListProxyBase.C

\*---------------------------------------------------------------------------*/

#ifndef uListProxyBase_H
#define uListProxyBase_H

#include "label.H"
#include "scalar.H"
#include <ios>  // std::streamsize

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class Ostream;

/*---------------------------------------------------------------------------*\
                           Class uListProxyBase Declaration
\*---------------------------------------------------------------------------*/

class uListProxyBase
{
public:

    // Public types

        enum uniformity
        {
            EMPTY,
            UNIFORM,
            NONUNIFORM
        };

    // Constructors

        //- Default construct
        uListProxyBase()
        {}


    //- Destructor
    virtual ~uListProxyBase()
    {}


    // Member Functions

        //- Write the first element of UList.
        virtual void writeFirstElement(Ostream&, const char*) const = 0;


    // Byte-wise Access

        //- Return pointer to the underlying array serving as data storage,
        //  reinterpreted as byte data
        //  \note Only meaningful for contiguous data
        virtual inline const char* cdata_bytes() const = 0;

        //- Return pointer to the underlying array serving as data storage,
        //  reinterpreted as byte data
        //  \note Only meaningful for contiguous data
        virtual inline char* data_bytes() = 0;

        //- Number of contiguous bytes for the List data.
        //  \note Only meaningful for contiguous data
        virtual inline std::streamsize size_bytes() const = 0;


        // Access

            //- Return the binary size in number of characters of the UList
            //  if the element is a primitive type
            //  i.e. contiguous<T>() == true
            virtual label byteSize() const = 0;

            //- Return the number of elements in the UList.
            virtual label size() const = 0;

            //- Return the number of components per element.
            virtual label nComponents() const = 0;

            //- Return a pointer to the data casted to char from the actual
            //  data type.
            virtual inline char* data() = 0;

            //- Return a const pointer to the first data element casted to char.
            virtual inline const char* cdata() const = 0;

            //- Determine whether the underlying list is UNIFORM, NONUNIFORM or
            //  EMPTY.
            virtual uniformity determineUniformity() const = 0;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
