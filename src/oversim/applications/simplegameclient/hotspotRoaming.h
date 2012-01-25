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
* @file hotspotRoaming.h
* @author Stephan Krause
*/

#ifndef __HOTSPOTROAMING_H_
#define __HOTSPOTROAMING_H_

#include "MovementGenerator.h"

/// hotspotRoaming class
/**
Simulates nodes roaming an area with hotpots.
*/

class hotspotRoaming : public MovementGenerator
{
    protected:
        struct Hotspot {
            Vector2D center;
            double radius;
            double probability;
        };
        std::vector<Hotspot> hotspots;
        std::vector<Hotspot>::iterator curHotspot;
        simtime_t hotspotStayTime;
        bool stayInHotspot;
    public:
        hotspotRoaming(double areaDimension, double speed, NeighborMap *Neighbors, GlobalCoordinator* coordinator, CollisionList* CollisionRect);
        double getDistanceFromHotspot();
        virtual ~hotspotRoaming() {}
        virtual void move();
};

#endif
