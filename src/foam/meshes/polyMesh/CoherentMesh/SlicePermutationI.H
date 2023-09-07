
// Transformations to SlicePermutation

template<typename Container>
void Foam::SlicePermutation::permute(Container &input)
{
    applyPermutation(input, permutationToSlice_);
}


template<typename MeshElement>
void Foam::SlicePermutation::apply(MeshElement &input)
{
    permute(input);
}

// ************************************************************************* //
