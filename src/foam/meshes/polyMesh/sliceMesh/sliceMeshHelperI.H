
#include "sliceMeshHelper.H"

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
        [](const auto& inputId, const auto& index)
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
        [](const auto& value1, const auto& value2)
        {
            return std::pair<type1, type2>{ value1, value2 };
        }
    );
    return zipped;
}


template<typename IndexContainer, typename DataContainer>
void Foam::indexSort(IndexContainer& indices, const DataContainer& data)
{
    std::stable_sort
    (
        indices.begin(),
        indices.end(),
        [&data](const auto& i, const auto& j)
        {
            return data[i]<data[j];
        }
    );
}


template<typename ValueType, typename UnaryOperation>
auto Foam::extractNth
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
                [value](const auto& element)
                {
                    return element == value;
                }
            );
    end = std::find_if
          (
              begin,
              fin,
              [value](const auto& element)
              {
                  return element != value;
              }
          );
}


template<typename Container, typename IndexContainer>
void Foam::applyPermutation(Container& data, const IndexContainer& permutation)
{
    typedef typename IndexContainer::value_type size_type;
    IndexContainer indices{permutation};
    auto size = static_cast<size_type>(indices.size());
    for (size_type rootIndex = 0; rootIndex<size; rootIndex++)
    {
        // Swap data elements and indices until indices match
        auto currentIndex = rootIndex;
        while (rootIndex != indices[currentIndex])
        {
            using std::swap;
            auto leafIndex = indices[currentIndex];
            swap(data[currentIndex], data[leafIndex]);
            swap(indices[currentIndex], currentIndex);
        }
        indices[currentIndex] = currentIndex;
    }
}


template<typename Container, typename UnaryPredicate>
std::set<Foam::label>
Foam::pointSubset(const Container& list, UnaryPredicate unary_predicate)
{
    std::set<Foam::label> pointIDs{};
    std::copy_if
    (
        list.begin(),
        list.end(),
        std::inserter(pointIDs, pointIDs.end()),
        unary_predicate
    );
    return pointIDs;
}


template<typename UnaryPredicate>
std::set<Foam::label>
Foam::pointSubset(const Foam::faceList& faces, UnaryPredicate unary_predicate)
{
    std::set<Foam::label> pointIDs{};
    for (const auto& face: faces)
    {
        auto tmp = Foam::pointSubset(face, unary_predicate);
        pointIDs.insert(std::begin(tmp), std::end(tmp));
    }
    return pointIDs;
}


template<typename Container>
std::set<Foam::label> Foam::pointSubset(const Container& data)
{
    return Foam::pointSubset(data, [](auto){ return true; });
}

// ************************************************************************* //
