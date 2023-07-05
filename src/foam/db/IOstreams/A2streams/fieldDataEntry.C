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

#include "fieldDataEntry.H"
#include "ITstream.H"

static Foam::ITstream dummyITstream_("dummy", Foam::UList<Foam::token>());


// * * * * * * * * * * * * * * * Local Functions * * * * * * * * * * * * * * //

namespace Foam
{

// Capture the first element and save as a scalarList
static inline scalarList getFirstElement(const uListProxyBase* uListProxyPtr)
{
    if (uListProxyPtr && uListProxyPtr->size())
    {
        // Could also return a SubList or UList

        // Needs reworking for labels etc (should save char data)
        const scalar* scalarData =
            reinterpret_cast<const scalar*>(uListProxyPtr->cdata_bytes());

        const label nCmpts = uListProxyPtr->nComponents();

        scalarList list(nCmpts);

        for (label i = 0; i < nCmpts; i++)
        {
            list[i] = scalarData[i];
        }

        return list;
    }

    return scalarList();
}

} // End namespace Foam


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fieldDataEntry::fieldDataEntry
(
    const keyType& keyword,
    const word& compoundTokenName,
    uListProxyBase* uListProxyPtr
)
:
    entry(keyword),
    name_(),
    compoundTokenName_(compoundTokenName),
    uListProxyPtr_(uListProxyPtr),
    nGlobalElems_(0),
    tag_
    (
        uListProxyPtr->determineUniformity(),
        getFirstElement(uListProxyPtr)
    )
{}


// * * * * * * * * * * * * * * * * Destructors * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::label Foam::fieldDataEntry::startLineNumber() const
{
    return 0;
}


Foam::label Foam::fieldDataEntry::endLineNumber() const
{
    return 0;
}


Foam::ITstream& Foam::fieldDataEntry::stream() const
{
    FatalErrorInFunction
        << "Attempt to return field data entry " << keyword()
        << " as a primitive"
        << abort(FatalError);

    return dummyITstream_;
}


const Foam::dictionary& Foam::fieldDataEntry::dict() const
{
    FatalErrorInFunction
        << "Attempt to return primitive entry " << keyword()
        << " as a sub-dictionary"
        << abort(FatalError);

    return dictionary::null;
}


Foam::dictionary& Foam::fieldDataEntry::dict()
{
    FatalErrorInFunction
        << "Attempt to return primitive entry " << keyword()
        << " as a sub-dictionary"
        << abort(FatalError);

    return const_cast<dictionary&>(dictionary::null);
}


void Foam::fieldDataEntry::write(Ostream& os) const
{
    // dict uses "::" as a separator - replace it with the fileName standard "/"
    fileName fn = name();
    fn.replaceAll("::", "/");
    os.writeKeyword(fn.name());

    if (tag_.uniformityState() == uListProxyBase::UNIFORM)
    {
        os  << "uniform" << token::SPACE;

        // If EMPTY, data is nullptr. Thus, use the field tag to provide the
        // first element.
        uListProxyPtr_->writeFirstElement
        (
            os,
            reinterpret_cast<const char*>(tag_.firstElement().cdata())
        );
    }
    else
    {
        os  << "nonuniform" << token::SPACE << compoundTokenName_
            << token::SPACE << nGlobalElems_ << fn;
    }

    os << token::END_STATEMENT << endl;
}


// ************************************************************************* //
