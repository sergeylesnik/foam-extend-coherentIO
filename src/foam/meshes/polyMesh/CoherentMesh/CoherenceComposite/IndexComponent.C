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

#include "IndexComponent.H"

#include "DataComponentFree.H" // head_of_composition

//#include "InitStrategies.H"

#include <utility>

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //


Foam::IndexComponent::IndexComponent
(
    const Foam::string type,
    const Foam::string name,
    std::unique_ptr<InitStrategy> init_strategy,
    Foam::OffsetStrategy calc_start,
    Foam::OffsetStrategy calc_count,
    Foam::DataComponent* const parent_component
)
:
    DataComponent(type, name, parent_component),
    components_map_{},
    data_{},
    init_strategy_{std::move(init_strategy)},
    calc_start_{calc_start},
    calc_count_{calc_count},
    initialized_{false}
{}


// * * * * * * * * * * * * * Public Member Functions  * * * * * * * * * * * //

Foam::label
Foam::IndexComponent::accept(const Foam::OffsetStrategy& offset_strategy) const
{
    return !offset_strategy ? -1 : offset_strategy(*this);
}


Foam::DataComponent::index_citerator
Foam::IndexComponent::begin() const
{
    return data_.begin();
}


Foam::DataComponent::index_citerator
Foam::IndexComponent::end() const
{
    return data_.end();
}


Foam::label
Foam::IndexComponent::front() const
{
    return *data_.begin();
}


Foam::label
Foam::IndexComponent::back() const
{
    return *data_.rbegin();
}


Foam::label
Foam::IndexComponent::size() const
{
    return data_.size();
}

// * * * * * * * * * * * * * Private Member Functions * * * * * * * * * * * //

void Foam::IndexComponent::init()
{
    auto parent = !parent_component_ ? this : parent_component_;
    Foam::InitStrategy::labelPair start_count
    (
        parent->accept(calc_start_),
        parent->accept(calc_count_)
    );
    init_strategy_->operator()(data_, start_count);
    initialized_ = true;
}


void Foam::IndexComponent::init_children()
{
    using std::begin;
    using std::end;
    std::for_each
    (
        begin(components_map_),
        end(components_map_),
        []
        (
            const std::pair
            <
                std::string,
                Foam::DataComponent::base_ptr
            >& named_component
        )
        {
            named_component.second->initialize();
        }
    );
}


void Foam::IndexComponent::_v_add_(base_ptr input)
{
    auto name_of_input = input->name();
    if (components_map_.count(name_of_input) == 1)
    {
        components_map_[name_of_input] = input;
    }
    else
    {
        components_map_.emplace(std::make_pair(name_of_input, input));
    }
}


void Foam::IndexComponent::_v_initialize_()
{
    if (!initialized_ && !head_of_composition(*this))
    {
        init();
    }
    init_children();
}


void Foam::IndexComponent::_v_pull_node_
(
    const Foam::string& by_name,
    base_ptr& output
)
{
    if (output) { return; }
    auto pulled_component = components_map_.find(by_name);
    if (pulled_component != components_map_.end())
    {
        output = pulled_component->second;
        return;
    }
    for (const auto& named_component: components_map_)
    {
        named_component.second->pull_node(by_name, output);
    }
}


void
Foam::IndexComponent::_v_extract_(Foam::DataComponent::index_container& output)
{
    output = std::move(data_);
    data_.clear();
    initialized_ = false;
}


void Foam::IndexComponent::_v_extract_(std::vector<label>& output)
{
    output.resize(data_.size());
    std::move(data_.begin(), data_.end(), output.begin());
    data_.clear();
    initialized_ = false;
}


void Foam::IndexComponent::_v_extract_(Foam::Offsets& output)
{
    if (type() != "offsets") { return; }
    if (!initialized_) { return; }
    output = init_strategy_->offsets();
}

// ************************************************************************* //
