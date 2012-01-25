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
* @file MovementGenerator.h
* @author Helge Backhaus
*/

#ifndef __MOVEMENTGENERATOR_H_
#define __MOVEMENTGENERATOR_H_

#include <omnetpp.h>
#include <Vector2D.h>
#include <BoundingBox2D.h>
#include <TransportAddress.h>
#include <list>
#include "GlobalCoordinator.h"

enum SCDir {
    DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT
};

class NeighborMapEntry {
    public:
        Vector2D position, direction;
        friend std::ostream& operator<<(std::ostream& Stream, const NeighborMapEntry& e);
};

typedef std::map<TransportAddress, NeighborMapEntry> NeighborMap;
typedef std::list<BoundingBox2D> CollisionList;

/// (Abstract) MovementGenerator class
/**
An interface for different movement generation set-ups.
*/

class MovementGenerator
{
    public:
        /// Initialize the generator with the movement area dimensions and node movement speed.
        /**
        \@param areaDimension Movement range from [-areaDimension, -areaDimension] to [areaDimension, areaDimension].
        \@param speed Movement speed in units per movement.
        */
        MovementGenerator(double areaDimension, double speed, NeighborMap *Neighbors, GlobalCoordinator* coordinator, CollisionList* CollisionRect);
        virtual ~MovementGenerator() {}
        /// Defined in subclasses only.
        virtual void move() = 0;
        /// Get the nodes current position
        /**
        \@return Returns the current node position.
        */
        Vector2D getPosition();

    protected:
        double areaDimension, speed;
        Vector2D direction, position, target;
        /// Prevents the node from leaving the defined area and checks for obstacle hits.
        bool testBounds();
        /// Simple flocking algorithm.
        void flock();
        /// Generates scenery objects.
        void generateScenery(unsigned int seed);
        NeighborMap *Neighbors;
        NeighborMap::iterator itNeighbors;
        GlobalCoordinator* coordinator;
        CollisionList* CollisionRect;
};

#endif
