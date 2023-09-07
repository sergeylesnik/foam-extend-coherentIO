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
    Foam::OFCstream

Description
    Output to file stream for coherent mesh and fields.

SourceFiles
    OFCstream.C

\*---------------------------------------------------------------------------*/

#ifndef OFCstream_H
#define OFCstream_H

#include "OFstream.H"
#include "CoherentMesh.H"
#include "dictionary.H"
#include "fieldDataEntry.H"
#include "uListProxyBase.H"
#include "IOstream.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class OFCstreamBase Declaration
\*---------------------------------------------------------------------------*/

class OFCstreamBase
:
    public OFstream
{

protected:

    // Protected data

        fileName pathname_;
        CoherentMesh& coherentMesh_;

        //- Dictionary holding both entries for the ASCII file and pointer,
        //  size data for the BLOBs.
        dictionary dict_;

        //- Pointer to the (sub-)dictionary the ofstream currently writes to
        dictionary* currentSubDictPtr_;

        //- The last keyword seen on the stream
        keyType currentKeyword_;

        //- Entry counter used for the formatting entries
        label currentEntryI_;

        //- The last compound token seen on the stream
        word currentCompoundTokenName_;


    // Protected member functions

        //- Gather all objects of type fieldDataEntry from a dictionary
        virtual void gatherFieldDataEntries
        (
            dictionary&,
            DynamicList<fieldDataEntry*>&
        );

        //- Remove processor patches from the dictionary
        virtual void removeProcPatchesFromDict();

public:

    // Declare name of the class and its debug switch
    // TypeName("OFCstreamBase");


    // Constructors

        //- Default construct
        OFCstreamBase
        (
            const fileName& pathname,
            const objectRegistry& registry,
            ios_base::openmode mode = ios_base::out|ios_base::trunc,
            IOstreamOption streamOpt = IOstreamOption()
        );

        //- Construct from pathname
        OFCstreamBase
        (
            const fileName& pathname,
            const objectRegistry& registry,
            ios_base::openmode mode,
            streamFormat format = ASCII,
            versionNumber version = currentVersion,
            compressionType compression = UNCOMPRESSED
        )
        :
            OFCstreamBase
            (
                pathname,
                registry,
                mode,
                IOstreamOption(format, version, compression)
            )
        {}


    //- Destructor
    virtual ~OFCstreamBase();


    // Member functions

        //- Get variable id without "processor.*" if writing a global field
        virtual string getBlockId() const;

        //- Get variable id without "processor.*"
        string getGlobalId() const;

        virtual Ostream& writeKeyword(const keyType&);
        virtual Ostream& write(const token&);
        virtual Ostream& write(const word&);
        virtual word incrBlock(const word);
        virtual void decrBlock();
        virtual Ostream& parwrite(std::unique_ptr<uListProxyBase>);

        //- Put current stream buffer to a dictionary as an entry
        void moveStreamBufferToDict();

        //- Write the dictionary with correct formatting
        void writeDict(Ostream& os, const dictionary& dict, bool subDict) const;

        //- Write data with the specified engine and dictionary by master
        void writeGlobalGeometricField();
};

template<class Type, template<class> class PatchField, class GeoMesh>
class OFCstream
:
public OFCstreamBase
{

public:

    // Constructors

        //- Default construct
        OFCstream
        (
            const fileName& pathname,
            const objectRegistry& registry,
            ios_base::openmode mode = ios_base::out|ios_base::trunc,
            IOstreamOption streamOpt = IOstreamOption()
        )
        :
            OFCstreamBase(pathname, registry, mode, streamOpt)
        {}

        //- Construct given path and registry
        OFCstream
        (
            const fileName& pathname,
            const objectRegistry& registry,
            ios_base::openmode mode,
            IOstream::streamFormat format = IOstream::ASCII,
            IOstream::versionNumber version = IOstream::currentVersion,
            IOstream::compressionType compression = IOstream::UNCOMPRESSED
        )
        :
            OFCstream
            (
                pathname,
                registry,
                mode,
                IOstreamOption(format, version, compression)
            )
        {}


    //- Destructor
    virtual ~OFCstream()
    {
        this->removeProcPatchesFromDict();
        this->writeGlobalGeometricField();
    }
};

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
