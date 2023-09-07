
#include "SliceBuffer.H"


adios2::Dims Foam::toDims(const Foam::labelList& list)
{
    adios2::Dims dims(list.size());
    std::copy(list.begin(), list.end(), dims.begin());
    return dims;
}


void Foam::SliceBuffer::v_transfer
(
    adios2::Engine* engine,
    const char* data,
    const labelList& mapping,
    const bool masked
)
{};
