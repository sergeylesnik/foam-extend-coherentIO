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

#include "fieldTag.H"

// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

Foam::List<Foam::fieldTag>
Foam::fieldTag::uniformityCompareOp
(
    const List<fieldTag>& x,
    const List<fieldTag>& y
)
{
    List<fieldTag> res(x);

    forAll(x, i)
    {
        if
        (
            x[i].uniformity_ == uListProxyBase::NONUNIFORM
         || y[i].uniformity_ == uListProxyBase::NONUNIFORM
        )
        {
            res[i].uniformity_ = uListProxyBase::NONUNIFORM;
        }
        else if (x[i].uniformity_ == uListProxyBase::EMPTY)
        {
            res[i] = y[i];
        }
        else if
        (
            x[i].uniformity_ == uListProxyBase::UNIFORM
        )
        {
            if (y[i].uniformity_ == uListProxyBase::UNIFORM)
            {
                if (x[i].firstElement_ != y[i].firstElement_)
                {
                    res[i].uniformity_ = uListProxyBase::NONUNIFORM;
                }
            }
            else if (y[i].uniformity_ == uListProxyBase::NONUNIFORM)
            {
                res[i].uniformity_ = uListProxyBase::NONUNIFORM;
            }
        }
    }

    return res;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::fieldTag::fieldTag()
:
    uniformity_(),
    firstElement_()
{}


Foam::fieldTag::fieldTag(uListProxyBase::uniformity u, const scalarList& l)
:
    uniformity_(u),
    firstElement_(l)
{}


Foam::fieldTag::fieldTag(const fieldTag& t)
:
    uniformity_(t.uniformity_),
    firstElement_(t.firstElement_)
{}


// * * * * * * * * * * * * * Public Member Functions  * * * * * * * * * * * //

bool Foam::fieldTag::operator==(const fieldTag& d) const
{
    if (this->uniformity_ != d.uniformity_)
    {
        return false;
    }
    else if (this->firstElement_ != d.firstElement_)
    {
        return false;
    }

    return true;
}


bool Foam::fieldTag::operator!=(const fieldTag& d) const
{
    return !operator==(d);
}


void Foam::fieldTag::operator=(const fieldTag& t)
{
    this->uniformity_ = t.uniformity_;
    this->firstElement_ = t.firstElement_;
}


Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const fieldTag& d
)
{
    os << d.firstElement() << d.uniformityState();
    return os;
}

Foam::Istream& Foam::operator>>
(
    Istream& is,
    fieldTag& d
)
{
    is >> d.firstElement();
    label lfu;
    is >> lfu;
    d.uniformityState() = static_cast<uListProxyBase::uniformity>(lfu);
    return is;
}
