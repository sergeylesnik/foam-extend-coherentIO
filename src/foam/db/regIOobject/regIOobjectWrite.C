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

Description
    write function for regIOobjects

\*---------------------------------------------------------------------------*/

#include "regIOobject.H"
#include "objectRegistry.H"
#include "OSspecific.H"
#include "OFstream.H"
#include "SliceStream.H"
#include "Pstream.H"

#include "profiling.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool Foam::regIOobject::writeObject
(
    IOstream::streamFormat fmt,
    IOstream::versionNumber ver,
    IOstream::compressionType cmp
) const
{
    return writeObject(IOstreamOption(fmt, ver, cmp));
}


bool Foam::regIOobject::writeObject(IOstreamOption streamOpt) const
{
    if (!good())
    {
        SeriousErrorIn("regIOobject::write()")
            << "bad object " << name()
            << endl;

        return false;
    }

    if (instance().empty())
    {
        SeriousErrorIn("regIOobject::write()")
            << "instance undefined for object " << name()
            << endl;

        return false;
    }

    if
    (
        instance() != time().timeName()
     && instance() != time().system()
     && instance() != time().caseSystem()
     && instance() != time().constant()
     && instance() != time().caseConstant()
    )
    {
        const_cast<regIOobject&>(*this).instance() = time().timeName();
    }

    if (time().writeFormat() == IOstream::COHERENT)
    {
        if (Pstream::master())
        {
            mkDir(path());
        }
    }
    else
    {
        mkDir(path());
    }

    if (OFstream::debug)
    {
        Info<< "regIOobject::write() : "
            << "writing file " << objectPath() << " ..." << nl;
    }

    // Try opening an OFstream for object.
    // May open different streams in the derived classes.
    bool osGood = writeToStream
    (
        objectPath(),
        ios_base::out|ios_base::trunc,
        streamOpt
    );


    if (OFstream::debug)
    {
        Info<< " .... written" << endl;
    }

    // Only update the lastModified_ time if this object is re-readable,
    // i.e. lastModified_ is already set
    if (watchIndex_ != -1)
    {
        time().setUnmodified(watchIndex_);
    }

    return osGood;
}


bool Foam::regIOobject::write() const
{
    addProfile2(io, "Foam::regIOobject::write()");

    bool writeBulkData = false;
    auto destination = IOstreamOption::TIME;
    if (time().controlDict().lookupOrDefault("writeBulkData", false))
    {
        writeBulkData = true;
        destination = IOstreamOption::CASE;
    }

    IOstreamOption streamOpt
    (
        time().writeFormat(),
        IOstream::currentVersion,
        time().writeCompression(),
        IOstreamOption::SYNC,  // ToDoIO Store this default in foamTime?
        destination
    );

    if (time().writeFormat() == IOstream::COHERENT)
    {
        auto repo = SliceStreamRepo::instance();
        repo->open(writeBulkData);
    }

    bool ok = writeObject(streamOpt);

    if (time().writeFormat() == IOstream::COHERENT)
    {
        auto repo = SliceStreamRepo::instance();
        repo->close(writeBulkData);
    }

    return ok;
}


bool Foam::regIOobject::writeToStream
(
    const fileName& pathname,
    ios_base::openmode mode,
    IOstream::streamFormat fmt,
    IOstream::versionNumber ver,
    IOstream::compressionType cmp
) const
{
    return writeToStream(pathname, mode, IOstreamOption(fmt, ver, cmp));
}


bool Foam::regIOobject::writeToStream
(
    const fileName& pathname,
    ios_base::openmode mode,
    IOstreamOption streamOpt
) const
{
    // Try opening an OFstream for object
    // Stream open for over-write.  HJ, 17/Aug/2010
    OFstream os
    (
        objectPath(),
        ios_base::out|ios_base::trunc,
        streamOpt
    );

    // If any of these fail, return (leave error handling to Ostream class)
    if (!os.good())
    {
        return false;
    }

    if (!writeHeader(os))
    {
        return false;
    }

    // Write the data to the Ostream
    if (!writeData(os))
    {
        return false;
    }

    writeEndDivider(os);

    return os.good();
}

// ************************************************************************* //
