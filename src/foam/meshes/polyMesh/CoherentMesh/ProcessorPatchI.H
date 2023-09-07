
// * * * * * * * * * * * * * * Free Functions  * * * * * * * * * * * * * * //

template<typename SomeList, typename IndexList>
SomeList Foam::extractor(const SomeList& input, const IndexList& extractorList)
{
    typedef typename IndexList::value_type index_type;
    SomeList output;
    output.resize(extractorList.size());
    std::transform
    (
        std::begin(extractorList),
        std::end(extractorList),
        std::begin(output),
        [&input](const index_type& id)
        {
            return input[id];
        }
    );
    return output;
}

// * * * * * * * * * * * * Public Member Functions * * * * * * * * * * * * //

template<typename FaceList>
FaceList Foam::ProcessorPatch::extractFaces(const FaceList& input)
{
    return Foam::extractor(input, localFaceIDs_);
}

// ************************************************************************* //
