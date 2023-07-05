
#include "sliceMeshHelper.H"

// * * * * * * * * * * * * * * Free Functions * * * * * * * * * * * * * * * //

void Foam::partitionByFirst(Foam::pairVector<Foam::label, Foam::label>& input)
{
    std::stable_partition
    (
        input.begin(),
        input.end(),
        [](const auto& n)
        {
            return n.first>0;
        }
    );
    std::stable_sort
    (
        std::partition_point
        (
            input.begin(),
            input.end(),
            [](const auto& n)
            {
                return n.first>0;
            }
        ),
        input.end(),
        [](const auto& n, const auto& m)
        {
            return n.first>m.first;
        }
    );
}


void
Foam::renumberFaces(Foam::faceList& faces, const std::vector<Foam::label>& map)
{
    std::for_each
    (
        faces.begin(),
        faces.end(),
        [&map](auto& face)
        {
            std::transform
            (
                face.begin(),
                face.end(),
                face.begin(),
                [&map](const auto& id)
                {
                    return map[id];
                }
            );
        }
    );
}


std::vector<Foam::label>
Foam::permutationOfSorted(const Foam::labelList& input)
{
    std::vector<Foam::label> indices{};
    indexIota(indices, input.size(), 0);
    indexSort(indices, input);
    return indices;
}

// ************************************************************************* //