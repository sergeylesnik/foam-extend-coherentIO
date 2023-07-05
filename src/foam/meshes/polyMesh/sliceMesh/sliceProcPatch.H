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
    Foam::sliceProcPatch

Description
    Required for processor patch construction on coherent mesh layout.

SourceFiles
    sliceProcPatch.C

\*---------------------------------------------------------------------------*/

#ifndef sliceProcPatch_H
#define sliceProcPatch_H

#include "labelList.H"
#include "faceList.H"

#include "Slice.H"

#include <vector>

#include <algorithm>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<typename SomeList, typename IndexList>
SomeList extractor(const SomeList& input, const IndexList& extractorList);

// Forward declaration
class Offsets;

/*---------------------------------------------------------------------------* \
                        Class sliceProcPatch Declaration
\*---------------------------------------------------------------------------*/

class sliceProcPatch
{
    using index_container = labelList;
    using iterator = index_container::iterator;

    // Counting the number of local processor patches
    static label instanceCount_;

    // ID of the processor patch
    label id_{};

    // Number of physical boundary patches
    label numNonProcPatches_{};

    // Cell slice
    Slice slice_{};

    // Indices in face ordered list that constitute the processor patch
    index_container localFaceIDs_{};

    // Indices in point ordered list that constitute the processor patch
    index_container localPointIDs_{};

    // Name of the processor patch
    word procBoundaryName_{};

public:

    // Constructors
    sliceProcPatch
    (
        const Foam::Slice&,
        const Foam::labelList&,
        const Foam::label&
    );

    // Copy constructor
    sliceProcPatch(const sliceProcPatch&);

    // Copy assignment operator
    sliceProcPatch& operator=(const sliceProcPatch&);

    // Destructor
    ~sliceProcPatch();

    // Copy-and-swap idiom
    void swap(sliceProcPatch&) noexcept;

    // Return the processor patch id
    label id();

    // Return the processor patch name
    word name();

    // Return the neighbouring processor
    label partner();

    // Extract faces IDs from the neighbour list in sliceable data layout
    void determineFaceIDs(const labelList&);

    // Extract point IDs from the face list in sliceable data layout
    void determinePointIDs(const faceList&, const label&);

    // Append owners from received processor patch to owner list
    void appendOwner(Foam::labelList&, Foam::labelList&);

    // Encode patch IDs in input neighbour list
    void encodePatch(labelList&);

    // Append patch IDs to neighbour list
    void encodePatch(std::vector<label>&, const label&);

    // Append patch IDs to neighbour list
    void encodePatch(labelList&, const label&);

    // Extract face IDs of processor patch from input list
    template<typename FaceList>
    FaceList extractFaces(const FaceList& input);

    // Extract point IDs of processor patch from input list
    pointField extractPoints(const pointField& input);

    // Return begin iterator of local face IDs
    iterator begin() { return localFaceIDs_.begin(); }

    // Return end iterator of local face IDs
    iterator end() { return localFaceIDs_.end(); }

    // Return size of face ID list
    label size() { return localFaceIDs_.size(); }
};

void swap(sliceProcPatch&, sliceProcPatch&) noexcept;

}

#include "sliceProcPatchI.H"

#endif
