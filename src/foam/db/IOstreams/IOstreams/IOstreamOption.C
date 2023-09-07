/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.1
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "IOstreamOption.H"
#include "messageStream.H"
#include "Switch.H"

// * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * * //

Foam::IOstreamOption::streamFormat
Foam::IOstreamOption::formatEnum(const word& format)
{
    if (format == "ascii")
    {
        return IOstreamOption::ASCII;
    }
    else if (format == "binary")
    {
        return IOstreamOption::BINARY;
    }
    else if (format == "coherent")
    {
        return IOstreamOption::COHERENT;
    }
    else
    {
        WarningIn("IOstreamOption::formatEnum(const word&)")
            << "bad format specifier '" << format << "', using 'ascii'"
            << endl;

        return IOstreamOption::ASCII;
    }
}


Foam::IOstreamOption::compressionType
Foam::IOstreamOption::compressionEnum(const word& compression)
{
    // get Switch (bool) value, but allow it to fail
    Switch sw(compression, true);

    if (sw.valid())
    {
        return sw ? IOstreamOption::COMPRESSED : IOstreamOption::UNCOMPRESSED;
    }
    else if (compression == "uncompressed")
    {
        return IOstreamOption::UNCOMPRESSED;
    }
    else if (compression == "compressed")
    {
        return IOstreamOption::COMPRESSED;
    }
    else
    {
        WarningIn("IOstreamOption::compressionEnum(const word&)")
            << "bad compression specifier '" << compression
            << "', using 'uncompressed'"
            << endl;

        return IOstreamOption::UNCOMPRESSED;
    }
}


Foam::IOstreamOption::streamMode
Foam::IOstreamOption::modeEnum(const word& mode)
{
    if (mode == "sync")
    {
        return IOstreamOption::SYNC;
    }
    else if (mode == "deferred")
    {
        return IOstreamOption::DEFERRED;
    }
    else
    {
        WarningInFunction
            << "bad mode specifier '" << mode << "', using 'sync'"
            << endl;

        return IOstreamOption::SYNC;
    }
}


Foam::IOstreamOption::dataDestination
Foam::IOstreamOption::destinationEnum(const word& destination)
{
    if (destination == "time")
    {
        return IOstreamOption::TIME;
    }
    else if (destination == "case")
    {
        return IOstreamOption::CASE;
    }
    else
    {
        WarningInFunction
            << "bad data specifier '" << destination << "', using 'sync'"
            << endl;

        return IOstreamOption::TIME;
    }
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //

Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const IOstreamOption::streamFormat& sf
)
{
    if (sf == IOstreamOption::ASCII)
    {
        os << "ascii";
    }
    else if (sf == IOstreamOption::BINARY)
    {
        os << "binary";
    }
    else if (sf == IOstreamOption::COHERENT)
    {
        os << "coherent";
    }

    return os;
}


Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const IOstreamOption::versionNumber& vn
)
{
    os << vn.str().c_str();
    return os;
}


// ************************************************************************* //
