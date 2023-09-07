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
    Foam::CoherentMesh

Description
    Foam::CoherentMesh

SourceFiles
    CoherentMesh.C

\*---------------------------------------------------------------------------*/

#ifndef CoherentMesh_H
#define CoherentMesh_H

#include "Offsets.H"
#include "Slice.H"
#include "ProcessorPatch.H"
#include "FragmentPermutation.H"
#include "polyMesh.H"
#include "MeshObject.H"
#include "globalIndex.H"

#include "IndexComponent.H"

#include <vector>
#include <algorithm>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class CoherentMesh Declaration
\*---------------------------------------------------------------------------*/

class CoherentMesh
:
    public MeshObject<polyMesh, CoherentMesh>
{

    IndexComponent tree_{};

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

    std::vector<ProcessorPatch> slicePatches_{};

    FragmentPermutation splintedPermutation_{};

    PtrList<globalIndex> boundaryGlobalIndex_{};

    // Internal face indices
    // in processor boundaries
    Foam::labelList internalFaceIDs_;

    // Processor boundary indices of internal faces
    // participating in a processor boundary
    Foam::labelList procBoundaryIDs_;

    // Private Member Functions
    void readMesh(const fileName&);

    void sendSliceFaces(std::pair<label, label> sendPair);

    void recvSliceFaces(std::pair<label, label> recvPair);

    // De-serialize faces
    void deserializeFaces(const std::vector<label>&, const std::vector<label>&);

    // Serialize owner list
    void serializeOwner(const std::vector<label>&);

    // TODO: lengthy functions that must be refactored
    //       (structure and design still pending)
    void initializeSurfaceFieldMappings();

    void commSlicePatches();

    void commSharedPoints();

    void renumberFaces();

public:

    TypeName("CoherentMesh");

    // Constructors
    CoherentMesh() = default;

    CoherentMesh(const polyMesh&);

    // Destructor
    ~CoherentMesh() = default;

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

    void polyNeighbours(labelList&);

    void polyOwner(labelList&);

    void polyFaces(faceList&);

    void polyPoints(pointField&);

    std::vector<label> polyPatches();

    std::vector<ProcessorPatch> procPatches();

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
