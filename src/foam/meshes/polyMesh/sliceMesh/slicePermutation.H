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
    Foam::slicePermutation

Description
    Permuting fragmented data structure to coherent and sliceable data layout

SourceFiles
    slicePermutation.C

\*---------------------------------------------------------------------------*/

#ifndef slicePermutation_H
#define slicePermutation_H

#include "sliceMeshHelper.H"
#include "polyMesh.H"

#include <vector>
#include <algorithm>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration
class pointMesh;

// Encoding a patch Id to differentiate from internal face IDs
label encodeSlicePatchId(const label&);

// Decoding an encoded patch Id to receive an indexing Id for face-ordered lists
label decodeSlicePatchId(const label&);

/*---------------------------------------------------------------------------*\
                       Class slicePermutation Declaration
\*---------------------------------------------------------------------------*/

class slicePermutation
{
    pairVector<label, label> polyNeighboursPermutation_{};

    std::vector<label> polyNeighboursAndPatches_{};

    // Permutation from sliceable layout to
    // fragmented layout for face-ordered lists
    std::vector<label> permutationToPoly_{};

    // Permutation from fragmented layout to
    // sliceable layout for face-ordered lists
    std::vector<label> permutationToSlice_{};

    // Permutation from fragmented layout
    // to sliceable layout for point-ordered lists
    std::vector<label> permutationToSlicePoint_{};

    // Permutation from sliceable layout
    // sliceable layout for point-ordered lists
    std::vector<label> permutationToPolyPoint_{};

    //
    pairVector<label, label>
    createPolyNeighbourPermutation(const labelList&);

    // Create the point permutation from fragmented to sliceable data layout
    void createPointPermutation(faceList&, const label&);

    void renumberToSlice(faceList&);

    template<typename IterType>
    void resetNextPatch
    (
        IterType& patchBegin,
        IterType& patchEnd,
        polyPatch& patch,
        const label& patchId
    )
    {
        findValueExtend
        (
            patchBegin,
            patchEnd,
            polyNeighboursAndPatches_.end(),
            encodeSlicePatchId(patchId)
        );
        patch.resetPatch
        (
            std::distance(patchBegin, patchEnd),
            std::distance(polyNeighboursAndPatches_.begin(), patchBegin)
        );
    }


    auto findPatchBegin()
    {
        return std::find_if
               (
                   polyNeighboursAndPatches_.begin(),
                   polyNeighboursAndPatches_.end(),
                   [](const auto &id)
                   {
                       return id < 0;
                   }
               ); //== encodeSlicePatchId( 0 ); } );
    }


public:

    // Constructors
    slicePermutation() = default;

    //- Construct from slicePermutation neighbour and patch list
    explicit slicePermutation(const labelList&);

    //- Construct from polyMesh
    explicit slicePermutation(const polyMesh&);

    //- Construct from faces, owners and number of points
    slicePermutation(const labelList&, const faceList&, const label&);

    // Member Functions
    //

    // Transformations to slicePermutation

    template<typename Container>
    void mapToSlice(Container&);

    void mapToSlice(pointField&);

    label mapToSlice(const label&);

    template<typename MeshElement>
    MeshElement generateSlice(MeshElement&);

    faceList generateSlice(faceList&);

    // Generate 'sliced' neighbours
    labelList generateSlice(labelList&, const polyMesh&);

    // Transformations to polyMesh

    template<typename Container>
    void mapToPoly(Container&);

    label mapToPoly(const label&);

    template<typename Container>
    void copyPolyNeighbours(Container&);

    template<typename Container>
    void copyPolyPatches(Container&);

};

#include "slicePermutationI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
