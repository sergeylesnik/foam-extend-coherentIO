
// TODO: Decision of MPI_Datatype must depend on deduced Type.
template<class Type>
std::map<Foam::label, std::vector<Type>>
Foam::nonblockConsensus
(
    const std::map<Foam::label,
    std::vector<Type>>& data,
    MPI_Datatype dtype
)
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
            &(msg.second[0]),
            msg.second.size(),
            dtype,
            msg.first,
            tag,
            MPI_COMM_WORLD,
            &request
        );
        issRequests.push_back(request);
    }

    std::map<Foam::label, std::vector<Type>> recvBuffer{};
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
            MPI_Get_count(&status, dtype, &count);
            std::vector<Type> recvMessage(count);
            MPI_Recv
            (
                &(recvMessage[0]),
                count,
                dtype,
                status.MPI_SOURCE,
                tag,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );
            recvBuffer[status.MPI_SOURCE] = std::move(recvMessage);
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
