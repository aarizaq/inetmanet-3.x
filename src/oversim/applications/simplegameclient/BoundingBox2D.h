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
 * @file BoundingBox2D.h
 * @author Helge Backhaus
 */

/// BoundingBox2D class
/**
 * axies aligned two dimensional bounding box
 */

#ifndef __BOUNDINGBOX2D_H_
#define __BOUNDINGBOX2D_H_

#include <Vector2D.h>

class BoundingBox2D
{
    public:
        BoundingBox2D();
        BoundingBox2D(Vector2D tl, Vector2D br);
        BoundingBox2D(double tlx, double tly, double brx, double bry);
        BoundingBox2D(Vector2D center, double width);
        bool collide(const BoundingBox2D box) const;
        bool collide(const Vector2D p) const;
        double top();
        double bottom();
        double left();
        double right();
        Vector2D tl, br; //top left, bottom right
        friend std::ostream& operator<<(std::ostream& Stream, const BoundingBox2D& box);
};

#endif
