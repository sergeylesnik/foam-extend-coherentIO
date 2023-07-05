
#include "adiosWriting.H"

#include "adiosOutputFeatures.H"
#include "adiosFileStream.H"


std::unique_ptr<Foam::adiosStream>
Foam::adiosWriting::createStream()
{
    std::unique_ptr<Foam::adiosFeatures>
    file = std::make_unique<Foam::adiosOutputFeatures>();
    return std::make_unique<Foam::adiosFileStream>(file);
}


