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
    Foam::InitStrategies

Description
    Foam::InitStrategies

SourceFiles
    InitStrategies.C

\*---------------------------------------------------------------------------*/

#ifndef InitStrategies_H
#define InitStrategies_H

#include "Offsets.H"
#include "adiosStream.H"
#include "adiosReadPrimitives.H"

#include "labelList.H"
#include "scalarField.H"
#include "vectorField.H"

#include <utility>

namespace Foam
{

struct InitStrategy
{
    virtual ~InitStrategy() = default;

    using index_container = labelList;
    using labelPair = std::pair<Foam::label, Foam::label>;

    template<typename Container>
    void operator()(Container& data, labelPair& start_count)
    {
        execute(data, start_count);
    }

    virtual Foam::Offsets offsets() { return {}; }

private:

    virtual void execute(index_container& data, labelPair& start_count) {}

    virtual void execute(Foam::scalarField& data, labelPair& start_count) {}

    virtual void execute(Foam::vectorField& data, labelPair& start_count) {}

};


template<typename FieldType = InitStrategy::index_container>
struct InitFromADIOS
:
    public InitStrategy
{
    InitFromADIOS() = default;

    explicit InitFromADIOS
    (
        const Foam::string& type,
        const Foam::string& pathname,
        const Foam::string& name
    )
    :
        type_{type},
        pathname_{pathname},
        name_{name}
    {}

    label size() const
    {
        auto adiosStreamPtr = adiosReading{}.createStream();
        adiosStreamPtr->open(type_, pathname_);
        FieldType dummy{};
        return adiosStreamPtr->getBufferSize(name_, dummy.data());
    }

    explicit operator bool() const
    {
        return this->size();
    }

private:

    void execute
    (
        FieldType& data,
        Foam::InitStrategy::labelPair& start_count
    ) final
    {
        auto start = (start_count.first != -1) ?
                     labelList({start_count.first}) :
                     labelList({});
        auto count = (start_count.second != -1) ?
                     labelList({start_count.second}) :
                     labelList({});
        Foam::adiosReadToContainer
        (
            type_,
            pathname_,
            name_,
            data,
            start,
            count
        );
    }

    Foam::string type_{};

    Foam::string pathname_{};

    Foam::string name_{};

};


struct InitOffsets
:
    public InitStrategy
{
    InitOffsets() = default;

    explicit InitOffsets(const bool accumulate) : accumulate_{accumulate} {}

    Foam::Offsets offsets() { return offsets_; }

private:

    void execute(index_container&, labelPair&) final;

    Foam::Offsets offsets_{};

    bool accumulate_{};

};


template<typename FieldType = InitStrategy::index_container>
struct InitPrimitivesFromADIOS
:
    public InitStrategy
{
    InitPrimitivesFromADIOS() = default;

    explicit InitPrimitivesFromADIOS
    (
        const Foam::string& type,
        const Foam::string& pathname,
        const Foam::string& name
    )
    :
        type_{type},
        pathname_{pathname},
        name_{name}
    {}

private:

    void execute
    (
        FieldType& data,
        Foam::InitStrategy::labelPair& start_count
    ) final
    {
        auto start = (start_count.first != -1) ?
                     labelList({start_count.first}) :
                     labelList({});
        auto count = (start_count.second != -1) ?
                     labelList({start_count.second}) :
                     labelList({});
        data.resize(count[0]);
        Foam::adiosReadPrimitives
        (
            type_,
            pathname_,
            name_,
            data.data(),
            start,
            count
        );
    }

    Foam::string type_{};

    Foam::string pathname_{};

    Foam::string name_{};

};


template<typename FieldType = InitStrategy::index_container>
struct NaivePartitioningFromADIOS
:
    public InitStrategy
{
    NaivePartitioningFromADIOS() = default;

    explicit NaivePartitioningFromADIOS
    (
        const Foam::string& type,
        const Foam::string& pathname,
        const Foam::string& name
    )
    :
        type_{type},
        pathname_{pathname},
        name_{name}
    {}

private:

    void execute
    (
        FieldType& data,
        Foam::InitStrategy::labelPair& start_count
    ) final
    {
        auto adiosStreamPtr = adiosReading{}.createStream();
        adiosStreamPtr->open(type_, pathname_);

        // Naive partitioning based on total size of input data
        label total_size = adiosStreamPtr->getBufferSize(name_, data.data());
        total_size--;
        label partition_size = total_size / Pstream::nProcs();
        labelList start(1, partition_size * Pstream::myProcNo());
        labelList count = (Pstream::myProcNo() == Pstream::nProcs()-1) ?
                          labelList({total_size - start[0]}) :
                          labelList({partition_size});
        count[0] += 1;
        data.resize(count[0]);

        adiosStreamPtr->transfer(name_, data, start, count);
        adiosStreamPtr->close();
    }

    Foam::string type_{};

    Foam::string pathname_{};

    Foam::string name_{};

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
