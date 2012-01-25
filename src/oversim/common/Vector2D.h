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
 * @file Vector2D.h
 * @author Helge Backhaus
 */

#ifndef __VECTOR2D_H_
#define __VECTOR2D_H_

#include <math.h>
#include <omnetpp.h>
#include <iostream>

class Vector2D
{
    public:
        Vector2D();
        Vector2D(double x, double y);

        double x, y;

        void normalize();
        double distanceSqr(const Vector2D v) const;
        double xyMaxDistance(const Vector2D v) const;
        double cosAngle(const Vector2D& v) const;
        int getQuadrant(const Vector2D& v) const;

        Vector2D& operator=(const Vector2D& v);
        Vector2D& operator+=(const Vector2D& v);
        Vector2D& operator-=(const Vector2D& v);
        Vector2D& operator*=(const double s);
        Vector2D& operator/=(const double s);
        Vector2D operator+(const Vector2D& v) const;
        Vector2D operator-(const Vector2D& v) const;
        Vector2D operator*(const double s) const;
        Vector2D operator/(const double s) const;
        bool operator==(const Vector2D& v) const;
        bool operator!=(const Vector2D& v) const;

        friend bool operator<(const Vector2D& a, const Vector2D& b);
        friend std::ostream& operator<<(std::ostream& Stream, const Vector2D& v);

        void netPack(cCommBuffer *b);
        void netUnpack(cCommBuffer *b);
};

/**
* netPack for Vector2D
*
* @param b the buffer
* @param obj the Vector2D to serialise
*/
inline void doPacking(cCommBuffer *b, Vector2D& obj) {obj.netPack(b);}

/**
* netUnpack for Vector2D
*
* @param b the buffer
* @param obj the Vector2D to unserialise
*/
inline void doUnpacking(cCommBuffer *b, Vector2D& obj) {obj.netUnpack(b);}

#endif
