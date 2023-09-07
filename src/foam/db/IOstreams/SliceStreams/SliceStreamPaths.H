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

Class
    Foam::SliceStreamPaths

Description
    Class to manage and fix file naming for ADIOS2 files

SourceFiles
    SliceStreamPaths.C

\*---------------------------------------------------------------------------*/

#ifndef SliceStreamPaths_H
#define SliceStreamPaths_H

#include "fileName.H"

namespace Foam
{

class SliceStreamPaths
{

    // Mesh data file name
    const Foam::fileName meshPathname_{"data.bp"};

    // Field data file name
    const Foam::fileName dataPathname_{"data.bp"};

    // State like member that keeps the current file name
    Foam::fileName pathname_{"data.bp"};

    // State whether method checkFiles() was called before
    bool filesChecked_{false};

    // State if field data file is present
    bool dataPresent_{false};

    // State if mesh data file is present
    bool meshPresent_{false};

    // Check mesh and field files
    void checkFiles();

public:

    // Set the state of pathname_
    void setPathName(const fileName&);

    // Get the state of pathname_
    const fileName& getPathName() const;

    // Return mesh data file name
    fileName meshPathname(const fileName& path = "");

    // Return field data file name
    fileName dataPathname(const fileName& path = "");

    // Check if field data file is present
    bool dataPresent();

    // Check if mesh data file is present
    bool meshPresent();

};

}

#endif
