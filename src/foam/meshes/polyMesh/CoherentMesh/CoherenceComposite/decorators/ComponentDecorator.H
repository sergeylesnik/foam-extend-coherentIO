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
    Foam::ComponentDecorator

Description
    Foam::ComponentDecorator

SourceFiles
    ComponentDecorator.C

\*---------------------------------------------------------------------------*/

#ifndef ComponentDecorator_H
#define ComponentDecorator_H

#include "DataComponent.H"

namespace Foam
{

struct ComponentDecorator
:
    public DataComponent
{
    ComponentDecorator
    (
        DataComponentPtr&& component,
        base_raw_ptr const parent_component
    )
    :
        DataComponent{component->type(), component->name(), parent_component},
        component_{std::move(component)}
    {}

protected:

    DataComponentPtr component_{nullptr};

private:

    void _v_add_(base_ptr input) final
    {
        component_->add(input);
    }

    virtual void _v_pull_node_
    (
        const Foam::string& by_name,
        base_ptr& output
    )
    {
        output = component_->node(by_name);
    }

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

}// End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
