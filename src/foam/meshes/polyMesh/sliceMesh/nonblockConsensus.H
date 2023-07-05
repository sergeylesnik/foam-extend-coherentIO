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

Function
    Foam::nonblockConsensus

Description
    Implements the nonblocking concensus exchange algorithm.
    @article
    {
        doi = {10.1145/1837853.1693476},
        author = {Hoefler, Torsten and Siebert, Christian and Lumsdaine, Andrew},
        title = {Scalable Communication Protocols for Dynamic Sparse Data Exchange},
        year = {2010},
        volume = {45},
        number = {5},
        journal = {SIGPLAN Not.},
        pages = {159–168}
    }

SourceFiles
    nonblockConsensus.C

\*---------------------------------------------------------------------------*/

#ifndef nonblockConsensus_H
#define nonblockConsensus_H

#include "label.H"
#include "labelList.H"

#include <vector>
#include <map>

#include "mpi.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declarations
class Offsets;

std::map<label, label>
numFacesToExchange(const Offsets&, const Offsets&, const labelList&);

template<class Type>
std::map<label, std::vector<Type>>
nonblockConsensus(const std::map<label, std::vector<Type>>&, MPI_Datatype);

std::map<label, label> nonblockConsensus(const std::map<label, label>&);

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "nonblockConsensusI.H"

#endif

// ************************************************************************* //

