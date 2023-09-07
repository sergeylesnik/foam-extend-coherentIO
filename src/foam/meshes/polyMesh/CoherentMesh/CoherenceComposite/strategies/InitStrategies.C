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

#include "InitStrategies.H"

#include "SliceStream.H"


void Foam::InitOffsets::execute
(
    Foam::InitStrategy::index_container& data,
    Foam::InitStrategy::labelPair& start_count
)
{
    auto value = !accumulate_ ?
                 start_count.first :
                 start_count.second;
    offsets_.set(value, accumulate_);
    data = Foam::labelList
           (
               {
                   offsets_.lowerBound(Pstream::myProcNo()),
                   offsets_.upperBound(Pstream::myProcNo())
               }
           );
}

// ************************************************************************* //
