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

#include "slicePermutation.H"

#include <numeric>
#include <cmath>


// * * * * * * * * * * * * * * Free functions  * * * * * * * * * * * * * * //

Foam::label Foam::encodeSlicePatchId(const Foam::label& Id)
{
    return -(Id + 1);
}


Foam::label Foam::decodeSlicePatchId(const Foam::label& encodedId)
{
    return -(encodedId + 1);
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


void Foam::slicePermutation::createPointPermutation
(
    Foam::faceList& faces,
    const Foam::label& nPoints
)
{
    permutationToPolyPoint_.resize(nPoints, -1);
    permutationToSlicePoint_.resize(nPoints, -1);
    Foam::label slicePointId = 0;
    for (const auto& face: faces)
    {
        for (const auto& polyPointId: face)
        {
            if (permutationToPolyPoint_[polyPointId] == -1)
            {
                permutationToPolyPoint_[polyPointId] = slicePointId;
                permutationToSlicePoint_[slicePointId] = polyPointId;
                ++slicePointId;
            }
        }
    }
}


Foam::pairVector<Foam::label, Foam::label>
Foam::slicePermutation::createPolyNeighbourPermutation
(
    const Foam::labelList& sliceNeighbours
)
{
    auto polySlicePairs{generateIndexedPairs(sliceNeighbours)};
    partitionByFirst(polySlicePairs);
    return polySlicePairs;
}


void Foam::slicePermutation::renumberToSlice(Foam::faceList& input)
{
    renumberFaces(input, permutationToPolyPoint_);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::slicePermutation::slicePermutation(const Foam::labelList& sliceNeighbours)
:
    polyNeighboursPermutation_{createPolyNeighbourPermutation(sliceNeighbours)},
    polyNeighboursAndPatches_
    {
        extractNth
        (
            polyNeighboursPermutation_,
            [](const auto& pair)
            {
                return pair.first;
            }
        )
    },
    permutationToPoly_
    {
        extractNth
        (
            polyNeighboursPermutation_,
            [](const auto& pair)
            {
                return pair.second;
            }
        )
    }
{
    polyNeighboursPermutation_.clear();
}


Foam::slicePermutation::slicePermutation(const polyMesh& mesh)
:
    permutationToSlice_{permutationOfSorted(mesh.faceOwner())}
{
    Foam::faceList sliceFaces(mesh.allFaces());
    mapToSlice(sliceFaces);
    createPointPermutation(sliceFaces, mesh.nPoints());
}


Foam::slicePermutation::slicePermutation
(
    const Foam::labelList& faceOwner,
    const Foam::faceList& allFaces,
    const Foam::label& nPoints
)
:
    permutationToSlice_{permutationOfSorted(faceOwner)}
{
    Foam::faceList sliceFaces(allFaces);
    mapToSlice(sliceFaces);
    createPointPermutation(sliceFaces, nPoints);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Transformations to slicePermutation

void Foam::slicePermutation::mapToSlice(Foam::pointField& allPoints)
{
    applyPermutation(allPoints, permutationToSlicePoint_);
}


Foam::label Foam::slicePermutation::mapToSlice(const Foam::label& id)
{
    return permutationToSlice_[id];
}


Foam::faceList Foam::slicePermutation::generateSlice(Foam::faceList& allFaces)
{
    mapToSlice(allFaces);
    renumberToSlice(allFaces);
    return allFaces;
}


Foam::labelList
Foam::slicePermutation::generateSlice
(
    Foam::labelList& sliceNeighbours,
    const Foam::polyMesh& mesh
)
{
    sliceNeighbours.resize(permutationToSlice_.size());
    std::transform
    (
        permutationToSlice_.begin(),
        permutationToSlice_.end(),
        sliceNeighbours.begin(),
        [&mesh](const auto& fragFaceId)
        {
            auto patchId = mesh.boundaryMesh().whichPatch(fragFaceId);
            return (patchId != -1) ? encodeSlicePatchId(patchId)
                                   : mesh.faceNeighbour()[fragFaceId];
        }
    );
    return sliceNeighbours;
}


// Transformations to polyMesh

Foam::label Foam::slicePermutation::mapToPoly(const Foam::label& id)
{
    return permutationToPoly_[id];
}

// ************************************************************************* //
