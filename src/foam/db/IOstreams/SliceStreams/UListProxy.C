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

#include "UListProxy.H"
#include "primitives_traits.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class T>
Foam::UListProxy<T>::UListProxy(UList<T> uList)
:
    UList<T>(uList.data(), uList.size())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class T>
void Foam::UListProxy<T>::writeFirstElement
(
    Ostream& os,
    const char* data
) const
{
    if (data)
    {
        const T* dataT = reinterpret_cast<const T*>(data);
        os << *dataT;
    }
    else
    {
        FatalErrorInFunction
            << "Given data pointer is nullptr."
            << abort(FatalError);
    }
}


#if (OPENFOAM >= 2206)

template<class T>
const char* Foam::UListProxy<T>::cdata_bytes() const
{
    return UList<T>::cdata_bytes();
}


template<class T>
char* Foam::UListProxy<T>::data_bytes()
{
    return UList<T>::data_bytes();
}


template<class T>
std::streamsize Foam::UListProxy<T>::size_bytes() const
{
    return UList<T>::size_bytes();
}

#else

template<class T>
const char* Foam::UListProxy<T>::cdata_bytes() const
{
    return reinterpret_cast<const char*>(UList<T>::cdata());
}


template<class T>
char* Foam::UListProxy<T>::data_bytes()
{
    return reinterpret_cast<char*>(UList<T>::data());
}


template<class T>
std::streamsize Foam::UListProxy<T>::size_bytes() const
{
    return std::streamsize(UList<T>::size())*sizeof(T);
}

#endif


template<class T>
Foam::label Foam::UListProxy<T>::byteSize() const
{
    return UList<T>::byteSize();
}


template<class T>
Foam::label Foam::UListProxy<T>::size() const
{
    return UList<T>::size();
}


template<class T>
Foam::label Foam::UListProxy<T>::nComponents() const
{
    return Foam::nComponentsOf<T>();
}


template<class T>
char* Foam::UListProxy<T>::data()
{
    return reinterpret_cast<char*>(UList<T>::data());
}


template<class T>
const char* Foam::UListProxy<T>::cdata() const
{
    return reinterpret_cast<const char*>(UList<T>::cdata());
}


template<class T>
Foam::uListProxyBase::uniformity
Foam::UListProxy<T>::determineUniformity() const
{
    label nElems = size();

    if (nElems > 0)
    {
        uListProxyBase::uniformity u = uListProxyBase::UNIFORM;

        // Skip comparison of the first element with itself
        for (label i = 1; i < nElems; i++)
        {
            if (UList<T>::operator[](i) != UList<T>::operator[](0))
            {
                u = uListProxyBase::NONUNIFORM;
                break;
            }
        }

        return u;
    }
    else
    {
        return uListProxyBase::EMPTY;
    }
}

// ************************************************************************* //
