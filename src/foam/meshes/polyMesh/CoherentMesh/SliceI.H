
// * * * * * * * * * * * * * * Free Functions  * * * * * * * * * * * * * * //

template<typename Container>
Foam::label Foam::numCellsFromSlice
(
    const Container& neighbours,
    const Foam::Slice& slice
)
{
    return std::count_if
           (
               std::begin(neighbours),
               std::end(neighbours),
               slice
           );
}


template<typename Container, typename Operator>
void Foam::appendTransformed
(
    Container& ret,
    Container& input,
    Operator operation
)
{
    std::transform
    (
        std::begin(input),
        std::end(input),
        std::begin(input),
        operation
    );
    ret.append(input);
}

// * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * * //

template<typename Container>
typename std::enable_if<Foam::is_range<Container>::value, Container>::type
Foam::Slice::convert(Container& list) const
{
    typedef typename Container::value_type index_type;
    std::transform
    (
        std::begin(list),
        std::end(list),
        std::begin(list),
        [this](const index_type& id)
        {
            return convert(id);
        }
    );
    return list;
}


template<typename Container>
void Foam::Slice::append(const Container& list)
{
    mapping_->append(list);
}

// ************************************************************************* //
