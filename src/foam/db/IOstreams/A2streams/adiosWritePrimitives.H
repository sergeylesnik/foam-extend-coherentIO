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

#ifndef adiosWritePrimitives_H
#define adiosWritePrimitives_H

#include "label.H"
#include "scalar.H"

#include "primitives_traits.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declarations
class string;
template<typename T> class List;

//- Write local array
void adiosWritePrimitives
(
    const string type,
    const string pathname,
    const string blockId,
    const label count,
    const scalar* buf
);

//- Write local array
void adiosWritePrimitives
(
    const string type,
    const string pathname,
    const string blockId,
    const label count,
    const label* buf
);

//- Write global n-dimensional array
void adiosWritePrimitives
(
    const string type,
    const string pathname,
    const string blockId,
    const List<label> shape,
    const List<label> start,
    const List<label> count,
    const scalar* buf
);

//- Write global array
void adiosWritePrimitives
(
    const string type,
    const string pathname,
    const string blockId,
    const label shape,
    const label start,
    const label count,
    const scalar* buf
);

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template
<
    typename T,
    std::enable_if_t<!Foam::is_vectorspace<T>::value, void>* = nullptr
>
void Foam::adiosWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string name,
    const Foam::label count,
    const T* buf
)
{
    std::cout << "adiosWritePrimitives not implemented for this type.\n";
    std::cout << typeid(buf).name() << "\n";
    std::cout << name << "\n";
}


//- Write local array
template
<
    typename T,
    std::enable_if_t<Foam::is_vectorspace<T>::value, void>* = nullptr
>
void Foam::adiosWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string name,
    const Foam::label count,
    const T* buf
)
{
    Foam::List<Foam::label> shapeList({count, buf[0].size()});
    Foam::List<Foam::label> startList({0, 0});
    Foam::List<Foam::label> countList{shapeList};
    adiosWritePrimitives
    (
        type,
        pathname,
        name,
        shapeList,
        startList,
        countList,
        reinterpret_cast<const Foam::scalar*>( buf )
    );
}


//- Write global array
template
<
    typename T,
    std::enable_if_t<Foam::is_vectorspace<T>::value, void>* = nullptr
>
void Foam::adiosWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string name,
    const Foam::label shape,
    const Foam::label start,
    const Foam::label count,
    const T* buf
)
{
    Foam::List<Foam::label> shapeList({shape, buf[0].size()});
    Foam::List<Foam::label> startList({start, 0});
    Foam::List<Foam::label> countList({count, buf[0].size()});
    adiosWritePrimitives
    (
        type,
        pathname,
        name,
        shapeList,
        startList,
        countList,
        reinterpret_cast<const Foam::scalar*>( buf )
    );
}


#endif

// ************************************************************************* //
