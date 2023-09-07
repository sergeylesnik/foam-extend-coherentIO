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
    Foam::IndexComponent

Description
    Foam::IndexComponent

SourceFiles
    IndexComponent.C

\*---------------------------------------------------------------------------*/

#ifndef IndexComponent_H
#define IndexComponent_H

#include "DataComponent.H"
#include "InitStrategies.H"

namespace Foam
{

class IndexComponent
:
    public DataComponent
{
    std::map<std::string, base_ptr> components_map_{};

    index_container data_{};

    std::unique_ptr<InitStrategy> init_strategy_{nullptr};

    OffsetStrategy calc_start_{nullptr};

    OffsetStrategy calc_count_{nullptr};

    bool initialized_{false};

    // Core initialization for this index component
    // with given initialization strategy:
    // 1) data from ADIOS or
    // 2) or offsets based on parent
    void init();

    // Core initialization for children
    void init_children();

    // Adds a component into component_map_
    void _v_add_(base_ptr input) final;

    // Initialize this index component and
    // its children in components_map_
    void _v_initialize_() final;

    // Search for node by name and copy it to the output pointer
    void _v_pull_node_(const Foam::string& by_name, base_ptr& output) final;

    // Different methods to retrieve data_
    // TODO: could/should be replaced by iterator pattern and
    //       corresponding free functions with std::copy
    void _v_extract_(index_container& output) final;

    void _v_extract_(std::vector<label>& output) final;

    void _v_extract_(Foam::Offsets& output) final;

public:

    IndexComponent() = default;

    IndexComponent
    (
        const Foam::string type,
        const Foam::string name,
        std::unique_ptr<InitStrategy> init_strategy,
        OffsetStrategy calc_start = nullptr,
        OffsetStrategy calc_count = nullptr,
        base_raw_ptr const parent_component = nullptr
    );

    // Key feature to determine coherents between a
    // component and its parent:
    //
    // Takes a function to calculate an offset integer.
    // This is used by an index component to determine
    // its offsets by the data of its parent component.
    label accept(const OffsetStrategy& offset_strategy) const final;

    // Returning begin iterator to internal data_
    index_citerator begin() const final;

    // Returning end iterator to internal data_
    index_citerator end() const final;

    // Returns the first value
    // contained in data_
    label front() const final;

    // Returns the last value
    // contained in data_
    label back() const final;

    // Returns the size of data_
    label size() const final;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
