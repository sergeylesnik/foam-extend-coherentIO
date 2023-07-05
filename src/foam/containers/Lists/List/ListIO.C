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

#include "List.H"
#include "Istream.H"
#include "token.H"
#include "SLList.H"
#include "contiguous.H"
#include <iostream>

#include "Ostream.H"
#include "prefixOSstream.H"

// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

namespace Foam {
    extern prefixOSstream Pout;
}

template<class T>
Foam::List<T>::List(Istream& is)
:
    UList<T>(nullptr, 0)
{
    if (List<T>::debug)
    {
        Pout<< "List<T>::List(Istream& is)" << endl;
    }

    operator>>(is, *this);
}


template<class T>
Foam::Istream& Foam::operator>>(Istream& is, List<T>& list)
{
    if (List<T>::debug)
    {
        Pout<< "ListIO: operator>>, start" << endl;
    }

    // Anull list
    list.resize(0);

    is.fatalCheck(FUNCTION_NAME);

    token firstToken(is);

    is.fatalCheck(FUNCTION_NAME);

    // Compound: simply transfer contents
    if (firstToken.isCompound())
    {
        if (List<T>::debug)
        {
            Pout<< "ListIO: operator>>, transfering compound token" << endl;
        }

        list.transfer
        (
            dynamicCast<token::Compound<List<T> > >
            (
                firstToken.transferCompoundToken(is)
            )
        );

        return is;
    }


    // Label: could be int(..), int{...} or just a plain '0'
    if (firstToken.isLabel())
    {
        const label len = firstToken.labelToken();

        if (List<T>::debug)
        {
            Pout<< "ListIO: operator>>, reading a list of size " << len << endl;
        }

        if (is.format() != IOstream::COHERENT)
        {
            // Resize to length read
            list.resize(len);
        }

        // Read list contents depending on data format

        if (is.format() == IOstream::ASCII || !contiguous<T>())
        {
            if (is.format() == IOstream::COHERENT)
            {
                // Read the id of list contents
                string id;
                is >> id;

                // TODO: Re-enable reading non-contiguous data.
                //Istream& iss = is.readToStringStream(id);
                //for (label i=0; i<len; ++i)
                //{
                    //iss >> list[i];

                    //iss.fatalCheck
                    //(
                        //"operator>>(Istream&, List<T>&) : "
                        //"reading entry"
                    //);
                //}
                if (List<T>::debug > 1)
                {
                    Pout<< "List read via COHERENT = " << nl << list << endl;
                }
            }
            else
            {
                // Read beginning of contents
                const char delimiter = is.readBeginList("List");

                if (len)
                {
                    if (delimiter == token::BEGIN_LIST)
                    {
                        for (label i=0; i<len; ++i)
                        {
                            is >> list[i];

                            is.fatalCheck
                            (
                                "operator>>(Istream&, List<T>&) : "
                                "reading entry"
                            );
                        }
                    }
                    else
                    {
                        // Uniform content (delimiter == token::BEGIN_BLOCK)

                        T element;
                        is >> element;

                        is.fatalCheck
                        (
                            "operator>>(Istream&, List<T>&) : "
                            "reading the single entry"
                        );

                        for (label i=0; i<len; ++i)
                        {
                            list[i] = element;  // Copy the value
                        }
                    }
                }

                // Read end of contents
                is.readEndList("List");
            }
        }
        else if (len && is.format() == IOstream::BINARY)
        {
            // Non-empty, binary, contiguous

            is.read(reinterpret_cast<char*>(list.data()), len*sizeof(T));

            is.fatalCheck
            (
                "operator>>(Istream&, List<T>&) : "
                "reading the binary block"
            );
        }
        else if (len && is.format() == IOstream::COHERENT)
        {
            string id;
            is >> id;

            if (List<T>::debug)
            {
                Pout<< "Reading via COHERENT id = " << id << endl;
            }

            if (UList<T>::debug > 1)
            {
                Pout<< "List read via COHERENT = " << nl << list << endl;
            }

        }

        return is;
    }


    // "(...)" : read as SLList and transfer contents
    if (firstToken.isPunctuation())
    {
        if (firstToken.pToken() != token::BEGIN_LIST)
        {
            FatalIOErrorInFunction(is)
                << "incorrect first token, expected '(', found "
                << firstToken.info()
                << exit(FatalIOError);
        }

        is.putBack(firstToken); // Putback the opening bracket

        SLList<T> sll(is);      // Read as singly-linked list

        // Reallocate and move assign list elements
        list = std::move(sll);

        return is;
    }


    FatalIOErrorInFunction(is)
        << "incorrect first token, expected <int> or '(', found "
        << firstToken.info()
        << exit(FatalIOError);

    return is;
}


template<class T>
Foam::List<T> Foam::readList(Istream& is)
{
    List<T> L;
    token firstToken(is);
    is.putBack(firstToken);

    if (firstToken.isPunctuation())
    {
        if (firstToken.pToken() != token::BEGIN_LIST)
        {
            FatalIOErrorInFunction(is)
                << "incorrect first token, expected '(', found "
                << firstToken.info()
                << exit(FatalIOError);
        }

        // Read via a singly-linked list
        L = SLList<T>(is);
    }
    else
    {
        // Create list with a single item
        L.setSize(1);

        is >> L[0];
    }

    return L;
}


// ************************************************************************* //
