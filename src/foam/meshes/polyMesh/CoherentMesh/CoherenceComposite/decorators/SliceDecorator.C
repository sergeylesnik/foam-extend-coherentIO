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

\*---------------------------------------------------------------------------*/

#include "SliceDecorator.H"

#include "Offsets.H"
#include "Slice.H"

// * * * * * * * * * * * * * Private Member Functions * * * * * * * * * * * //

void Foam::SliceDecorator::_v_initialize_()
{
    component_->initialize();
    if (!initialized_)
    {
        Foam::Offsets offsets{};
        component_->extract(offsets);
        slice_ = Foam::Slice{Pstream::myProcNo(), offsets};
        initialized_ = true;
    }
}


void Foam::SliceDecorator::_v_extract_(Foam::Slice& output)
{
    output = slice_;
}


void Foam::SliceDecorator::_v_extract_(Foam::Offsets& output)
{
    component_->extract(output);
}


void
Foam::SliceDecorator::_v_extract_(Foam::DataComponent::index_container& output)
{
    component_->extract(output);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::SliceDecorator::SliceDecorator
(
    Foam::DataComponentPtr&& component,
    Foam::DataComponent* const parent_component
)
:
    ComponentDecorator{std::move(component), parent_component},
    slice_{},
    initialized_{false}
{}

// * * * * * * * * * * * * * Public Member Functions  * * * * * * * * * * * //

Foam::DataComponent::index_citerator
Foam::SliceDecorator::begin() const
{
    return component_->begin();
}


Foam::DataComponent::index_citerator
Foam::SliceDecorator::end() const
{
    return component_->end();
}


Foam::label
Foam::SliceDecorator::front() const
{
    return component_->front();
}


Foam::label
Foam::SliceDecorator::back() const
{
    return component_->back();
}


Foam::label
Foam::SliceDecorator::size() const
{
    return component_->size();
}

// ************************************************************************* //
