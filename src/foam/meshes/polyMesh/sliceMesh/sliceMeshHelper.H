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

Free functions

Description
    The free functions in this file render algorithms to create the coherent
    data layout.

SourceFiles
    sliceMeshHelper.C
    sliceMeshHelperI.C

\*---------------------------------------------------------------------------*/

#ifndef sliceMeshHelper_H
#define sliceMeshHelper_H

#include "label.H"
#include "faceList.H"

#include <set>
#include <vector>
#include <algorithm>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<typename Type1, typename Type2>
using pairVector = std::vector<std::pair<Type1, Type2>>;

/*---------------------------------------------------------------------------*\
                       Helper Functions Declaration
\*---------------------------------------------------------------------------*/

// Partition and sort negative key indices (std::pair::first)
// (creates the permutation from sliceable to fragmented data layout)
void partitionByFirst(pairVector<label, label>&);

// Renumber point indices in faces by point mapping
void renumberFaces(faceList& faces, const std::vector<label>& map);

// Return the permutation that sorts the input list
// (creates the permutation from fragmented to sliceable data layout)
std::vector<label> permutationOfSorted(const labelList& input);

/*---------------------------------------------------------------------------*\
                     Helper Template Functions Declaration
\*---------------------------------------------------------------------------*/

// Create a list of pairs storing the explicit index with the input data
template<typename Container>
pairVector<label, label> generateIndexedPairs(const Container&);

// Create index list with given size and starting index
template<typename Container>
void
indexIota(Container&, const size_t&, const typename Container::value_type&);

// Permute input index list based on sorting of input data
template<typename IndexContainer, typename DataContainer>
void indexSort(IndexContainer&, const DataContainer&);

// Zip-combine two containers of equal length
template<typename Container1, typename Container2>
pairVector<typename Container1::value_type, typename Container2::value_type>
zip(const Container1&, const Container2&);

// Extract values from input that satisfy the unary condition
template<typename ValueType, typename UnaryOperation>
auto extractNth(const std::vector<ValueType>&, UnaryOperation);

// Determine extend of repeating value sequence in list
template<typename IterType, typename ValType>
void findValueExtend(IterType&, IterType&, const IterType&, const ValType&);

// Permute input data according to the input index list
template<typename Container, typename IndexContainer>
void applyPermutation(Container&, const IndexContainer&);

// Return the set of indices in the input container
// that fulfill the unary predicate
template<typename Container, typename UnaryPredicate>
std::set<label> pointSubset(const Container&, UnaryPredicate);

// Return the set of (point) indices in the input face list
// that fulfill the unary predicate
template<typename UnaryPredicate>
std::set<label> pointSubset(const faceList&, UnaryPredicate);

// Return the set of indices in the input container
template<typename Container>
std::set<label> pointSubset(const Container&);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "sliceMeshHelperI.H"

#endif

// ************************************************************************* //
