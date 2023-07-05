
#include <algorithm>

template<typename Container>
void Foam::sliceMap::append(const Container& list)
{
    Foam::label currId = numNativeEntities_ + mapping_.size();
    std::transform
    (
        std::begin(list),
        std::end(list),
        std::inserter(mapping_, std::end(mapping_)),
        [&currId](const auto& id) mutable
        {
            auto ret = std::pair<label, label>({id, currId});
            ++currId;
            return ret;
        }
    );
}
