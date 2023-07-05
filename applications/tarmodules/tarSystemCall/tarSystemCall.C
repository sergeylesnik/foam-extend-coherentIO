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

\*---------------------------------------------------------------------------*/

#include "tarSystemCall.H"
#include "dictionary.H"
#include "objectRegistry.H"


//#include "dynamicCode.H"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam {
    defineTypeNameAndDebug(tarSystemCall, 0);
}

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::tarSystemCall::debugPathOut(const string out)
{

  Info << "--------------------------------" << endl;
  Info << "Debug " << out << endl;

  Info << endl << "path: " << db_.path() << endl;
  Info << "time.timePath: " << db_.time().timePath() << endl;
  Info << "rootPath: " << db_.rootPath() << endl;
  Info << "caseName: " << db_.caseName() << endl;
  Info << "instance: " << db_.instance() << endl;
  Info << "local: " << db_.local() << endl;
  Info << "name " << db_.name() << endl << endl << endl;

  Info << "--------------------------------" << endl;
}

// Return components following the IOobject requirements
//
// behaviour
//    input               IOobject(instance, local, name)
//    -----               ------
//    "foo"               ("", "", "foo")
//    "foo/bar"           ("foo", "", "bar")
//    "/XXX/bar"          ("/XXX", "", "bar")
//    "foo/bar/"          ERROR - no name
//    "foo/xxx/bar"       ("foo", "xxx", "bar")
//    "foo/xxx/yyy/bar"   ("foo", "xxx/yyy", "bar")

bool Foam::tarSystemCall::myfileNameComponents(
        const fileName& path,
        fileName& instance,
        fileName& local,
        word& name
        )
{
  instance.clear();
  local.clear();
  name.clear();

  // called with directory //gives error in foam-extend, not in 2.4.x
  if (isDir(path)) {
      WarningIn
              (
              "IOobject::fileNameComponents"
              "("
              "const fileName&, "
              "fileName&, "
              "fileName&, "
              "word&"
              ")"
              )
              << " called with directory: " << path << endl;

      //return false;//therefore it is commented out!
  }

  string::size_type first = path.find('/');

  if (first == string::npos) {
      // no '/' found - no instance or local

      // check afterwards
      name.string::operator=(path);
  }
  else {
      instance = path.substr(0, first);

      string::size_type last = path.rfind('/');
      if (last > first) {
          // with local
          local = path.substr(first + 1, last - first - 1);
      }

      // check afterwards
      name.string::operator=(path.substr(last + 1));
  }

  // check for valid (and stripped) name, regardless of the debug level
  if (name.empty() || string::stripInvalid<word>(name)) {
      WarningIn
              (
              "IOobject::fileNameComponents"
              "("
              "const fileName&, "
              "fileName&, "
              "fileName&, "
              "word&"
              ")"
              )
              << "has invalid word for name: \"" << name
              << "\"\nwhile processing path: " << path << endl;

      return false;
  }

  return true;
}
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::tarSystemCall::tarSystemCall
(
        const word& name,
        const objectRegistry& obr,
        const dictionary& dict,
        const bool
        )
