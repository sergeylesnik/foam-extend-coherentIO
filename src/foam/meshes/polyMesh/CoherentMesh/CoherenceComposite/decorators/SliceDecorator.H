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
    Foam::SliceDecorator

Description
    Foam::SliceDecorator

SourceFiles
    SliceDecorator.C

\*---------------------------------------------------------------------------*/

#ifndef SliceDecorator_H
#define SliceDecorator_H

#include "ComponentDecorator.H"

namespace Foam
{

// Only for IndexComponents decorated by OffsetDecorator
class SliceDecorator
:
    public ComponentDecorator
{
    Foam::Slice slice_{};

    bool initialized_{false};

    void _v_initialize_() final;

    void _v_extract_(Foam::Slice& output) final;

    void _v_extract_(Foam::Offsets& output) final;

    void _v_extract_(index_container& output) final;

public:

    SliceDecorator
    (
        DataComponentPtr&& component,
        base_raw_ptr const parent_component
    );

    index_citerator begin() const final;

    index_citerator end() const final;

    label front() const final;

    label back() const final;

    label size() const final;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
