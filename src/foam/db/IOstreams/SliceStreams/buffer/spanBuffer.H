#ifndef spanBuffer_H
#define spanBuffer_H

#include <memory>
#include <numeric>

#include "label.H"
#include "labelList.H"

#include "SliceBuffer.H"

namespace Foam
{

template<typename ForwardIterator>
void check_extent_of_range
(
    ForwardIterator iter1,
    ForwardIterator iter2,
    label extent
)
{
    if (std::distance(iter1, iter2)<extent)
    {
        FatalErrorIn
        (
        "check_extent_of_range: range does not satsify the required extent."
        )
        << abort(FatalError);
    }
}


// assumes mapping to be sorted by value (and accordingly sorted input)
template<typename ForwardIterator, typename OutputIterator>
void mapped_copy
(
    ForwardIterator input_iter,
    const ForwardIterator input_end,
    OutputIterator output_iter,
    const OutputIterator output_end,
    const labelList& mapping,
    const label serialization = 1,
    const bool masked = false
)
{
    // first, check if the maximum index in mapping
    // actually fits to the output range.
    auto max_index = *std::max_element(mapping.begin(), mapping.end());
    check_extent_of_range(output_iter, output_end, max_index);

    // second map or mask loop
    auto output_begin = output_iter;
    for (const auto& next_pos: mapping)
    {
        auto cur_pos = std::distance(output_begin, output_iter) / serialization;
        auto n = next_pos - cur_pos;

        // advance to mapped position (no advancement if masked)
        auto n_mapped = n * (1 - masked); // masked ? 0 : n
        std::advance(output_iter, n_mapped * serialization);
        // copy and advance one element (n elements if masked)
        auto n_copy = 1 + (n - 1) * masked; // masked ? n : 1
        std::copy_n(input_iter, n_copy * serialization, output_iter);
        std::advance(input_iter, n_copy * serialization);
        std::advance(output_iter, n_copy * serialization);
        // advance over masked element (only if masked)
        std::advance(output_iter, masked * serialization);
    }

    // masked ? "finite value" : 0
    auto remaining_extent = std::distance(output_iter, output_end) * masked;
    // third, copy remaining elements if masking was turned on
    check_extent_of_range(input_iter, input_end, remaining_extent);
    std::copy(input_iter, input_end, output_iter);
}


template<typename DataType>
adios2::Variable<DataType>
inquire_define_variable
(
    adios2::IO* io,
    adios2::Variable<DataType>& variable,
    const Foam::string blockId,
    const adios2::Dims& shape,
    const adios2::Dims& start,
    const adios2::Dims& count
)
{
    variable = io->InquireVariable<DataType>( blockId );
    if ( !variable )
    {
        variable = io->DefineVariable<DataType>( blockId, shape, start, count );
    }
    return variable;
}

template<typename DataType>
class spanBuffer
:
    public SliceBuffer
{

    adios2::Dims shape_;

    adios2::Dims start_;

    adios2::Dims count_;

    label serialization_;

    adios2::Variable<DataType> variable_;

    typename adios2::Variable<DataType>::Span span_;

    // Reading
    void v_transfer
    (
        adios2::Engine* engine,
        DataType* data,
        const labelList& mapping = {},
        const bool masked = false
    ) final
    {
        FatalErrorIn
        (
        "spanBuffer is only for output transfers. Reading through transfer( adios2::Engine*, DataType* const ) is disabled."
        )
        << abort(FatalError);
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
        const DataType* data_end = data + count_[0] * serialization_ + 1;
        mapped_copy
        (
            data,
            data_end,
            span_.begin(),
            span_.end(),
            mapping,
            serialization_,
            masked
        );
    }


public:

    spanBuffer
    (
        adios2::IO* io,
        adios2::Engine* engine,
        const Foam::string blockId,
        const Foam::labelList& shape,
        const Foam::labelList& start,
        const Foam::labelList& count
    );

    ~spanBuffer() = default;

    label size() final;
};
}

template<typename DataType>
Foam::spanBuffer<DataType>::spanBuffer
(
    adios2::IO* io,
    adios2::Engine* engine,
    const Foam::string blockId,
    const Foam::labelList& shape,
    const Foam::labelList& start,
    const Foam::labelList& count
)
    : shape_{ toDims( shape ) }
    , start_{ toDims( start ) }
    , count_{ toDims( count ) }
    , serialization_
      {
          std::accumulate
          (
              count_.begin() + 1,
              count_.end(),
              1,
              std::multiplies<label>()
          )
      }
    , variable_
      {
          inquire_define_variable
          (
              io,
              variable_,
              blockId,
              shape_,
              start_,
              count_
          )
      }
    , span_{ engine->Put( variable_ ) }
{}


template<typename DataType>
Foam::label Foam::spanBuffer<DataType>::size()
{
    if (!variable_) { return 0; }
    return variable_.Count()[0];
}


#endif
