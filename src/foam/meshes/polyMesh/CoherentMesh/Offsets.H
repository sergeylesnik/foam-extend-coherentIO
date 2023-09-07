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
    Foam::Offsets

Description
    Foam::Offsets

SourceFiles
    Offsets.C

\*---------------------------------------------------------------------------*/

#ifndef Offsets_H
#define Offsets_H

#include "Pstream.H"

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class Offsets Declaration
\*---------------------------------------------------------------------------*/

class Offsets
{
    using pair_container = List<std::pair<label,label>>;

    //labelList offsets_{labelList(Pstream::nProcs(), 0)};

    pair_container offset_pairs_
                   {
                       pair_container
                       (
                           Pstream::nProcs(),
                           std::pair<label, label>(0,0)
                       )
                   };

public:

    using iterator = pair_container::iterator;

    // Default constructor
    Offsets() = default;

    // Construct from value
    explicit Offsets
    (
        label value,
        bool reduce = false
    );

    // Setting gather-scatter of values across processors
    void set(label value, bool reduce = false);

    // Return offset to processor below
    label lowerBound(label) const;

    // Return offset to processor above
    label upperBound(label) const;

    // Return the difference between lower and upper bound
    label count(label) const;

    label size() const;

    iterator begin();

    iterator end();

    std::pair<label, label> operator[](label) const;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
