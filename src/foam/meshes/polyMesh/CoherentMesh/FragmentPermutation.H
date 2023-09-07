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
    Foam::FragmentPermutation

Description
    Permuting from coherent and sliceable to fragmented data layout

SourceFiles
    FragmentPermutation.C

\*---------------------------------------------------------------------------*/

#ifndef FragmentPermutation_H
#define FragmentPermutation_H

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
                       Class FragmentPermutation Declaration
\*---------------------------------------------------------------------------*/

class FragmentPermutation
{
    pairVector<label, label> polyNeighboursPermutation_{};

    std::vector<label> polyNeighboursAndPatches_{};

    // Permutation from sliceable layout to
    // fragmented layout for face-ordered lists
    std::vector<label> facePermutation_{};

    //
    pairVector<label, label>
    createPolyNeighbourPermutation(const labelList&);

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


    auto findPatchBegin() -> decltype(polyNeighboursAndPatches_.begin())
    {
        return std::find_if
               (
                   polyNeighboursAndPatches_.begin(),
                   polyNeighboursAndPatches_.end(),
                   [](const Foam::label& id)
                   {
                       return id < 0;
                   }
               );
    }


public:

    // Constructors

        //- Default constructor
        FragmentPermutation() = default;

        //- Construct FragmentPermutation from neighbour
        explicit FragmentPermutation(const labelList&);

    // Member Functions
    //

        // Transformations to FragmentPermutation

        template<typename Container>
        void permute(Container&);

        label permute(const label&);

        template<typename Container>
        void retrieveNeighbours(Container&);

        template<typename Container>
        void retrievePatches(Container&);

};

#include "FragmentPermutationI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
