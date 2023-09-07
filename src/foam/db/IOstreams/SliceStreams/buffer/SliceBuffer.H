
#ifndef SliceBuffer_H
#define SliceBuffer_H

#include "adios2.h"

#include <memory>

#include "label.H"
#include "labelList.H"
#include "vector.H"
#include "messageStream.H" // FatalError()

namespace Foam
{

// Convert Foam::labelList to adios2::Dims
adios2::Dims toDims(const Foam::labelList& list);

class SliceBuffer
{
    virtual void v_transfer
    (
        adios2::Engine*,
        const char*,
        const labelList& = {},
        const bool = false
    );

    virtual void v_transfer
    (
        adios2::Engine*,
        char*,
        const labelList& = {},
        const bool = false
    ) {};


    virtual void v_transfer
    (
        adios2::Engine*,
        const label*,
        const labelList& = {},
        const bool = false
    ) {};


    virtual void v_transfer
    (
        adios2::Engine*,
        label*,
        const labelList& = {},
        const bool = false
    ) {};


    virtual void v_transfer
    (
        adios2::Engine*,
        const scalar*,
        const labelList& = {},
        const bool = false
    ) {};


    virtual void v_transfer
    (
        adios2::Engine*,
        scalar*,
        const labelList& = {},
        const bool = false
    ) {};

public:

    SliceBuffer() = default;

    virtual ~SliceBuffer() = default;

    virtual label size() = 0;


    // Note: constness is included in with type deduction of DataType
    template<typename DataType>
    void transfer
    (
        adios2::Engine* engine,
        DataType* const data,
        const labelList& mapping = {},
        const bool masked = false
    )
    {
        if (!engine)
        {
            FatalErrorInFunction
                << "Engine is nullptr" << abort(FatalError);
        }
        if (!data && size() > 0)
        {
            FatalErrorInFunction
                << "Data is nullptr" << abort(FatalError);
        }
        v_transfer(engine, data, mapping, masked);
    }
};

}

#endif
