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

#include "FragmentPermutation.H"

#include <numeric>
#include <cmath>


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

Foam::pairVector<Foam::label, Foam::label>
Foam::FragmentPermutation::createPolyNeighbourPermutation
(
    const Foam::labelList& sliceNeighbours
)
{
    auto polySlicePairs{generateIndexedPairs(sliceNeighbours)};
    partitionByFirst(polySlicePairs);
    return polySlicePairs;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::FragmentPermutation::FragmentPermutation(const Foam::labelList& sliceNeighbours)
:
    polyNeighboursPermutation_{createPolyNeighbourPermutation(sliceNeighbours)},
    polyNeighboursAndPatches_
    {
        extractNth
        (
            polyNeighboursPermutation_,
            [](const std::pair<Foam::label, Foam::label>& inputPair)
            {
                return inputPair.first;
            }
        )
    },
    facePermutation_
    {
        extractNth
        (
            polyNeighboursPermutation_,
            [](const std::pair<Foam::label, Foam::label>& inputPair)
            {
                return inputPair.second;
            }
        )
    }
{
    polyNeighboursPermutation_.clear();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Transformations to FragmentPermutation

Foam::label Foam::FragmentPermutation::permute(const Foam::label& id)
{
    return facePermutation_[id];
}

// ************************************************************************* //
