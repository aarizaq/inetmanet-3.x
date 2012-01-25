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
 * @file Vector2D.cc
 * @author Helge Backhaus
 */

#include <Vector2D.h>
#include <algorithm>

Vector2D::Vector2D()
{
    x = 0.0;
    y = 0.0;
}

Vector2D::Vector2D(double x, double y)
{
    this->x = x;
    this->y = y;
}

void Vector2D::normalize()
{
    double temp;
    temp = sqrt(x * x + y * y);
    if(temp != 0.0) {
        x /= temp;
        y /= temp;
    }
}

double Vector2D::distanceSqr(const Vector2D v) const
{
    double dx, dy;
    dx = x - v.x;
    dy = y - v.y;
    return dx * dx + dy * dy;
}

double Vector2D::xyMaxDistance(const Vector2D v) const
{
    return std::max(fabs(x - v.x), fabs(y - v.y));
}

double Vector2D::cosAngle(const Vector2D& v) const
{
    return (x * v.x + y * v.y) / (sqrt(x * x + y * y) * sqrt(v.x * v.x + v.y * v.y));
}

/**
  * @brief  Determine the Quarant a point is contained in
  *
  *         Return the quadrant of this point that containes point v.
  *         Upper right quadrant is 0, lower right is 1, lower left is 2 and upper left is 3
  *
  * @param v       the target point
  * @return        the quadrant v is in in
  *
*/

int Vector2D::getQuadrant(const Vector2D& v) const
{
    int quad = 0;
    // v.y <= this.y -> quadrant 1 or 2
    if( v.y <= y ) quad = 1;
    // v.x <= this.x -> quadrant 2 or 3
    if( v.x <= x ) quad ^= 3;
    return quad;
}

Vector2D& Vector2D::operator=(const Vector2D& v)
{
    x = v.x;
    y = v.y;
    return *this;
}

Vector2D& Vector2D::operator+=(const Vector2D& v)
{
    x += v.x;
    y += v.y;
    return *this;
}

Vector2D& Vector2D::operator-=(const Vector2D& v)
{
    x -= v.x;
    y -= v.y;
    return *this;
}

Vector2D& Vector2D::operator*=(const double s)
{
    x *= s;
    y *= s;
    return *this;
}

Vector2D& Vector2D::operator/=(const double s)
{
    x /= s;
    y /= s;
    return *this;
}

Vector2D Vector2D::operator+(const Vector2D& v) const
{
    Vector2D temp;
    temp.x = x + v.x;
    temp.y = y + v.y;
    return temp;
}

Vector2D Vector2D::operator-(const Vector2D& v) const
{
    Vector2D temp;
    temp.x = x - v.x;
    temp.y = y - v.y;
    return temp;
}

Vector2D Vector2D::operator*(const double s) const
{
    Vector2D temp;
    temp.x = x * s;
    temp.y = y * s;
    return temp;
}

Vector2D Vector2D::operator/(const double s) const
{
    Vector2D temp;
    temp.x = x / s;
    temp.y = y / s;
    return temp;
}

bool Vector2D::operator==(const Vector2D& v) const
{
    if(x == v.x && y == v.y)
        return true;
    else
        return false;
}

bool Vector2D::operator!=(const Vector2D& v) const
{
    if(x != v.x || y != v.y)
        return true;
    else
        return false;
}

bool operator<(const Vector2D& a, const Vector2D& b)
{
    if(a.y == b.y)
        return a.x < b.x;
    else
        return a.y < b.y;
}

std::ostream& operator<<(std::ostream& Stream, const Vector2D& v)
{
    return Stream << std::fixed << "[" << v.x << ", " << v.y << "]";
}

void Vector2D::netPack(cCommBuffer *b)
{
    //cMessage::netPack(b);
    doPacking(b, this->x);
    doPacking(b, this->y);
}

void Vector2D::netUnpack(cCommBuffer *b)
{
    //cMessage::netUnpack(b);
    doUnpacking(b, this->x);
    doUnpacking(b, this->y);
}
