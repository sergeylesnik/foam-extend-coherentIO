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
    Foam::OffsetDecorator

Description
    Foam::OffsetDecorator

SourceFiles
    OffsetDecorator.C

\*---------------------------------------------------------------------------*/

#ifndef OffsetDecorator_H
#define OffsetDecorator_H

#include "ComponentDecorator.H"

namespace Foam
{

// Only for IndexComponents
class OffsetDecorator
:
    public ComponentDecorator
{
    Foam::Offsets offsets_{};

    OffsetStrategy offset_strategy_{nullptr};

    bool accumulate_{false};

    bool initialized_{false};

    void _v_extract_(Foam::Offsets& output) final
    {
        output = offsets_;
    }

    void _v_extract_(index_container& output) final
    {
        component_->extract(output);
    }

    void _v_initialize_() final
    {
        component_->initialize();
        if (!initialized_)
        {
            offsets_.set(offset_strategy_(*component_.get()), accumulate_);
            initialized_ = true;
        }
    }

public:

    OffsetDecorator
    (
        DataComponentPtr&& component,
        base_raw_ptr const parent_component,
        OffsetStrategy offset_strategy,
        bool accumulate = false
    )
    :
        ComponentDecorator{std::move(component), parent_component},
        offsets_{},
        offset_strategy_{offset_strategy},
        accumulate_{accumulate},
        initialized_{false}
    {}


    label accept(const OffsetStrategy& determine_offset) const final
    {
        if (!determine_offset)
        {
            return {};
        }
        return determine_offset(*this);
    }

    label front() const final
    {
        return offsets_.front();
    }

    label back() const final
    {
        return offsets_.back();
    }

    label size() const final
    {
        return offsets_.size();
    }

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
