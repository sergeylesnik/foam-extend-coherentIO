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
    Foam::DataComponent

Description
    Foam::DataComponent

SourceFiles
    DataComponent.C

\*---------------------------------------------------------------------------*/

#ifndef DataComponent_H
#define DataComponent_H

#include "labelList.H"
#include "scalarField.H"
#include "vectorField.H"

#include "Offsets.H"
#include "Slice.H"

#include "InitStrategies.H"

#include <memory>
#include <map>

namespace Foam
{

// Forward declarations
struct DataComponent;
class IndexComponent;

using DataComponentPtr = std::shared_ptr<DataComponent>;
// used as strategy in construction of components and decorators
using OffsetStrategy = std::function<Foam::label(const DataComponent&)>;

struct DataComponent
{
    // used as data container for IndexComponent
    using index_container = labelList;
    using index_iterator = index_container::iterator;
    using index_citerator = index_container::const_iterator;

    using base_raw_ptr = DataComponent*;
    using base_ptr = std::shared_ptr<DataComponent>;

    DataComponent() = default;

    DataComponent
    (
        const Foam::string type,
        const Foam::string name,
        base_raw_ptr const parent_component = nullptr
    );

    DataComponent(const DataComponent& other) = default;

    DataComponent(DataComponent&& other) = default;

    DataComponent& operator=(const DataComponent& other) = default;

    DataComponent& operator=(DataComponent&& other) = default;

    virtual ~DataComponent() = default;

    Foam::string parent_name() const;

    Foam::string name() const;

    Foam::string type() const;

    base_ptr add(const base_ptr& component);

    void initialize();

    base_ptr node(const Foam::string& by_name);

    void pull_node(const Foam::string& by_name, base_ptr& output);

    template<typename ComponentType = IndexComponent>
    base_ptr add
    (
        const Foam::string type,
        const Foam::string name,
        std::unique_ptr<InitStrategy> init_strategy,
        OffsetStrategy calc_start = nullptr,
        OffsetStrategy calc_count = nullptr
    );

    template<typename DecoratorType, typename... Args>
    base_ptr decorate(const Foam::string name, Args... args);

    template<class DataType>
    void extract(DataType& output);

    virtual label accept(const OffsetStrategy& offset_strategy) const
    {
        return !offset_strategy ? -1 : offset_strategy(*this);
    }

    virtual index_citerator begin() const { return index_container{}.begin(); }

    virtual index_citerator end() const { return index_container{}.end(); }

    virtual label front() const { return {}; }

    virtual label back() const { return {}; }

    virtual label size() const { return {}; }

private:

    Foam::string type_{};

    Foam::string name_{};

    virtual void _v_add_(base_ptr) = 0;

    virtual void _v_initialize_() = 0;

    virtual void
    _v_pull_node_(const Foam::string& by_name, base_ptr& output) = 0;

    virtual void _v_extract_(index_container& output) {}

    virtual void _v_extract_(std::vector<label>& output) {}

    virtual void _v_extract_(Foam::scalarField& output) {}

    virtual void _v_extract_(Foam::vectorField& output) {}

    virtual void _v_extract_(Foam::Offsets& output) {}

    virtual void _v_extract_(Foam::Slice& output) {}

protected:

    DataComponent* parent_component_{nullptr};

    base_raw_ptr parent_component_of(const base_ptr& component);

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

#include "DataComponentI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
