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

#include "arcEdge.H"
#include "unitConversion.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(arcEdge, 0);
    addToRunTimeSelectionTable(curvedEdge, arcEdge, Istream);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

Foam::cylindricalCS Foam::arcEdge::calcAngle()
{
    vector a = p2_ - p1_;
    vector b = p3_ - p1_;

    // find centre of arcEdge
    scalar asqr = a & a;
    scalar bsqr = b & b;
    scalar adotb = a & b;

    scalar denom = asqr*bsqr - adotb*adotb;

    if (mag(denom) < VSMALL)
    {
        FatalErrorIn("cylindricalCS arcEdge::calcAngle()")
            << "Invalid arc definition - are the points co-linear?  Denom ="
            << denom
            << abort(FatalError);
    }

    scalar fact = 0.5*(bsqr - adotb)/denom;

    point centre = 0.5*a + fact*((a ^ b) ^ a);

    centre += p1_;

    // find position vectors w.r.t. the arcEdge centre
    vector r1(p1_ - centre);
    vector r2(p2_ - centre);
    vector r3(p3_ - centre);

    // find angles
    angle_ = radToDeg(acos((r3 & r1)/(mag(r3) * mag(r1))));

    // check if the vectors define an exterior or an interior arcEdge
    if (((r1 ^ r2) & (r1 ^ r3)) < 0.0)
    {
        angle_ = 360.0 - angle_;
    }

    vector tempAxis;

    if (angle_ <= 180.0)
    {
        tempAxis = r1 ^ r3;

        if (mag(tempAxis)/(mag(r1)*mag(r3)) < 0.001)
        {
            tempAxis = r1 ^ r2;
        }
    }
    else
    {
        tempAxis = r3 ^ r1;
    }

    radius_ = mag(r3);

    // set up and return the local coordinate system
    return cylindricalCS("arcEdgeCS", centre, tempAxis, r1);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::arcEdge::arcEdge
(
    const pointField& points,
    const label start,
    const label end,
    const point& pMid
)
:
    curvedEdge(points, start, end),
    p1_(points_[start_]),
    p2_(pMid),
    p3_(points_[end_]),
    cs_(calcAngle())
{}


Foam::arcEdge::arcEdge(const pointField& points, Istream& is)
:
    curvedEdge(points, is),
    p1_(points_[start_]),
    p2_(is),
    p3_(points_[end_]),
    cs_(calcAngle())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::point Foam::arcEdge::position(const scalar lambda) const
{
    if (lambda < 0 || lambda > 1)
    {
        FatalErrorIn("arcEdge::position(const scalar lambda) const")
            << "Parameter out of range, lambda = " << lambda
            << abort(FatalError);
    }

    if (lambda < SMALL)
    {
        return p1_;
    }
    else if (lambda > 1 - SMALL)
    {
        return p3_;
    }
    else
    {
        return cs_.globalPosition(vector(radius_, lambda*angle_, 0.0));
    }
}


Foam::scalar Foam::arcEdge::length() const
{
    return degToRad(angle_*radius_);
}


// ************************************************************************* //
