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

#include "DataComponent.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::DataComponent::DataComponent
(
    const Foam::string type,
    const Foam::string name,
    Foam::DataComponent* const parent_component
)
:
    type_{type},
    name_{name},
    parent_component_{parent_component}
{}

// * * * * * * * * * * * * * Public Member Functions  * * * * * * * * * * * //

Foam::string
Foam::DataComponent::parent_name() const
{
    return parent_component_ ? parent_component_->name() : Foam::string{};
}


Foam::string
Foam::DataComponent::name() const
{
    return name_;
}


Foam::string
Foam::DataComponent::type() const
{
    return type_;
}


Foam::DataComponentPtr
Foam::DataComponent::add(const Foam::DataComponentPtr& component)
{
    _v_add_(component);
    return component;
}


void Foam::DataComponent::initialize()
{
    _v_initialize_();
}


Foam::DataComponentPtr
Foam::DataComponent::node(const Foam::string& by_name)
{
    Foam::DataComponentPtr ret{nullptr};
    if (by_name.compare("") == 0)
    {
        return ret;
    }
    pull_node(by_name, ret);
    return ret;
}


void Foam::DataComponent::pull_node
(
    const Foam::string& by_name,
    Foam::DataComponentPtr& output
)
{
    _v_pull_node_(by_name, output);
}

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

// if parent is the head node,
// parent_component will be set to this
Foam::DataComponent*
Foam::DataComponent::parent_component_of
(
    const Foam::DataComponentPtr& component
)
{
    Foam::DataComponent* parent_component = node
                                            (
                                                component->parent_name()
                                            ).get();
    return !parent_component ? this : parent_component;
}

// ************************************************************************* //
