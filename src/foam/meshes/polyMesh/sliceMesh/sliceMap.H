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
    Foam::sliceMap

Description
    Defining a mesh slice of a cell/face/point list.

SourceFiles
    sliceMap.C
    sliceMapI.C

\*---------------------------------------------------------------------------*/

#ifndef sliceMap_H
#define sliceMap_H

#include "label.H"

#include <map>

namespace Foam
{

class sliceMap
{
    std::map<label, label> mapping_{};

    const label numNativeEntities_{};

public:

    // Constructor
    sliceMap() = default;

    // Construct with an offset in mapped IDs
    sliceMap(const label&);

    // Append a list of IDs to the map
    template<typename Container>
    void append(const Container&);

    // Return mapped Id
    label operator[](const label&);

    // Check if input Id is mapped
    bool exist(const label&);

};

}

#include "sliceMapI.H"

#endif
