//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file QuonHelper.cc
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#include <QuonHelper.h>

QuonAOI::QuonAOI(bool useSquareMetric)
{
    this->useSquareMetric = useSquareMetric;
    radius = 0.0;
}

QuonAOI::QuonAOI(Vector2D center, double radius, bool useSquareMetric)
{
    this->useSquareMetric = useSquareMetric;
    this->center = center;
    this->radius = radius;
}

void QuonAOI::resize(double radius)
{
    this->radius = radius;
}

bool QuonAOI::collide(const Vector2D p) const
{
    if(!useSquareMetric && center.distanceSqr(p) < (radius*radius))
    {
        return true;
    }
    else if(useSquareMetric && center.xyMaxDistance(p) < (radius))
    {
        return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& Stream, const QuonAOI& aoi)
{
    return Stream << aoi.center << " - " << aoi.radius;
}

QuonSite::QuonSite()
{
    type = QUNDEFINED;
    dirty = false;
    alive = false;
    softNeighbor = false;
    address = NodeHandle::UNSPECIFIED_NODE;
    AOIwidth = 0.0;
}

std::ostream& operator<<(std::ostream& Stream, const QuonSite& s)
{
    Stream << s.address.getIp() << ":" << s.address.getPort() << " Type: ";
    switch(s.type) {
        case QUNDEFINED:
            if( s.softNeighbor) {
                Stream << "\"Softstate Neighbor\"";
            } else {
                Stream << "\"Undefined\"";
            }
            break;
        case QTHIS:
            Stream << "\"Self\"";
            break;
        case QNEIGHBOR:
            Stream << "\"Direct Neighbor\"";
            break;
        case QBINDING:
            Stream << "\"Binding Neighbor\"";
            break;
            break;
    }
    Stream << " Position: " << s.position;
    return Stream;
}

