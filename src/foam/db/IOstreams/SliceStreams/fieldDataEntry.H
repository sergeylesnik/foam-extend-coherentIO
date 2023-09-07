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
    Foam::fieldDataEntry

Description
    Entry class to store field relevant data in a dictionary

SourceFiles
    fieldDataEntry.C

\*---------------------------------------------------------------------------*/

#ifndef fieldDataEntry_H
#define fieldDataEntry_H

#include "dictionary.H"
#include "fieldTag.H"
#include "uListProxyBase.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                          Class fieldDataEntry Declaration
\*---------------------------------------------------------------------------*/

class fieldDataEntry
:
    public entry
{

    // Private data

        //- Entry name
        fileName name_;

        //- Name of the compound token associated with the data, i.e. List<...>
        const word compoundTokenName_;

        const autoPtr<uListProxyBase> uListProxyPtr_;

        //- Number of elements summed over all ranks
        label nGlobalElems_;

        //- Field tag needed for global communication
        fieldTag tag_;


public:

    // Constructors

        fieldDataEntry
        (
            const keyType& keyword,
            const word& compoundTokenName,
            uListProxyBase* uListProxyPtr
        );

        //- Construct on freestore as copy with reference to the
        //  dictionary the copy belongs to
        virtual autoPtr<entry> clone(const dictionary&) const
        {
            return autoPtr<entry>(new fieldDataEntry(*this));
        }


    //-Destructor
    virtual ~fieldDataEntry()
    {}


    // Member functions

        //- Return the dictionary name
        virtual const fileName& name() const
        {
            return name_;
        }

        //- Return the dictionary name
        virtual fileName& name()
        {
            return name_;
        }

        //- Return line number of first token in dictionary
        virtual label startLineNumber() const;

        //- Return line number of last token in dictionary
        virtual label endLineNumber() const;

        //- Return true if this entry is a stream
        virtual bool isStream() const
        {
            return false;
        }

        //- This entry is not a primitive,
        //  calling this function generates a FatalError
        virtual ITstream& stream() const;

        //- This entry is not a dictionary,
        //  calling this function generates a FatalError
        virtual const dictionary& dict() const;

        //- This entry is not a dictionary,
        //  calling this function generates a FatalError
        virtual dictionary& dict();

        //- Write
        virtual void write(Ostream&) const;

        // Access

            //- Return number of bytes
            inline const word& compoundTokenName() const
            {
                return compoundTokenName_;
            }

            inline fileName id() const
            {
                fileName id = name_;
                id.replaceAll("::", "/");

                return id;
            }

            inline bool uniform() const
            {
                if (tag_.uniformityState() == uListProxyBase::UNIFORM)
                {
                    return true;
                }

                return false;
            }

            //- Set number of elements summed over all ranks
            inline label& nGlobalElems()
            {
                return nGlobalElems_;
            }

            inline const fieldTag& tag() const
            {
                return tag_;
            }

            inline fieldTag& tag()
            {
                return tag_;
            }

            inline const uListProxyBase& uList() const
            {
                return uListProxyPtr_();
            }

        //- Return uniformity of the data list
        inline uListProxyBase::uniformity uniformityState() const
        {
            return tag_.uniformityState();
        }

        //- Return access to the uniformity of the data list
        inline uListProxyBase::uniformity& uniformityState()
        {
            return tag_.uniformityState();
        }
};

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
