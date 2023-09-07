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

#ifndef sliceReadPrimitives_H
#define sliceReadPrimitives_H

#include "List.H"
#include "label.H"
#include "scalar.H"

#include "primitives_traits.H"

#include <initializer_list>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class string;


void sliceReadPrimitives
(
    const string type,
    const string pathname,
    const string name,
    scalar* buf,
    const List<label>& start = {},
    const List<label>& count = {}
);

void sliceReadPrimitives
(
    const string type,
    const string pathname,
    const string name,
    label* buf,
    const List<label>& start = {},
    const List<label>& count = {}
);

void sliceReadPrimitives
(
    const string type,
    const string pathname,
    const string name,
    char* buf,
    const List<label>& start = {},
    const List<label>& count = {}
);
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<typename T>
typename std::enable_if<!Foam::is_vectorspace<T>::value, void>::type
Foam::sliceReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string name,
    T* buf,
    const Foam::List<Foam::label>& start = {},
    const Foam::List<Foam::label>& count = {}
)
{
    std::cout << "sliceReadPrimitives not implemented for this type.\n";
    std::cout << typeid(buf).name() << "\n";
    std::cout << name << "\n";
}


template<typename T>
typename std::enable_if<Foam::is_vectorspace<T>::value, void>::type
Foam::sliceReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string name,
    T* buf,
    const Foam::List<Foam::label>& start = {},
    const Foam::List<Foam::label>& count = {}
)
{
    auto startList = start;
    auto countList = count;
    if (start.size()>0 && count.size()>0)
    {
        startList = Foam::List<Foam::label>({start[0], 0});
        countList = Foam::List<Foam::label>({count[0], buf[0].size()});
    }
    Foam::sliceReadPrimitives
    (
        type,
        pathname,
        name,
        reinterpret_cast<scalar*>( buf ),
        startList,
        countList
    );
}


// if T is arithmetic
template
<
    typename T,
    typename Integral
>
typename std::enable_if
<
    std::is_integral<Integral>::value && std::is_arithmetic<T>::value,
    void
>::type
Foam::sliceReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string name,
    T* buf,
    Integral start = -1,
    Integral count = -1
)
{
    Foam::List<Integral> startList{};
    Foam::List<Integral> countList{};
    if (start>-1 && count>-1)
    {
        startList = Foam::List<Foam::label>({start});
        countList = Foam::List<Foam::label>({count});
    }
    Foam::sliceReadPrimitives(type, pathname, name, buf, startList, countList);
}


// if T is vector space
template
<
    typename T,
    typename Integral
>
typename std::enable_if
<
    Foam::is_vectorspace<T>::value && std::is_integral<Integral>::value,
    void
>::type
Foam::sliceReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string name,
    T* buf,
    Integral start = -1,
    Integral count = -1
)
{
    Foam::List<Integral> startList({start});
    Foam::List<Integral> countList({count});
    Foam::sliceReadPrimitives(type, pathname, name, buf, startList, countList);
}


#endif

// ************************************************************************* //
