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

Description
    Searches the current tar case directory for valid times
    and sets the time list to these.
    //This is done if a times File does not exist.

\*---------------------------------------------------------------------------*/

#include "startTarCase.H"
#include "OSspecific.H"
#include "IStringStream.H"
#include "archive.h"
#include "archive_entry.h"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

Foam::instantList Foam::startTarCase::findTarTimes
(
        const fileName& tarFile,
        const word& constantName
        )
{
  if (debug) {
      InfoIn(
              "Foam::startTarCase::findTarTimes"
              ) << "Finding times in directory " << tarFile << endl;
  }

  //fileNameList dirEntries();
  DynamicList<fileName> dirEntries;
  // Read tar entries into a list
  const char *cstr = tarFile.c_str();
  struct archive *a;
  struct archive_entry *entry;
  a = archive_read_new();
  archive_read_support_filter_all(a);
  archive_read_support_format_all(a);
  int r;
  r = archive_read_open_filename(a, cstr, 10240);
  if (r != ARCHIVE_OK) {
      FatalErrorIn("findTarTimes")
              << "tar archive " << tarFile
              << "not ok."
              << nl << nl
              << exit(FatalError);
  }
  while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {

      mode_t archiveType = archive_entry_filetype(entry);
      if (archiveType == S_IFDIR) {//directory!

          //deleting last slash
          fileName tmp(archive_entry_pathname(entry));
          tmp.clean();
          wordList tmpComponents = tmp.components();

          if (tmpComponents.size() >= 2) {
              //if (tmpComponents[1] != constantName) {
                  if (debug) {
                      InfoIn (
                              "Foam::startTarCase::findTarTimes"
                              ) << "archive_entry_pathname " << tmpComponents[1] << endl;
                  }
                  dirEntries.append(tmpComponents[1]);
              //}
          }


      }

      archive_read_data_skip(a); // Note 2
  }
  r = archive_read_free(a); // Note 3
  if (r != ARCHIVE_OK) {
      FatalErrorIn("findTarTimes")
              << "tar archive " << tarFile
              << "could not be freed."
              << nl << nl
              << exit(FatalError);
  }
  // we have now all directories of tar-file


  instantList Times(dirEntries.size() + 1);
  label nTimes = 0;

  // Check for "constant"
  bool haveConstant = false;

  //forAll(dirEntries, i)

  forAll(dirEntries, i)
  {
    if (dirEntries[i] == constantName) {
        Times[nTimes].value() = 0;
        Times[nTimes].name() = dirEntries[i];
        nTimes++;
        haveConstant = true;
        break;
    }
  }

  // Read and parse all the entries in the directory

  forAll(dirEntries, i)
  {
    IStringStream timeStream(dirEntries[i]);
    token timeToken(timeStream);

    if (timeToken.isNumber() && timeStream.eof()) {
        Times[nTimes].value() = timeToken.number();
        Times[nTimes].name() = dirEntries[i];
        nTimes++;
    }
  }

  // Reset the length of the times list
  Times.setSize(nTimes);

  if (haveConstant) {
      if (nTimes > 2) {
          std::sort(&Times[1], Times.end(), instant::less());
      }
  }
  else if (nTimes > 1) {
      std::sort(&Times[0], Times.end(), instant::less());
  }

  return Times;
}

// ************************************************************************* //
