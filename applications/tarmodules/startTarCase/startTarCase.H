/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2016 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.



\*---------------------------------------------------------------------------*/

#ifndef startTarCase_H
#define startTarCase_H


#include "objectRegistry.H"
#include "polyMesh.H"
#include "stringList.H"
#include "instantList.H"
#include "argList.H"
#include <unistd.h>
#include <sys/wait.h> 
#include <time.h>


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

    // Forward declaration of classes
    class Time;
/*---------------------------------------------------------------------------*\
                         Class startTarCase Declaration
\*---------------------------------------------------------------------------*/

class startTarCase
{
protected:

    // Private data
    
    const argList& args_;
   
    //new working directory
        string pathToRamDisk_;
        
        bool useRamDisk_;
        
        bool onlyRestartAndDataInTar_;
        
        scalar startTime_;
        
        //old working directory (e.g. PBS_O_WRKDIR of lustre)
        fileName oldPath_;

        //new working directory
        fileName newPath_;
   

private:

    // Private member functions
// Private Member Functions
        void debugPathOut (const string);
    
    
        //- Disallow default bitwise copy construct
        startTarCase(const startTarCase&);

        //- Disallow default bitwise assignment
        void operator=(const startTarCase&);


public:

    //- Runtime type information
    TypeName("startTarCase");


    // Constructors

        //- Construct from Time and dictionary
        startTarCase
        (
        
                argList&
               
        );


    //- Destructor
    virtual ~startTarCase();


    // Member Functions
    
        //- Read the system calls
        bool readControlDict();

        //- Execute the "executeCalls" at each time-step
        bool moveCase();
        
        // Access
        //- Return old run old case path
        const fileName returnOldPath();
        
        //- Search a given directory for valid time directories
            static instantList findTarTimes
            (
                const fileName& tarFile,
                const word& constantName = "constant"
            );
        
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
