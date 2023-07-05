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
    Foam::OffsetStrategies

Description
    Foam::OffsetStrategies

SourceFiles
    OffsetStrategies.C

\*---------------------------------------------------------------------------*/

#ifndef OffsetStrategies_H
#define OffsetStrategies_H

#include "DataComponent.H"

#include <algorithm>

namespace Foam
{

struct count_geq
{
    explicit count_geq(const label value) : value_{value} {}

    label operator()(const DataComponent&);

private:
    label value_{};
};


struct count_eq
{
    explicit count_eq(const label value) : value_{value} {}

    label operator()(const DataComponent&);

private:
    label value_{};
};


label offset_by_max_plus_one(const DataComponent&);

label offset_by_size_minus_one(const DataComponent&);

label start_from_myProcNo(const DataComponent&);

label count_two(const DataComponent&);

label start_from_front(const DataComponent&);

label count_from_front(const DataComponent&);

label count_from_size(const DataComponent&);

label count_from_front_plus_one(const DataComponent&);

label start_from_max(const DataComponent&);

label count_from_max(const DataComponent&);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
