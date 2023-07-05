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

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<typename ComponentType = Foam::IndexComponent>
Foam::DataComponentPtr Foam::DataComponent::add
(
    const Foam::string type,
    const Foam::string name,
    std::unique_ptr<InitStrategy> init_strategy,
    Foam::OffsetStrategy calc_start,
    Foam::OffsetStrategy calc_count
)
{
    auto component = std::make_shared<ComponentType>
                     (
                         type,
                         name,
                         std::move(init_strategy),
                         calc_start,
                         calc_count,
                         this
                     );
    _v_add_(component);
    return component;
}


template<typename DecoratorType, typename... Args>
Foam::DataComponentPtr
Foam::DataComponent::decorate(const Foam::string name, Args... args)
{
    DataComponentPtr component = node(name);
    auto parent_component = parent_component_of(component);
    component = std::make_shared<DecoratorType>
                (
                    std::move(component),
                    parent_component,
                    args...
                );
    parent_component->add(component);
    return component;
}


template<class DataType>
void Foam::DataComponent::extract(DataType& output)
{
    _v_extract_(output);
};

// ************************************************************************* //
