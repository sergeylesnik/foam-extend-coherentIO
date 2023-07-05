/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2013 OpenFOAM Foundation
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

Class
    Foam::tarSystemCall

Group
    grpFunctionObjects

Description
    This function object executes system calls, entered in the form of a
    string lists.  Calls can be made at the following points in the
    calculation:
    - every time step
    - every output time
    - end of the calculation

    Example of function object specification:
    \verbatim
    tarSystemCall1
    {
        type        tarSystemCall;
        functionObjectLibs ("libtarSystemCall.so");
        ...
        executeCalls
        (
            "echo execute"
        );
        writeCalls
        (
            "echo \*\*\* writing data \*\*\*"
        );
        endCalls
        (
            "echo \*\*\* writing .bashrc \*\*\*"
            "cat ~/.bashrc"
            "echo \*\*\* done \*\*\*"
        );
    }
    \endverbatim

    \heading Function object usage
    \table
        Property     | Description             | Required    | Default value
        type         | type name: tarSystemCall   | yes         |
        executeCalls | list of calls on execute | yes        |
        writeCalls   | list of calls on write  | yes         |
        endCalls     | list of calls on end    | yes         |
    \endtable

Note
    Since this function object executes system calls, there is a potential
    security risk.  In order to use the \c tarSystemCall function object, the
    \c allowSystemOperations must be set to '1'; otherwise, system calls will
    not be allowed.

SeeAlso
    Foam::functionObject
    Foam::OutputFilterFunctionObject

SourceFiles
    tarSystemCall.C
    IOtarSystemCall.H

\*---------------------------------------------------------------------------*/

#ifndef tarSystemCall_H
#define tarSystemCall_H

#include "stringList.H"
#include "IOobject.H"
#include "foamTime.H"
#include "Pstream.H"
#include "pointFieldFwd.H"
#include "PstreamReduceOps.H"

#include <unistd.h>
#include <sys/wait.h> 
#include <time.h>
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam {

    // Forward declaration of classes
    class objectRegistry;
    class dictionary;
    class polyMesh;
    class mapPolyMesh;
    class fvMesh;

    /*---------------------------------------------------------------------------*\
                             Class tarSystemCall Declaration
    \*---------------------------------------------------------------------------*/

    class tarSystemCall {
    protected:

        // Private data
        //- Name of this set of system calls
        word name_;

        //- objectRegistry reference
        const objectRegistry& db_;

        //use of ramdisk true/false:
        bool useRamDisk_;

        // Aggregate tar calls on process if (Pstream::myProcNo() % tarAggregationCount_ == 0)
        label tarAggregationCount_;

        //new working directory
        string pathToRamDisk_;

        //given path to original case folder for restart from tar files
        string pathToOriginalCaseFolder_;

        //old working directory (e.g. PBS_O_WRKDIR of lustre)
        fileName oldPath_;

        //new working directory
        fileName newPath_;

        //- List of calls to execute - every step
        stringList executeCalls_;

        //- List of calls to execute when exiting the time-loop
        stringList endCalls_;

        //- List of calls to execute - write steps
        stringList writeCalls_;

        //fork guard
        label locked_;

        //pid
        pid_t childpid_;

        //child status
        int childStatus_;

        //final write
        label finalWrite_;

        //precision
        int precision_;

    //delete startTime folders in original processorXXX/ folder
        bool deleteStartTimeFolders_;
        
        //write restart data in original processorXXX/ folder
        bool restartData_;
        
        //only last time step in tar file
        bool onlyRestartAndDataInTar_;

        //- List of data to recover other data e.g. postProcessing folder
        stringList endSyncDataMaster_;
        
        //- List of data to recover other data e.g. postProcessing folder
        stringList writeSyncDataMaster_;

        // Private Member Functions
        void debugPathOut (const string);

        //myfileNameComponents, stolen from IOobject.C from 2.4.x
        bool myfileNameComponents (
                const fileName& path,
                fileName& instance,
                fileName& local,
                word& name
                );

        //- Disallow default bitwise copy construct
        tarSystemCall (const tarSystemCall&);

        //- Disallow default bitwise assignment
        void operator= (const tarSystemCall&);


    public:

        //- Runtime type information
        TypeName ("tarSystemCall");


        // Constructors

        //- Construct for given objectRegistry and dictionary.
        //  Allow the possibility to load fields from files
        tarSystemCall
        (
                const word& name,
                const objectRegistry& unused,
                const dictionary&,
                const bool loadFromFilesUnused = false
                );


        //- Destructor
        virtual ~tarSystemCall ();


        // Member Functions

        //- Return name of the system call set

        virtual const word& name () const
        {
            return name_;
        }

        //- Read the system calls
        virtual void read (const dictionary&);

        //- Execute the "executeCalls" at each time-step
        virtual void execute ();

        //- Execute the "endCalls" at the final time-loop
        virtual void end ();

        //- Called when time was set at the end of the Time::operator++
        virtual void timeSet ();

        //- Write, execute the "writeCalls"
        virtual void write ();

        //- Update for changes of mesh

        virtual void updateMesh (const mapPolyMesh&)
        {
        }

        //- Update for changes of mesh

        virtual void movePoints (const pointField&)
        {
        }
    };


    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
