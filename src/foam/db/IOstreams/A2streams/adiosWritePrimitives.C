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

#include "adiosWritePrimitives.H"

#include "adiosWriting.H"
#include "adiosFileStream.H"

#include "foamString.H"
#include "labelList.H"

template< typename T >
void _implWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    const Foam::labelList& shape,
    const Foam::labelList& start,
    const Foam::labelList& count,
    const T* buf
)
{
    auto adiosStreamPtr = Foam::adiosWriting{}.createStream();
    adiosStreamPtr->open( type, pathname );
    adiosStreamPtr->transfer( blockId, shape, start, count, buf );
    adiosStreamPtr->close();
}

//- Write local array
void Foam::adiosWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    const Foam::label count,
    const Foam::scalar* buf
)
{
    _implWritePrimitives
    (
        type,
        pathname,
        blockId,
        { count },
        { 0 },
        { count },
        buf
    );
}

//- Write local array
void Foam::adiosWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    const Foam::label count,
    const Foam::label* buf
)
{
    _implWritePrimitives
    (
        type,
        pathname,
        blockId,
        { count },
        { 0 },
        { count },
        buf
    );
}

//- Write global n-dimensional array
void Foam::adiosWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    const Foam::List<label> shape,
    const Foam::List<label> start,
    const Foam::List<label> count,
    const Foam::scalar* buf
)
{
    _implWritePrimitives
    (
        type,
        pathname,
        blockId,
        shape,
        start,
        count,
        buf
    );
}

//- Write global array
void Foam::adiosWritePrimitives
(
    const Foam::string type,
    const Foam::string pathname,
    const Foam::string blockId,
    const Foam::label shape,
    const Foam::label start,
    const Foam::label count,
    const Foam::scalar* buf
)
{
    _implWritePrimitives
    (
        type,
        pathname,
        blockId,
        { shape },
        { start },
        { count },
        buf
    );
}

// ************************************************************************* //
