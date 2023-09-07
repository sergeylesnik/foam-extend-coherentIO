
#include <algorithm>

template<typename Container>
void Foam::sliceMap::append(const Container& list)
{
    typedef typename Container::value_type index_type;
    Foam::label currId = numNativeEntities_ + mapping_.size();
    std::transform
    (
        std::begin(list),
        std::end(list),
        std::inserter(mapping_, std::end(mapping_)),
        [&currId](const index_type& id) mutable
        {
            auto ret = std::pair<label, label>({id, currId});
            ++currId;
            return ret;
        }
    );
}
