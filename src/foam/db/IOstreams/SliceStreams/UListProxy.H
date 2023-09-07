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
    Foam::UListProxy

Description
    A proxy of UList<T> containing type T. It inheriting from UList and
    provides additional functionality such as determination of uniformity.

SourceFiles
    UListProxy.C

\*---------------------------------------------------------------------------*/

#ifndef UListProxy_H
#define UListProxy_H

#include "uListProxyBase.H"
#include "UList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class UListProxy Declaration
\*---------------------------------------------------------------------------*/

template<class T>
class UListProxy
:
    public uListProxyBase,
    public UList<T>
{
public:

    // Constructors

        //- Construct copying UList attributes such as data pointer and size.
        UListProxy(UList<T> uList);


    //- Destructor
    virtual ~UListProxy()
    {}


    // Member Functions

        //- Write the first element of UList.
        virtual void writeFirstElement
        (
            Ostream& os,
            const char* data
        ) const override;


    // Byte-wise Access

        //- Return pointer to the underlying array serving as data storage,
        //  reinterpreted as byte data
        //  \note Only meaningful for contiguous data
        virtual inline const char* cdata_bytes() const override;

        //- Return pointer to the underlying array serving as data storage,
        //  reinterpreted as byte data
        //  \note Only meaningful for contiguous data
        virtual inline char* data_bytes() override;

        //- Number of contiguous bytes for the List data.
        //  \note Only meaningful for contiguous data
        virtual inline std::streamsize size_bytes() const override;


        // Access

            //- Return the binary size in number of characters of the UList
            //  if the element is a primitive type
            //  i.e. contiguous<T>() == true
            virtual label byteSize() const override;

            //- Return the number of elements in the UList.
            virtual label size() const override;

            //- Return the number of components per element.
            virtual label nComponents() const override;

            //- Return a pointer to the data casted to scalar
            virtual inline char* data() override;

            //- Return a const pointer to the data casted to scalar
            virtual inline const char* cdata() const override;

            //- Determine whether the underlying list is UNIFORM, NONUNIFORM or
            //  EMPTY.
            virtual uListProxyBase::uniformity
            determineUniformity() const override;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "UListProxy.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
