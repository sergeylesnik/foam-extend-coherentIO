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

#include "sliceReadPrimitives.H"
#include "SliceReading.H"
#include "SliceStream.H"

#include "foamString.H"


template<typename T>
void _implReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    T* buf,
    const Foam::List<Foam::label>& start,
    const Foam::List<Foam::label>& count
)
{
    auto sliceStreamPtr = Foam::SliceReading{}.createStream();
    sliceStreamPtr->access(type, pathname);
    if (start.size()>0 && count.size()>0)
    {
        sliceStreamPtr->get(blockId, buf, start, count);
    }
    else
    {
        sliceStreamPtr->get(blockId, buf);
    }
    sliceStreamPtr->bufferSync();
}


void Foam::sliceReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    Foam::scalar* buf,
    const Foam::List<Foam::label>& start,
    const Foam::List<Foam::label>& count
)
{
    _implReadPrimitives(type, pathname, blockId, buf, start, count);
}


void Foam::sliceReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    Foam::label* buf,
    const Foam::List<Foam::label>& start,
    const Foam::List<Foam::label>& count
)
{
    _implReadPrimitives(type, pathname, blockId, buf, start, count);
}


void Foam::sliceReadPrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    char* buf,
    const Foam::List<Foam::label>& start,
    const Foam::List<Foam::label>& count
)
{
    _implReadPrimitives(type, pathname, blockId, buf, start, count);
}

// ************************************************************************* //
