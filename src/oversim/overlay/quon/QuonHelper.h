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
 * @file QuonHelper.h
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#ifndef __QUONHELPER_H_
#define __QUONHELPER_H_

#include <map>
#include <QuonDefs.h>

/// QuonP Definitions

class QuonAOI
{
    public:
        QuonAOI(bool useSquareMetric = false);
        QuonAOI(Vector2D center, double radius, bool useSquareMetric = false);
        void resize(double radius);
        bool collide(const Vector2D p) const;
        Vector2D center;
        double radius;
        bool useSquareMetric;
        friend std::ostream& operator<<(std::ostream& Stream, const QuonAOI& aoi);
};

class QuonSite
{
    public:
        QuonSite();
        Vector2D position;
        QNeighborType type;
        NodeHandle address;
        double AOIwidth;
        bool dirty;
        bool alive;
        bool softNeighbor;
        friend std::ostream& operator<<(std::ostream& Stream, const QuonSite& s);
};

typedef std::map<OverlayKey, QuonSite*> QuonSiteMap;

#endif
