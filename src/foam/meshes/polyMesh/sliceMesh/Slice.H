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
    Foam::Slice

Description
    Defining a mesh slice of a cell/face/point list.

SourceFiles
    Slice.C
    SliceI.C

\*---------------------------------------------------------------------------*/

#ifndef Slice_H
#define Slice_H

#include "label.H"

#include "faceList.H" //required for appendFaces

#include <functional>
#include <algorithm>
#include <utility> // std::declval

#include "sliceMap.H"
#include "sliceMeshHelper.H"

namespace Foam
{

// Type trait for 'ranges'
template<typename T, typename _ = void>
struct is_range : std::false_type {};

template<typename... Ts>
struct is_range_helper {};

template<typename T>
struct is_range
<
    T,
    std::conditional_t
    <
        false,
        is_range_helper
        <
            decltype(std::declval<T>().begin()),
            decltype(std::declval<T>().end())
        >,
        void
    >
>
:
    public std::true_type
{};


// Forward declaration
class Offsets;
class Slice;

std::set<label>
missingPoints(const faceList& faces, const Slice& slice);

template<typename Container>
label numCellsFromSlice(const Container&, const Slice&);

template<typename Container, typename Operator>
void appendTransformed(Container& ret, Container& input, Operator operation);

/*---------------------------------------------------------------------------* \
                            Class Slice Declaration
\*---------------------------------------------------------------------------*/

class Slice
{
    // Partition Id
    label partition_{};

    // Smallest index on slice; renders an offset to partition below
    label bottom_{};

    // Largest index on slice; renders an offset to partition above
    label top_{};

    // Explicit mapping of IDs from processor patches
    std::shared_ptr<sliceMap> mapping_{};

    // Shift id based on bottom_ offset
    label shift(const label& id) const;

public:

    // Constructors
    Slice() = default;

    // Construct based on partition id and offsets
    Slice(const label&, const Foam::Offsets&);

    // Return partition id
    label partition();

    // Check if input id natively belongs to slice
    bool operator()(const label& id) const;

    // Check if input id belongs to slice including mapping_
    bool exist(const label& id) const;

    // Convert from global to local id
    label convert(const label& id) const;

    // Convert container content from global to local IDs
    template<typename Container>
    typename std::enable_if<is_range<Container>::value, Container>::type
    convert(Container& list) const;

    // Append content of Container to mapping_
    template<typename Container>
    void append(const Container&);

};

}

#include "SliceI.H"

#endif
