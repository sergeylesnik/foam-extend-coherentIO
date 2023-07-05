
#include "adiosReading.H"

#include "adiosInputFeatures.H"
#include "adiosFileStream.H"


std::unique_ptr<Foam::adiosStream>
Foam::adiosReading::createStream()
{
    std::unique_ptr<Foam::adiosFeatures>
    file = std::make_unique<Foam::adiosInputFeatures>();
    return std::make_unique<Foam::adiosFileStream>(file);
}


