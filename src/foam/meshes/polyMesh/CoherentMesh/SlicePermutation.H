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
    Foam::SlicePermutation

Description
    Permuting fragmented data structure to coherent and sliceable data layout

SourceFiles
    SlicePermutation.C

\*---------------------------------------------------------------------------*/

#ifndef SlicePermutation_H
#define SlicePermutation_H

#include "sliceMeshHelper.H"
#include "polyMesh.H"

#include <vector>
#include <algorithm>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration
class pointMesh;

/*---------------------------------------------------------------------------*\
                       Class SlicePermutation Declaration
\*---------------------------------------------------------------------------*/

class SlicePermutation
{
    // Permutation from fragmented layout to
    // sliceable layout for face-ordered lists
    std::vector<label> permutationToSlice_{};

    // Permutation from fragmented layout
    // to sliceable layout for point-ordered lists
    std::vector<label> permutationToSlicePoint_{};

    // Permutation from sliceable layout
    // sliceable layout for point-ordered lists
    std::vector<label> permutationToPolyPoint_{};

    // Readily permuted faces
    faceList faces_{};

    // Create the point permutation from fragmented to sliceable data layout
    void createPointPermutation(faceList&, const label&);

public:

    // Constructors

        //- Default construct
        SlicePermutation() = default;

        //- Construct from polyMesh
        explicit SlicePermutation(const polyMesh&);

        //- Construct from faces, owners and number of points
        SlicePermutation(const labelList&, const faceList&, const label&);

    // Member Functions
    //

        // Transformations to SlicePermutation

        template<typename Container>
        void permute(Container&);

        void permute(pointField&);

        template<typename MeshElement>
        void apply(MeshElement&);

        void apply(faceList&);

        Foam::faceList retrieveFaces();

        // Generate 'sliced' neighbours
        void retrieveNeighbours(labelList&, const polyMesh&);

};

#include "SlicePermutationI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
