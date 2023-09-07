
#include "sliceMeshHelper.H"

#include "Slice.H"

#include <numeric>


template<typename Container>
Foam::pairVector<Foam::label, Foam::label>
Foam::generateIndexedPairs(const Container& input)
{
    std::vector<Foam::label> indices{};
    indexIota(indices, input.size(), 0);
    Foam::pairVector<Foam::label, Foam::label> indexedPairs{};
    indexedPairs.reserve(input.size());
    std::transform
    (
        input.begin(),
        input.end(),
        indices.begin(),
        std::back_inserter(indexedPairs),
        [](const Foam::label& inputId, const Foam::label& index)
        {
            return std::make_pair(inputId, index);
        }
    );
    return indexedPairs;
}


template<typename Container>
void Foam::indexIota
(
    Container& input,
    const size_t& size,
    const typename Container::value_type& start
)
{
    input.resize(size);
    std::iota(input.begin(), input.end(), start);
}


template<typename Container1, typename Container2>
Foam::pairVector
<
    typename Container1::value_type,
    typename Container2::value_type
>
Foam::zip(const Container1& container1, const Container2& container2)
{
    typedef typename Container1::value_type type1;
    typedef typename Container2::value_type type2;
    pairVector<type1, type2> zipped{};
    std::transform
    (
        container1.begin(),
        container1.end(),
        container2.begin(),
        std::back_inserter(zipped),
        [](const type1& value1, const type2& value2)
        {
            return std::pair<type1, type2>{ value1, value2 };
        }
    );
    return zipped;
}


template<typename IndexContainer, typename DataContainer>
void Foam::indexSort(IndexContainer& indices, const DataContainer& data)
{
    typedef typename IndexContainer::value_type index_type;
    std::stable_sort
    (
        indices.begin(),
        indices.end(),
        [&data](const index_type& i, const index_type& j)
        {
            return data[i]<data[j];
        }
    );
}

template<typename InputIterator, typename Container>
void Foam::permutationOfSorted
(
    InputIterator begin,
    InputIterator end,
    const Container& data
)
{
    std::iota(begin, end, 0);
    std::stable_sort
    (
        begin,
        end,
        [&data] (const label& i, const label& j)
        {
            return data[i] < data[j];
        }
    );
}

template<typename ValueType, typename UnaryOperation>
std::vector<Foam::label> Foam::extractNth
(
    const std::vector<ValueType>& input,
    UnaryOperation unary_op
)
{
    std::vector<Foam::label> indices; // Only takes containers of labels! TODO
    indices.reserve(input.size());
    std::transform
    (
        input.begin(),
        input.end(),
        std::back_inserter(indices),
        unary_op
    );
    return indices;
}


template<typename IterType, typename ValType>
void Foam::findValueExtend
(
    IterType& begin,
    IterType& end,
    const IterType& fin,
    const ValType& value
)
{
    begin = std::find_if
            (
                begin,
                fin,
                [value](const ValType& element)
                {
                    return element == value;
                }
            );
    end = std::find_if
          (
              begin,
              fin,
              [value](const ValType& element)
              {
                  return element != value;
              }
          );
}

#include <cassert>

template<typename Container, typename IndexContainer>
void Foam::applyPermutation(Container& data, const IndexContainer& permutation)
{
    Container output(permutation.size());
    for (Foam::label i = 0; i<permutation.size(); ++i)
    {
        output[i] = std::move(data[permutation[i]]);
    }
    std::move
    (
        output.begin(),
        output.end(),
        data.begin()
    );
}


// this only recursively binds to (non-range) arithmetic types, currently.
template
<
    typename ForwardIterator,
    typename OutputIterator,
    typename UnaryPredicate
>
typename std::enable_if
<
    std::is_arithmetic
    <
        typename std::iterator_traits<ForwardIterator>::value_type
    >::value,
    void
>::type
Foam::subset
(
    ForwardIterator input_iter,
    const ForwardIterator input_end,
    OutputIterator output_iter,
    UnaryPredicate unary_predicate
)
{
    std::copy_if
    (
        input_iter,
        input_end,
        output_iter,
        unary_predicate
    );
}


template
<
    typename ForwardIterator,
    typename OutputIterator,
    typename UnaryPredicate
>
typename std::enable_if
<
    Foam::is_range
    <
        typename std::iterator_traits<ForwardIterator>::value_type
    >::value,
    void
>::type
Foam::subset
(
    ForwardIterator input_iter,
    const ForwardIterator input_end,
    OutputIterator output_iter,
    UnaryPredicate unary_predicate
)
{
    while(input_iter != input_end)
    {
        Foam::subset
        (
            input_iter->begin(),
            input_iter->end(),
            output_iter,
            unary_predicate
        );
        ++input_iter;
    }
}


// ***************************************************************** //
