
#include "nonblockConsensus.H"

#include "Offsets.H"
#include "Slice.H"


std::map<Foam::label, Foam::label>
Foam::nonblockConsensus(const std::map<Foam::label, Foam::label>& data)
{
    int tag = 314159;
    bool barrier_activated = false;
    MPI_Barrier(MPI_COMM_WORLD);
    std::vector<MPI_Request> issRequests{};
    for (const auto& msg: data)
    {
        MPI_Request request;
        MPI_Issend
        (
            &(msg.second),
            1,
            MPI_INT,
            msg.first,
            tag,
            MPI_COMM_WORLD,
            &request
        );
        issRequests.push_back(request);
    }

    std::map<Foam::label, Foam::label> recvBuffer{};
    MPI_Request barrier = MPI_REQUEST_NULL;
    int done = 0;
    while (!done)
    {
        int flag = 0;
        int count = 0;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &status);
        if (flag)
        {
            MPI_Get_count(&status, MPI_INT, &count);
            Foam::label numberOfPartitionFaces{};
            MPI_Recv
            (
                &numberOfPartitionFaces,
                count,
                MPI_INT,
                status.MPI_SOURCE,
                tag,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );
            recvBuffer[status.MPI_SOURCE] = numberOfPartitionFaces;
        }

        if (barrier_activated)
        {
            MPI_Test(&barrier, &done, MPI_STATUS_IGNORE);
        }
        else
        {
            int sent;
            MPI_Testall
            (
                issRequests.size(),
                issRequests.data(),
                &sent,
                MPI_STATUSES_IGNORE
            );
            if (sent)
            {
                MPI_Ibarrier(MPI_COMM_WORLD, &barrier);
                barrier_activated = true;
            }
        }
    }

    return recvBuffer;
}

// ************************************************************************* //
