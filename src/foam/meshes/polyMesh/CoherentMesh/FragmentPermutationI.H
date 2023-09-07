
// Transformations to FragmentPermutation

template<typename Container>
void Foam::FragmentPermutation::permute(Container &input)
{
    applyPermutation(input, facePermutation_);
}


template<typename Container>
void Foam::FragmentPermutation::retrieveNeighbours(Container &neighbours)
{
    const auto polyNeighboursBegin = polyNeighboursAndPatches_.begin();
    const auto polyPatchBegin = findPatchBegin();
    neighbours.resize(std::distance(polyNeighboursBegin, polyPatchBegin));
    std::copy(polyNeighboursBegin, polyPatchBegin, neighbours.begin());
}


template<typename Container>
void Foam::FragmentPermutation::retrievePatches(Container &patches)
{
    const auto polyPatchBegin = findPatchBegin();
    const auto polyPatchEnd = polyNeighboursAndPatches_.end();
    patches.resize(std::distance(polyPatchBegin, polyPatchEnd));
    std::copy(polyPatchBegin, polyPatchEnd, patches.begin());
}

// ************************************************************************* //