:
name_(name),
db_(obr),
useRamDisk_(false),
pathToRamDisk_(),
pathToOriginalCaseFolder_(),
oldPath_(),
executeCalls_(),
endCalls_(),
writeCalls_(),
locked_(-1),
childpid_(-1),
childStatus_(-1),
finalWrite_(-1),
endSyncDataMaster_(),
writeSyncDataMaster_()
{
  read(dict);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::tarSystemCall::~tarSystemCall()
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::tarSystemCall::read(const dictionary& dict)
{

  useRamDisk_ = dict.lookupOrDefault("ramDiskUsage", false);
  tarAggregationCount_ = dict.lookupOrDefault("tarAggregationCount", Pstream::nProcs());

  if (useRamDisk_) {
      Info << "RamdiskUsage set to: true" << endl;
      deleteStartTimeFolders_ =
              dict.lookupOrDefault("deleteStartTimeFolders", false);
      restartData_ =
              dict.lookupOrDefault("restartData", false);

      onlyRestartAndDataInTar_ =
              dict.lookupOrDefault("onlyRestartAndDataInTar", false);

      if(dict.readIfPresent("pathToRamDisk", pathToRamDisk_)){
      Info << "pathToRamDisk " << pathToRamDisk_ << endl;
      }
      else{
          FatalErrorIn("tarSystemCall::read(const dictionary&)")
                      << "no pathToRamDisk  given" 
                      << ", should be set in dictionary"
                      << nl << nl
                      << exit(FatalError);
      }
      
      

      const dictionary& controlDict = db_.time().controlDict();
      label purgeWrite(-1);
      precision_ = -1;

      if (controlDict.readIfPresent("purgeWrite", purgeWrite)) {
          if (purgeWrite != 1) {
              //WarningInFunction 
              FatalErrorIn("tarSystemCall::read(const dictionary&)")
                      << "invalid value for purgeWrite " << purgeWrite
                      << ", shoud be == 1 for Ramdisk usage"
                      << nl << nl
                      << exit(FatalError);
          }
      }
      if (controlDict.readIfPresent("timePrecision", precision_)) {
          if (precision_ == -1) {
              //WarningInFunction 
              FatalErrorIn("tarSystemCall::read(const dictionary&)")
                      << "please set timePrecision value explicitly "
                      << nl << nl
                      << exit(FatalError);
          }
      }


      if (debug) {
          Foam::tarSystemCall::debugPathOut(string("input"));
      }

      if (pathToRamDisk_ == db_.rootPath()) {//we are already in ramdisk
          Info << "--------------------------------" << endl;
          Info << "We are already in ramdisk" << endl;
          Info << "Restarting from tar " << endl;
          Info << "--------------------------------" << endl;
          if (dict.readIfPresent("pathToOriginalCaseFolder", pathToOriginalCaseFolder_)) {
              //fine
          }
          else {
              //not set but we need it
              FatalErrorIn("tarSystemCall::read(const dictionary&)")
                      << "please set pathToOriginalCaseFolder "
                      << ", since you start try to start from tar case"
                      << nl << nl
                      << exit(FatalError);
          }
          Info << "pathToOriginalCaseFolder " << pathToOriginalCaseFolder_ << endl;
          //setting oldPath_:
          oldPath_ = pathToOriginalCaseFolder_;
          fileName caseName = db_.caseName();
          fileName caseFolder;
          fileName local;
          word proc;
          myfileNameComponents(
                  caseName,
                  caseFolder,
                  local,
                  proc
                  );
          newPath_ = pathToRamDisk_ / caseFolder;
      }

      else {//no restart run of tar file case, let's prepare and move data to ramdisk


          //label tmp = timeObject.purgeWrite_;
          fileName caseName = db_.caseName();
          fileName caseFolder;
          fileName local;
          word proc;
          myfileNameComponents(
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
              //Info << "instance: " << db_.instance() << endl;
              Info << "--------------------------------" << endl;
          }
          fileName pathToCaseFolder = db_.rootPath() / caseFolder;
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
          //Info << "ramDiskCase " << ramDiskCase << endl << endl;
          //Info << "ramDiskCase " << ramDiskCase << endl;
          std::ostringstream exeStream;
          //exeStream << "rm -rf " << ramDiskCase << ";mkdir -p " << absPathToRamDiskCase;
          exeStream << "rm -rf " << ramDiskCase;
          std::string executeString = exeStream.str();
          Foam::system(executeString);
          //start blocking (not nice but...)
          scalar tmp = 0;
          reduce(tmp, sumOp<scalar>());
          //end blocking
          //end deleting case folder in ramdisk in advance

          exeStream.str("");
          exeStream.clear();
          //exeStream << "mkdir -p " << absPathToRamDiskCase;
          exeStream << "rm -rf " << absPathToRamDiskCase
                  << " ;mkdir -p " << absPathToRamDiskCase;

          executeString = exeStream.str();

          if (debug) {
              Info << "3a--------------------------------" << endl;
              Info << "exeString " << executeString << endl;
              Info << "3a--------------------------------" << endl;
          }
          Foam::system(executeString);

          //copy my proc-data in ramDisk:
          exeStream.str("");
          exeStream.clear();

          exeStream << "rsync -q -a " << db_.path() << " " <<
                  absPathToRamDiskCase << "/";
          executeString = exeStream.str();
          if (debug) {
              Info << "3--------------------------------" << endl;
              Info << "db_.path() " << db_.path() << endl;
              Info << "exeString " << executeString << endl;
              Info << "3--------------------------------" << endl;
          }
          Foam::system(executeString);

          exeStream.str("");
          exeStream.clear();
          exeStream << "rsync -q -a " << db_.rootPath() << "/" << db_.caseName()
                  << "/constant " << absPathToRamDiskCase << "/";

          executeString = exeStream.str();
          if (debug) {
              Info << "4--------------------------------" << endl;
              Info << "exeString " << executeString << endl;
              Info << "4--------------------------------" << endl;
          }
          Foam::system(executeString);

        if (Pstream::master()) {
          //rsync -va/ln -s of constant except polyMeshData
          exeStream.str("");
          exeStream.clear();

          //exeStream << "rsync -a --exclude=polyMesh " << " " << pathToCaseFolder << "/constant " 
          //      << pathToRamDisk_ <<"/"<< caseFolder <<"/";

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

          //exeStream << "rsync -a " << " " << pathToCaseFolder << "/system " 
          //      << pathToRamDisk_ <<"/"<< caseFolder <<"/";
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

          //exeStream << "rsync -a " << " " << pathToCaseFolder << "/system " 
          //      << pathToRamDisk_ <<"/"<< caseFolder <<"/";
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

          //new pathes
          fileName newRootPath = pathToRamDisk_;
          setEnv("FOAM_CASE", newRootPath, true);
          setEnv("FOAM_CASENAME", caseFolder, true);

          //not nice, but...
          fileName & pathModifier(const_cast<fileName&> (db_.rootPath()));


          pathModifier = pathToRamDisk_;
          //##############################################################################
          if (debug) {
              Foam::tarSystemCall::debugPathOut(string("output"));
          }


          if (Pstream::master()) {
              exeStream.str("");
              exeStream.clear();
              exeStream << "echo $FOAM_CASE ";

              executeString = exeStream.str();
              if (debug) {
                  //Info << "exeString " << executeString << endl;
              }
              Foam::system(executeString);
              //--------------------
              exeStream.str("");
              exeStream.clear();
              exeStream << "echo $FOAM_CASENAME ";

              executeString = exeStream.str();
              if (debug) {
                  //Info << "exeString " << executeString << endl;
              }
              Foam::system(executeString);


          }

      }


      dict.readIfPresent("executeCalls", executeCalls_);
      dict.readIfPresent("endCalls", endCalls_);
      dict.readIfPresent("writeCalls", writeCalls_);
      dict.readIfPresent("endSyncDataMaster", endSyncDataMaster_);
      dict.readIfPresent("writeSyncDataMaster", writeSyncDataMaster_);

      if (executeCalls_.empty() && endCalls_.empty() && writeCalls_.empty()) {
         
          WarningIn("Foam::tarSystemCall::read(const dictionary&)") 
            << "no executeCalls, endCalls or writeCalls defined."
            << endl;
      }
      /*else if (!Foam::dynamicCode::allowSystemOperations) {
          FatalErrorInFunction
                  << "Executing user-supplied system calls is not enabled by "
                  << "default because of " << nl
                  << "security issues.  If you trust the case you can enable this "
                  << "facility by " << nl
                  << "adding to the InfoSwitches setting in the system controlDict:"
                  << nl << nl
                  << "    allowSystemOperations 1" << nl << nl
                  << "The system controlDict is either" << nl << nl
                  << "    ~/.OpenFOAM/$WM_PROJECT_VERSION/controlDict" << nl << nl
                  << "or" << nl << nl
                  << "    $WM_PROJECT_DIR/etc/controlDict" << nl << nl
                  << exit(FatalError);
      }*/
  }
  else {
      Info << "RamdiskUsage set to: false" << endl;
  }

}

void Foam::tarSystemCall::execute()
{

  forAll(executeCalls_, callI)
  {
    Foam::system(executeCalls_[callI]);
  }
}

void Foam::tarSystemCall::end()
{
  //start blocking (not nice but...)
  scalar tmp = 0;
  reduce(tmp, sumOp<scalar>());
  //end blocking
  
  scalar startTime = db_.time().startTime().value();

  scalar currentTime = db_.time().timeOutputValue();
  instant icurrentTime = db_.time().findClosestTime(currentTime);
  word wcurrentTime = icurrentTime.name();

  {//sync last tar files
      label myProcId = Pstream::myProcNo();
      std::ostringstream exeStream;
      std::string executeString;
      if (onlyRestartAndDataInTar_) {
          //Info << "onlyRestartAndDataInTar_  end() TRUE" << endl;
        //Pout << "My Proc myProcId "<< myProcId <<" modulo with tarAggregationCount_" << myProcId % tarAggregationCount_ << endl;
        if ((myProcId % tarAggregationCount_) == 0)
        {
          for (label procId = myProcId; procId < (myProcId + tarAggregationCount_); ++procId)
          {
            //Pout << "I am proc "<< myProcId << " and I am taking care of processor"<< procId<<endl;
            exeStream.str("");
            exeStream.clear();
            exeStream << "rm -rf " << oldPath_ << "/processor" << procId << ".tar ;"
                      << "cd " << newPath_ << "; tar -uf "
                      << oldPath_ << "/processor" << procId << ".tar "
                      << "processor" << procId << " --exclude=processor"
                      << procId << "/" << startTime << "  ; cd -;"
                      << "rm -rf " << oldPath_ << "/processor" << procId << "/constant ;";
            executeString = exeStream.str();
            //Pout << "exec: "<< executeString << endl;
            //Pout << "exeStream end() " << executeString << endl;
            //Pout << executeString << endl;
            //sleep(2); //test

            forAll(endCalls_, callI)
            {
              Foam::system(executeString);
            }
          }
        }
        //start blocking (not nice but...)
        //scalar tmp = 0;
        //reduce(tmp, sumOp<scalar>());
        //end blocking
      }
      else {
        //Pout << "My Proc myProcId "<< myProcId <<" modulo with tarAggregationCount_" << myProcId % tarAggregationCount_ << endl;
        if ((myProcId % tarAggregationCount_) == 0)
        {
          for (label procId = myProcId; procId < (myProcId + tarAggregationCount_); ++procId)
          {
            //Pout << "I am proc "<< myProcId << " and I am taking care of processor"<< procId<<endl;
            //correct
            exeStream.str("");
            exeStream.clear();
            exeStream << "cd " << newPath_ << " && "
                      << "cat processor" << procId << "/" << wcurrentTime << "/uniform/profilingInfo >> " 
                      << oldPath_ << "/profilingInfo_p" << myProcId << "_t" << wcurrentTime << " && "
                      << "tar -uvf " << oldPath_ << "/processor" << procId << ".tar " << "processor" << procId << "/* && "
                      << "cd - &>/dev/null";
            executeString = exeStream.str();
            //Pout << "exec: "<< executeString << endl;
            //Pout << executeString << endl;

            forAll(endCalls_, callI)
            {
              Foam::system(executeString);
            }
          }
        }

      }
      //start blocking (not nice but...)
      //scalar tmp = 0;
      //reduce(tmp, sumOp<scalar>());
      //end blocking
  }


  if (deleteStartTimeFolders_) {
      //Pout << "startTime " << startTime << endl;
      //backup for restart, optional, since it is in procxxx.tar
      label myProcId = Pstream::myProcNo();
      std::ostringstream exeStream;


      //correct
      exeStream << "rm -rf " << newPath_ << "/"
              << "processor" << myProcId << "/"
              << startTime;


      /*  exeStream     << "rm -rf " << newPath_ << "/"
             << "processor" << myProcId << "/"
             << startTime << " ; rsync -a --delete " << newPath_ << "/"
             << "processor" << myProcId << " "
             << oldPath_;*/

      std::string executeString = exeStream.str();
      //Pout << executeString << endl;

      forAll(endCalls_, callI)
      {
        Foam::system(executeString);
      }

      if (!restartData_) {
          std::ostringstream exeStream2;
          exeStream2 << "rm -rf " << oldPath_ << "/"
                  << "processor" << myProcId << "/"
                  << startTime;
          std::string executeString2 = exeStream2.str();

          forAll(endCalls_, callI)
          {
            Foam::system(executeString2);
          }
      }

  }

  if (restartData_) {
      //backup for restart, optional, since it is in procxxx.tar
      label myProcId = Pstream::myProcNo();
      std::ostringstream exeStream;

      //correct
      exeStream << "rsync -q -a --delete " << newPath_ << "/"
              << "processor" << myProcId << " "
              << oldPath_;

      std::string executeString = exeStream.str();
      //Pout << executeString << endl;

      forAll(endCalls_, callI)
      {
        Foam::system(executeString);
      }
  }

if ( (deleteStartTimeFolders_) && !(restartData_) && !(onlyRestartAndDataInTar_)){
    label myProcId = Pstream::myProcNo();
      std::ostringstream exeStream;
      std::string executeString;
      exeStream << "rm -rf " << oldPath_ << "/processor" << myProcId << "/constant ;";
          executeString = exeStream.str();
          forAll(endCalls_, callI)
          {
            Foam::system(executeString);
          }
}

  if (Pstream::master()) {

      forAll(endSyncDataMaster_, callI)
      {
        std::ostringstream exeStream;
        exeStream << "rsync -q -a " << newPath_ << "/"
                << endSyncDataMaster_[callI]
                << " " << oldPath_ << "/";
        std::string executeString = exeStream.str();
        Foam::system(executeString);
      }
  }


  /*
  forAll(endCalls_, callI) {
      Foam::system(endCalls_[callI]);
  }*/

  //start blocking (not nice but...)
  reduce(tmp, sumOp<scalar>());
  //end blocking
}

//timeSet not used:
void Foam::tarSystemCall::timeSet()
{
  // Do nothing
  //Info << "Index " << db_.time().timeIndex() << endl;

  if (db_.time().outputTime()) {
      //Info << "outputNow " << db_.time().outputTime() << endl;
      //wait for child
      pid_t tmpchildpid;


      do {
          tmpchildpid = waitpid(childpid_, &childStatus_, WNOHANG);
          if (tmpchildpid != childpid_) {//still locked
              //locked
          }
          else {
              //not locked

          }
      }
      while (tmpchildpid != childpid_);
      //Pout << "tmpchildpid " << tmpchildpid << endl;
      //Pout << "childpid_ " << childpid_ << endl;
      locked_ = 0;
  }
}

void Foam::tarSystemCall::write()
{
  //start blocking (not nice but...)
  scalar tmp = 0;
  reduce(tmp, sumOp<scalar>());
  //end blocking

  scalar startTime = db_.time().startTime().value();

  scalar currentTime = db_.time().timeOutputValue();
  instant icurrentTime = db_.time().findClosestTime(currentTime);
  instant iendTime(db_.time().endTime().value());
  word wcurrentTime = icurrentTime.name();
  word wendTime = iendTime.name();
  

  if (wcurrentTime.compare(wendTime) != 0) {
      //Info << "write: current!=end" << endl;
      /* Disabled forking because of problems across several nodes
      childpid_ = fork();
      if (childpid_ != 0) {//parent
          locked_ = 1;
          Pout << "I am parent " << childpid_ << " in myProcNo " << Pstream::myProcNo() << ". By the way, wendTime is "<< wendTime << endl;
          //wait(NULL);
          //sleep(10);
      }
      else if (childpid_ == -1)
      { 
          Pout << "SOMETHING JUST GOT OUT OF CONTROL " << childpid_ << " in myProcNo " << Pstream::myProcNo() << endl;
      }
      else {//first child
      */
          label myProcId = Pstream::myProcNo();
          std::ostringstream exeStream;
          std::string executeString;

          if (onlyRestartAndDataInTar_) {
              //Info << "onlyRestartAndDataInTar_  write() TRUE" << endl;
              //scalar currentTime = db_.time().timeOutputValue();
            if ((myProcId % tarAggregationCount_) == 0)
            {
              for (label procId = myProcId; procId < (myProcId + tarAggregationCount_); ++procId)
              {
                exeStream.str("");
                exeStream.clear();
                exeStream << "rm -rf " << oldPath_ << "/processor" << procId << ".tar ;"
                        << "cd " << newPath_ << "; tar -uf "
                        << oldPath_ << "/processor" << procId << ".tar "
                        << "processor" << procId << " --exclude=processor"
                        << procId << "/" << startTime << "  ; cd -;"
                        << "rm -rf " << oldPath_ << "/processor" << procId << "/constant ;";
                executeString = exeStream.str();
                //Pout << "exec: "<< executeString << endl;
                //sleep(2); //test

                forAll(writeCalls_, callI)
                {
                  //Pout << "I am proc "<< myProcId << " and I am taking care of processor"<< procId<<endl;
                  Foam::system(executeString);
                }
              }
            }
            //int exit_code;
            //sleep(10);
            //std::exit(EXIT_FAILURE);
            //Pout << "I am child " << childpid_ << " from myProcNo " << Pstream::myProcNo() << " and will exit now with code " << endl;
          }
          else {

            if ((myProcId % tarAggregationCount_) == 0)
            {
              for (label procId = myProcId; procId < (myProcId + tarAggregationCount_); ++procId)
              {
                exeStream.str("");
                exeStream.clear();
                exeStream << "cd " << newPath_ << " && "
                          << "cat processor" << procId << "/" << wcurrentTime << "/uniform/profilingInfo >> " 
                          << oldPath_ << "/profilingInfo_p" << myProcId << "_t" << wcurrentTime << " && "
                          << "tar -uvf " << oldPath_ << "/processor" << procId << ".tar " << "processor" << procId << "/* && "
                          << "cd - &>/dev/null";
                executeString = exeStream.str();
                //Pout << "exec: "<< executeString << endl;
                //sleep(2); //test

                forAll(writeCalls_, callI)
                {
                  //Pout << "This is proc "<< myProcId << " and I am taking care of processor"<< procId<<endl;
                  Foam::system(executeString);
                }
              }
            }
            //int exit_code;
            //sleep(10);
            //std::exit(EXIT_FAILURE);
            //Pout << "I am child " << childpid_ << " from myProcNo " << Pstream::myProcNo() << " and will exit now with code " << endl;
          }

          //Pout << "I am child " << childpid_ << " from myProcNo " << Pstream::myProcNo() << " and will exit now." << endl;
          //std::exit(EXIT_SUCCESS);
          //tgkill(childpid_,SIGTERM);
      //}
  }
  else {
      //identical, we do only a final write in end()
      //Info << "write: current==end" << endl;

  }
  
 

  //postsync start
  if (Pstream::master()) {

      forAll(writeSyncDataMaster_, callI)
      {
        std::ostringstream exeStream;
        exeStream << "rsync -q -a " << newPath_ << "/"
                << writeSyncDataMaster_[callI]
                << " " << oldPath_ << "/";
        std::string executeString = exeStream.str();
        Foam::system(executeString);
      }
  }
  //postsync end
  
  //start blocking (not nice but...)
  reduce(tmp, sumOp<scalar>());
  //end blocking
}






// ************************************************************************* //
