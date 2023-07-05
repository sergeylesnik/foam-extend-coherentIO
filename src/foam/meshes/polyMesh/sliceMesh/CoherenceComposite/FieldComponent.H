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
    Foam::FieldComponent

Description
    Foam::FieldComponent

SourceFiles
    FieldComponent.C
    FieldComponentI.C

\*---------------------------------------------------------------------------*/

#ifndef FieldComponent_H
#define FieldComponent_H

#include "DataComponent.H"

#include "InitStrategies.H"

namespace Foam
{

template<typename FieldType>
class FieldComponent
:
    public DataComponent
{
    FieldType data_;

    std::unique_ptr<InitStrategy> init_strategy_{nullptr};

    OffsetStrategy calc_start_{nullptr};

    OffsetStrategy calc_count_{nullptr};

    bool initialized_{false};

    // Core initialization for field component
    void init();

    // Adding and pulling components render redundant
    // because a field component acts as leaf component.
    // Hence, they are only dummy functions of the pure
    // virtual functions from base.
    void _v_add_(base_ptr input) final {}

    void _v_pull_node_(const Foam::string& by_name, base_ptr& output) final {}

    // Initialize this field component
    void _v_initialize_() final;

    // Method to retrieve field data_
    // TODO: could/should be replaced by iterator pattern and
    //       corresponding free functions with std::copy
    void _v_extract_(FieldType& output) final;

public:

    FieldComponent
    (
        const Foam::string type,
        const Foam::string name,
        std::unique_ptr<InitStrategy> init_strategy,
        OffsetStrategy calc_start = nullptr,
        OffsetStrategy calc_count = nullptr,
        base_raw_ptr const parent_component = nullptr
    );

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "FieldComponentI.H"

#endif

// ************************************************************************* //
