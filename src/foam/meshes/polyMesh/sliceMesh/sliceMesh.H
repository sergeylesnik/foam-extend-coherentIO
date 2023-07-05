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
    Foam::sliceMesh

Description
    Foam::sliceMesh

SourceFiles
    sliceMesh.C

\*---------------------------------------------------------------------------*/

#ifndef sliceMesh_H
#define sliceMesh_H

#include "Offsets.H"
#include "Slice.H"
#include "sliceProcPatch.H"
#include "slicePermutation.H"
#include "polyMesh.H"
#include "MeshObject.H"
#include "globalIndex.H"

#include <vector>
#include <algorithm>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class sliceMesh Declaration
\*---------------------------------------------------------------------------*/

void sendSliceFaces
(
    std::pair<label, label> sendPair,
    Offsets& cellOffsets,
    Offsets& pointOffsets,
    labelList& globalNeighbours,
    faceList& globalFaces,
    pointField& allPoints_,
    std::vector<sliceProcPatch>& sliceProcPatches,
    label numBoundaries
);

void recvSliceFaces
(
    std::pair<label, label> recvPair,
    faceList& globalFaces,
    labelList& localOwner,
    Offsets& cellOffsets,
    Offsets& pointOffsets,
    Slice& cellSlice,
    Slice& pointSlice,
    pointField& allPoints_,
    std::vector<sliceProcPatch>& sliceProcPatches,
    labelList& globalNeighbours,
    label numBoundaries
);


class sliceMesh
:
    public MeshObject<polyMesh, sliceMesh>
{
    label numBoundaries_{};

    labelList globalNeighbours_{};

    labelList localOwner_{};

    faceList globalFaces_{};

    pointField allPoints_{};

    Offsets cellOffsets_{};

    Offsets faceOffsets_{};

    Offsets internalSurfaceFieldOffsets_{};

    std::vector<Offsets> boundarySurfacePatchOffsets_{};

    Offsets pointOffsets_{};

    Slice cellSlice_{};

    Slice pointSlice_{};

    std::vector<sliceProcPatch> slicePatches_{};

    slicePermutation sliceablePermutation_{};

    PtrList<globalIndex> boundaryGlobalIndex_{};

    // Internal face indices
    // in processor boundaries
    Foam::labelList internalFaceIDs_;

    // Processor boundary indices of internal faces
    // participating in a processor boundary
    Foam::labelList procBoundaryIDs_;

    // Private Member Functions
    void readMesh(const fileName&);

    // De-serialize faces
    faceList
    deserializeFaces(const std::vector<label>&, const std::vector<label>&);

    // Serialize owner list
    labelList serializeOwner(const std::vector<label>&);

    // TODO: lengthy functions that must be refactored
    //       (structure and design still pending)
    void initializeSurfaceFieldMappings();

    void commSlicePatches();

    void commSharedPoints();

    void renumberFaces();

public:

    TypeName("sliceMesh");

    // Constructors
    sliceMesh() = default;

    sliceMesh(const polyMesh&);

    // Destructor
    ~sliceMesh() = default;

    // Member Functions

    // Access

    inline const Offsets& cellOffsets() const
    {
        return cellOffsets_;
    }

    inline const Offsets& internalSurfaceFieldOffsets() const
    {
        return internalSurfaceFieldOffsets_;
    }

    inline const labelList& internalFaceIDsFromBoundaries() const
    {
        return internalFaceIDs_;
    }

    inline const labelList& boundryIDsFromInternalFaces() const
    {
        return procBoundaryIDs_;
    }

    labelList polyNeighbours();

    labelList polyOwner();

    faceList polyFaces();

    pointField polyPoints();

    std::vector<label> polyPatches();

    std::vector<sliceProcPatch> procPatches();

    List<polyPatch*> polyPatches(polyBoundaryMesh&);

    // Compulsory overloads resulting from the inheritance from MeshObject

    virtual bool movePoints() const
    {
        // ToDoIO
        FatalErrorInFunction
        << "not implemented"
        << abort(FatalError);

        return true;
    }


    virtual bool updateMesh(const mapPolyMesh&) const
    {
        // ToDoIO
        FatalErrorInFunction
        << "not implemented"
        << abort(FatalError);

        return true;
    }


    // Field infrastructure

    const globalIndex& boundaryGlobalIndex(label patchId);

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
