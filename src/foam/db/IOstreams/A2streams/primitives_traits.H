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

#ifndef primitives_traits_H
#define primitives_traits_H

#include "scalar.H"
#include "direction.H"

#include <type_traits> // defines false_type, true_type, enable_if, void_t
#include <utility>     // defines declval

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// IS_VECTORSPACE
// primary template
template<typename, typename = std::void_t<>>
struct is_vectorspace : std::false_type {};

// partial specialization, maybe SFINAE'd away
template<typename T>
struct is_vectorspace
<
    T,
    std::void_t
    <
        decltype(std::declval<T>().size()),
        decltype
        (
            std::declval<T>().component(std::declval<direction>())
        ),
        decltype
        (
            std::declval<T>().replace
            (
                std::declval<direction>(),
                std::declval<T>()[std::declval<direction>()]
            )
        ),
        decltype(std::declval<T>()[std::declval<direction>()]),
        decltype(std::declval<T>().operator+=(std::declval<T>())),
        decltype(std::declval<T>().operator-=(std::declval<T>())),
        decltype(std::declval<T>().operator*=(std::declval<scalar>())),
        decltype(std::declval<T>().operator/=(std::declval<scalar>()))
    >
> : std::true_type {};

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
