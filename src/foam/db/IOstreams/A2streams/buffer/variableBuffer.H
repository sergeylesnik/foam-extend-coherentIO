#ifndef variableBuffer_H
#define variableBuffer_H

#include <memory>
#include <type_traits>

#include "label.H"
#include "labelList.H"

#include "adiosBuffer.H"

namespace Foam
{

template<typename VariableType, typename DataType>
void transferFromVariable
(
    adios2::Engine* const engine,
    VariableType& variable,
    adios2::Dims& shape,
    adios2::Dims& start,
    adios2::Dims& count,
    DataType* data
)
{
    if (variable)
    {
        engine->Get(variable, data, adios2::Mode::Deferred);
    }
}


template<typename VariableType, typename DataType>
void transferToVariable
(
    adios2::Engine* const engine,
    VariableType& variable,
    adios2::Dims& shape,
    adios2::Dims& start,
    adios2::Dims& count,
    DataType* data
)
{
    engine->Put(variable, data, adios2::Mode::Deferred);
}


template<typename VariableType, typename DataType>
void transferToSpan
(
    adios2::Engine* const engine,
    VariableType& variable,
    adios2::Dims& shape,
    adios2::Dims& start,
    adios2::Dims& count,
    DataType* data
)
{
    auto span = engine->Put(variable);
    for (Foam::label i = 0; i<count[0]; ++i)
    {
        span[i] = data[i];
    }
}


template<typename DataType>
class variableBuffer
:
    public adiosBuffer
{
    // Global shape of I/O data array in variable
    adios2::Dims shape_;

    // Local start of I/O data array in variable
    adios2::Dims start_;

    // Local count of I/O data array in variable
    adios2::Dims count_;

    // ADIOS2 variable defining I/O data layout
    adios2::Variable<DataType> variable_;

    // Reading
    void v_transfer
    (
        adios2::Engine* engine,
        DataType* data,
        const labelList& mapping = {},
        const bool masked = false
    ) final
    {
        transferFromVariable(engine, variable_, shape_, start_, count_, data);
    }

    // Writing
    void v_transfer
    (
        adios2::Engine* engine,
        const DataType* data,
        const labelList& mapping = {},
        const bool masked = false
    ) final
    {
        transferToVariable(engine, variable_, shape_, start_, count_, data);
    }


public:

    variableBuffer() = default;

    ~variableBuffer() = default;

    variableBuffer
    (
        adios2::IO* io,
        adios2::Engine* engine,
        const Foam::string blockId
    );

    variableBuffer
    (
        adios2::IO* io,
        adios2::Engine* engine,
        const Foam::string blockId,
        const Foam::labelList& start,
        const Foam::labelList& count
    );

    variableBuffer
    (
        adios2::IO* io,
        adios2::Engine* engine,
        const Foam::string blockId,
        const Foam::labelList& shape,
        const Foam::labelList& start,
        const Foam::labelList& count
    );

    label size() final;
};
}


template<typename DataType>
Foam::variableBuffer<DataType>::variableBuffer
(
    adios2::IO* io,
    adios2::Engine* engine,
    const Foam::string blockId
)
{
    variable_ = io->InquireVariable<DataType>(blockId);
    if (variable_)
    {
        // Set to last step by default
        auto step = variable_.Steps() - 1;
        variable_.SetStepSelection({step, 1});
        shape_ = variable_.Shape();
        start_ = variable_.Start();
        count_ = variable_.Count();
    }
}


template<typename DataType>
Foam::variableBuffer<DataType>::variableBuffer
(
    adios2::IO* io,
    adios2::Engine* engine,
    const Foam::string blockId,
    const Foam::labelList& start,
    const Foam::labelList& count
)
:
    start_{toDims(start)},
    count_{toDims(count)}
{
    variable_ = io->InquireVariable<DataType>(blockId);
    if (variable_)
    {
        // Set to last step by default
        auto step = variable_.Steps() - 1;
        variable_.SetStepSelection({step, 1});
        variable_.SetSelection({start_, count_});
        shape_ = variable_.Shape();
    }
}


template<typename DataType>
Foam::variableBuffer<DataType>::variableBuffer
(
    adios2::IO* io,
    adios2::Engine* engine,
    const Foam::string blockId,
    const Foam::labelList& shape,
    const Foam::labelList& start,
    const Foam::labelList& count
)
:
    shape_{toDims(shape)},
    start_{toDims(start)},
    count_{toDims(count)}
{
    variable_ = io->InquireVariable<DataType>(blockId);
    if (variable_)
    {
        // Set to last step by default
        auto step = variable_.Steps() - 1;
        variable_.SetStepSelection({step, 1});
        variable_.SetSelection({start_, count_});
        shape_ = variable_.Shape();
    }
    else if (!variable_)
    {
        variable_ = io->DefineVariable<DataType>
                        (
                            blockId,
                            shape_,
                            start_,
                            count_
                        );
    }
}


template<typename DataType>
Foam::label Foam::variableBuffer<DataType>::size()
{
    Foam::label count = 0;
    if (variable_)
    {
        count = variable_.Count()[0];
    }
    return count;
}


#endif
