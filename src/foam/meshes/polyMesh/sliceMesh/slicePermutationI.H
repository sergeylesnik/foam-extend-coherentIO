
// Transformations to slicePermutation

template<typename Container>
void Foam::slicePermutation::mapToSlice(Container &input)
{
    applyPermutation(input, permutationToSlice_);
}


template<typename MeshElement>
MeshElement Foam::slicePermutation::generateSlice(MeshElement &input)
{
    mapToSlice(input);
    return input;
}


// Transformations to polyMesh

template<typename Container>
void Foam::slicePermutation::mapToPoly(Container &input)
{
    applyPermutation(input, permutationToPoly_);
}


template<typename Container>
void Foam::slicePermutation::copyPolyNeighbours(Container &neighbours)
{
    const auto polyNeighboursBegin = polyNeighboursAndPatches_.begin();
    const auto polyPatchBegin = findPatchBegin();
    neighbours.resize(std::distance(polyNeighboursBegin, polyPatchBegin));
    std::copy(polyNeighboursBegin, polyPatchBegin, neighbours.begin());
}


template<typename Container>
void Foam::slicePermutation::copyPolyPatches(Container &patches)
{
    const auto polyPatchBegin = findPatchBegin();
    const auto polyPatchEnd = polyNeighboursAndPatches_.end();
    patches.resize(std::distance(polyPatchBegin, polyPatchEnd));
    std::copy(polyPatchBegin, polyPatchEnd, patches.begin());
}

// ************************************************************************* //