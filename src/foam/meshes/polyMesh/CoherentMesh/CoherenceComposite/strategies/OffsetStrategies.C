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

#include "OffsetStrategies.H"

#include "DataComponent.H"

#include <iterator>

Foam::label Foam::count_geq::operator()(const Foam::DataComponent& input)
{
    using std::begin;
    using std::end;
    using value_type = decltype(*begin(input));
    return std::count_if
    (
        begin(input),
        end(input),
        [this](const value_type& val)
        {
            return val>=value_;
        }
    );
}


Foam::label Foam::count_eq::operator()(const Foam::DataComponent& input)
{
    using std::begin;
    using std::end;
    using value_type = decltype(*begin(input));
    return std::count_if
           (
               begin(input),
               end(input),
               [this](const value_type& val)
               {
                   return val == value_;
               }
           );
}


Foam::label
Foam::offset_by_max_plus_one(const Foam::DataComponent& input)
{
    return *std::max_element(input.begin(), input.end()) + 1;
}


Foam::label
Foam::offset_by_size_minus_one(const Foam::DataComponent& input)
{
    return input.size() - 1;
}


Foam::label
Foam::start_from_myProcNo(const Foam::DataComponent& dummy)
{
    return Pstream::myProcNo();
}


Foam::label
Foam::count_two(const Foam::DataComponent& dummy)
{
    return 2;
}


Foam::label
Foam::start_from_front(const Foam::DataComponent& input)
{
    return input.front();
}


Foam::label
Foam::count_from_front(const Foam::DataComponent& input)
{
    return input.back() - input.front();
}


Foam::label
Foam::count_from_size(const Foam::DataComponent& input)
{
    return input.size();
}


Foam::label
Foam::count_from_front_plus_one(const Foam::DataComponent& input)
{
    return count_from_front(input) + 1;
}


Foam::label
Foam::start_from_max(const Foam::DataComponent& input)
{
    Foam::Offsets offsets(*std::max_element(input.begin(), input.end()) + 1);
    return offsets.lowerBound(Pstream::myProcNo());
}


Foam::label
Foam::count_from_max(const Foam::DataComponent& input)
{
    Foam::Offsets offsets(*std::max_element(input.begin(), input.end()) + 1);
    return offsets.count(Pstream::myProcNo());
}


// ************************************************************************* //
