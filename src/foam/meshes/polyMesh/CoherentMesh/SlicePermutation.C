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

#include "SlicePermutation.H"

#include <numeric>
#include <cmath>


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


void Foam::SlicePermutation::createPointPermutation
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

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::SlicePermutation::SlicePermutation(const polyMesh& mesh)
:
    SlicePermutation(mesh.faceOwner(), mesh.allFaces(), mesh.nPoints())
{}


Foam::SlicePermutation::SlicePermutation
(
    const Foam::labelList& faceOwner,
    const Foam::faceList& allFaces,
    const Foam::label& nPoints
)
:
    permutationToSlice_(faceOwner.size()),
    faces_{allFaces.begin(), allFaces.end()}
{
    permutationOfSorted
    (
        permutationToSlice_.begin(),
        permutationToSlice_.end(),
        faceOwner
    );
    permute(faces_);
    createPointPermutation(faces_, nPoints);
    renumberFaces(faces_, permutationToPolyPoint_);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Transformations to SlicePermutation

void Foam::SlicePermutation::permute(Foam::pointField& allPoints)
{
    applyPermutation(allPoints, permutationToSlicePoint_);
}


void Foam::SlicePermutation::apply(Foam::faceList& allFaces)
{
    permute(allFaces);
    renumberFaces(allFaces, permutationToPolyPoint_);
}


Foam::faceList Foam::SlicePermutation::retrieveFaces()
{
    return faces_;
}


void Foam::SlicePermutation::retrieveNeighbours
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
        [&mesh](const Foam::label& fragFaceId)
        {
            auto patchId = mesh.boundaryMesh().whichPatch(fragFaceId);
            return (patchId != -1) ? encodeSlicePatchId(patchId)
                                   : mesh.faceNeighbour()[fragFaceId];
        }
    );
}

// ************************************************************************* //
