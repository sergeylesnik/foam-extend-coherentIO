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

#include "startTarCase.H"
#include "foamTime.H"
//#include "dynamicCode.H"
//#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //
namespace Foam {
    defineTypeNameAndDebug(startTarCase, 0);

}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::startTarCase::debugPathOut(const string out)
{

  Info << "--------------------------------" << endl;
  Info << "Debug " << out << endl;

  /*  Info << endl << "path: " << db_.path() << endl;
    Info << "time.timePath: " << db_.time().timePath() << endl;
    Info << "rootPath: " << db_.rootPath() << endl;
    Info << "caseName: " << db_.caseName() << endl;
    Info << "instance: " << db_.instance() << endl;
    Info << "local: " << db_.local() << endl;
    Info << "name " << db_.name() << endl << endl << endl;
   */
  Info << "--------------------------------" << endl;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::startTarCase::startTarCase
(
        argList& args
        )
:
args_(args)
{

  //Pout << "startTarCase: I'm alive" << endl;

  readControlDict();
  if (useRamDisk_) {
      moveCase();
  }
  else {
      //nothing to shift
  }

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::startTarCase::~startTarCase()
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::startTarCase::readControlDict()
{

  Foam::Time runTime(
          Foam::Time::controlDictName,
          args_.rootPath(),
          args_.caseName()
          );

  const dictionary & controlDict(runTime.controlDict());

  const dictionary & functionSubDict(controlDict.subDict("functions"));
  const dictionary & mysystemCallDict(functionSubDict.subDict("tarSystemCall"));

  useRamDisk_ = mysystemCallDict.lookupOrDefault("ramDiskUsage", false);

  if (useRamDisk_) {

      

      if (mysystemCallDict.readIfPresent("pathToRamDisk", pathToRamDisk_)) {
          //Info << "pathToRamDisk " << pathToRamDisk_ << endl;
      }
      else {
          FatalErrorIn("startTarCase::read(const dictionary&)")
                  << "no pathToRamDisk  given"
                  << ", should be set in dictionary"
                  << nl << nl
                  << exit(FatalError);
      }

      Info << "[startTarCase] pathToRamDisk " << pathToRamDisk_ << endl;
      
      
      onlyRestartAndDataInTar_ = mysystemCallDict.lookupOrDefault("onlyRestartAndDataInTar_", false);

      label purgeWrite(-1);
      if (controlDict.readIfPresent("purgeWrite", purgeWrite)) {
          if (purgeWrite != 1) {
              //WarningInFunction 
              FatalErrorIn("startTarCase::read()")
                      << "invalid value for purgeWrite " << purgeWrite
                      << ", shoud be == 1 for Ramdisk usage"
                      << nl << nl
                      << exit(FatalError);
          }
      }

      fileName caseName = args_.caseName();
      fileName caseFolder;
      fileName local;
      word proc;
      Foam::IOobject::fileNameComponents
              (
              caseName,
              caseFolder,
              local,
              proc
              );

      //get startTime data of tar-processors in ramdisk
      std::ostringstream exeStream;
      exeStream.str("");
      exeStream.clear();
      exeStream << proc << ".tar";
      fileName tarName = exeStream.str();
      const word constantName = runTime.constant();

      const word startFrom = controlDict.lookupOrDefault<word>
              (
              "startFrom",
              "latestTime"
              );
      if (startFrom == "startTime") {
          controlDict.lookup("startTime") >> startTime_;
      }
      else {
          // Search directory for valid time directories
          //instantList timeDirs = findTimes(path(), constant());
          instantList timeDirectories = findTarTimes(tarName, constantName);
          if (debug) {
              Pout << "timeDirectories " << timeDirectories << endl;
          }
          if (startFrom == "firstTime") {
              if (timeDirectories.size()) {
                  if (timeDirectories[0].name() == constantName && timeDirectories.size() >= 2) {
                      startTime_ = timeDirectories[1].value();
                  }
                  else {
                      startTime_ = timeDirectories[0].value();
                  }
              }
          }
          else if (startFrom == "latestTime") {
              if (timeDirectories.size()) {
                  startTime_ = timeDirectories.last().value();
              }
          }
          else {
              FatalIOErrorIn("Foam::startTarCase::readControlDict with dictionary: ", controlDict)
                      << "expected startTime, firstTime or latestTime"
                      << " found '" << startFrom << "'"
                      << exit(FatalIOError);
          }

      }
      if (debug) {
          Pout << "startTime " << startTime_ << endl;
      }
  }
  else {
      //nothing to do
  }


  return true;
}

bool Foam::startTarCase::moveCase()
{
  fileName test = args_.caseName();
  if (debug) {
      Pout << "test: " << test << endl;
  }
  fileName caseName = args_.caseName();
  fileName caseFolder;
  fileName local;
  word proc;
  Foam::IOobject::fileNameComponents
          (
          caseName,
          caseFolder,
          local,
          proc
          );

  if (debug) {
      Info << "--------------------------------" << endl;
      Info << "caseName " << caseName << endl;
      Info << "caseFolder " << caseFolder << endl;
      Info << "proc " << proc << endl;

      Info << "--------------------------------" << endl;
  }

  fileName pathToCaseFolder = args_.rootPath() / caseFolder;
  oldPath_ = pathToCaseFolder;

  newPath_ = pathToRamDisk_ / caseFolder;
  if (debug) {
      Info << "2--------------------------------" << endl;
      Info << "oldPath_" << oldPath_ << endl;
      Info << "newPath_" << newPath_ << endl;
      Info << "pathToCaseFolder " << pathToCaseFolder << endl;
      Info << "2--------------------------------" << endl;
  }


  //creating procCases in ramDiskPath
  fileName absPathToRamDiskCase = pathToRamDisk_ / caseName;
  if (debug) {
      Info << "absPathToRamDiskCase " << absPathToRamDiskCase << endl << endl;
  }
  //delete case folder in ramdisk in advance

  fileName ramDiskCase = pathToRamDisk_ / caseFolder;
  if (debug) {
      Info << "ramDiskCase " << ramDiskCase << endl << endl;
  }
  //Info << "ramDiskCase " << ramDiskCase << endl;
  std::ostringstream exeStream;
  std::string executeString;
  //if (Pstream::master) {
  //exeStream << "rm -rf " << ramDiskCase << ";mkdir -p " << absPathToRamDiskCase;
  exeStream << "rm -rf " << ramDiskCase;
  executeString = exeStream.str();
  Foam::system(executeString);
  //}
  //start blocking (not nice but...)
  scalar tmp = 0;
  reduce(tmp, sumOp<scalar>());
  //end blocking
  //end deleting case folder in ramdisk in advance



  exeStream.str("");
  exeStream.clear();
  //exeStream << "mkdir -p " << absPathToRamDiskCase;
  exeStream << "rm -rf " << absPathToRamDiskCase
          << " ;mkdir -p " << ramDiskCase;
  //<< " ;mkdir -p " << absPathToRamDiskCase;
  //creating .../processorXXX folders in ramdisk

  executeString = exeStream.str();

  if (debug) {
      Info << "3a--------------------------------" << endl;
      Info << "exeString " << executeString << endl;
      Info << "3a--------------------------------" << endl;
  }
  Foam::system(executeString);




  exeStream.str("");
  exeStream.clear();
  //if (onlyRestartAndDataInTar_) {
  exeStream << "cd " << newPath_ << "; tar xf " << oldPath_ << "/" << proc
          << ".tar " << proc << "/" << startTime_ << " ; tar xf " << oldPath_ << "/" << proc
          << ".tar " << proc << "/constant" << " ;";
  executeString = exeStream.str();
  //}
  /*else {//not needed anymore since we now startTime now
      //get latestTime to start:

      exeStream << "rsync -q -va " << oldPath_ << "/" << proc << ".tar " <<
              newPath_ << "/ ;cd " << newPath_
              << " ; tar xf " << proc << ".tar ;rm " << proc << ".tar ";
      executeString = exeStream.str();
  }*/

  if (debug) {
      Info << "3tar--------------------------------" << endl;
      Info << "exeString " << executeString << endl;
      Info << "3tar--------------------------------" << endl;
  }
  Foam::system(executeString);

  if (Pstream::master()) {
  //rsync -va/ln -s of constant except polyMeshData
  exeStream.str("");
  exeStream.clear();

  //pathToCaseFolder orig case folder on filesystem
  //caseFolder name of case folder

  exeStream << "ln -s " << " " << pathToCaseFolder << "/constant "
          << pathToRamDisk_ << "/" << caseFolder << "/";

  executeString = exeStream.str();
  if (debug) {
      Info << "5--------------------------------" << endl;
      Info << "exeString " << executeString << endl;
      Info << "5--------------------------------" << endl;
  }
  Foam::system(executeString);



  //rsync -va/ln -s of system except polyMeshData
  exeStream.str("");
  exeStream.clear();

  exeStream << "ln -s " << " " << pathToCaseFolder << "/system "
          << pathToRamDisk_ << "/" << caseFolder << "/";

  executeString = exeStream.str();
  if (debug) {
      Info << "6--------------------------------" << endl;
      Info << "exeString " << executeString << endl;
      Info << "6--------------------------------" << endl;
  }
  Foam::system(executeString);


  //rsync -va/ln -s of config.xml
  //exeStream.str("");
  //exeStream.clear();

  //exeStream << "ln -s " << " " << pathToCaseFolder << "/config.xml "
          //<< pathToRamDisk_ << "/" << caseFolder << "/";

  //executeString = exeStream.str();
  //if (debug) {
      //Info << "6--------------------------------" << endl;
      //Info << "exeString " << executeString << endl;
      //Info << "6--------------------------------" << endl;
  //}
  //Foam::system(executeString);


  }
  //start blocking (not nice but...)
  tmp = 0;
  reduce(tmp, sumOp<scalar>());
  //end blocking
  //end deleting case folder in ramdisk in advance



  //new pathes
  fileName newRootPath = pathToRamDisk_;
  setEnv("FOAM_CASE", newRootPath, true);
  setEnv("FOAM_CASENAME", caseFolder, true);


  //not nice, but...
  fileName & pathModifier(const_cast<fileName&> (args_.rootPath()));
  if (debug) {
      Pout << "BEFORE args_.rootPath: " << args_.rootPath() << endl;
  }
  pathModifier = pathToRamDisk_;
  if (debug) {
      Pout << "AFTER args_.rootPath: " << args_.rootPath() << endl;
  }



  //##############################################################################
  if (Pstream::master()) {
      exeStream.str("");
      exeStream.clear();
      exeStream << "echo $FOAM_CASE ";

      executeString = exeStream.str();
      if (debug) {
          Info << "FOAM_CASE: " << endl;
          Info << "exeString " << executeString << endl;
      }
      //Foam::system(executeString);
      //--------------------
      exeStream.str("");
      exeStream.clear();
      exeStream << "echo $FOAM_CASENAME ";

      executeString = exeStream.str();
      if (debug) {
          Info << "FOAM_CASENAME: " << endl;
          Info << "exeString " << executeString << endl;
      }
      //Foam::system(executeString);


  }

  return true;
}

const Foam::fileName Foam::startTarCase::returnOldPath()
{
  return oldPath_;
}
// ************************************************************************* //
