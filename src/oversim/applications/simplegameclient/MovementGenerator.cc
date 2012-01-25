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
* @file MovementGenerator.cc
* @author Helge Backhaus
*/

#include "MovementGenerator.h"

std::ostream& operator<<(std::ostream& Stream, const NeighborMapEntry& e)
{
    return Stream << "Positon: " << e.position << " Direction: " << e.direction;
}

MovementGenerator::MovementGenerator(double areaDimension, double speed, NeighborMap *Neighbors, GlobalCoordinator* coordinator, CollisionList* CollisionRect)
{
    this->areaDimension = areaDimension;
    this->speed = speed;
    this->Neighbors = Neighbors;
    this->coordinator = coordinator;
    this->CollisionRect = CollisionRect;

    Vector2D center(areaDimension / 2, areaDimension / 2);
    position.x = uniform(0.0, areaDimension);
    position.y = uniform(0.0, areaDimension);
    direction = center - position;
    direction.normalize();
    if(CollisionRect != NULL) {
        generateScenery(coordinator->getSeed());
    }
}

Vector2D MovementGenerator::getPosition()
{
    return position;
}

bool MovementGenerator::testBounds()
{
    bool obstacleHit = false;

    if(position.x < 0.0) {
        position.x = 0.0;
    }
    if(position.x > areaDimension) {
       position.x = areaDimension;
    }
    if(position.y < 0.0) {
        position.y = 0.0;
    }
    if(position.y > areaDimension) {
        position.y = areaDimension;
    }

    double cosAngle = direction.x;
    SCDir scDirection;
    if(cosAngle > 0.71) {
        scDirection = DIR_RIGHT;
    }
    else if(cosAngle < -0.71) {
        scDirection = DIR_LEFT;
    }
    else if(direction.y > 0.0) {
        scDirection = DIR_UP;
    }
    else {
        scDirection = DIR_DOWN;
    }

    if(CollisionRect != NULL) {
        CollisionList::iterator i;
        for(i = CollisionRect->begin(); i != CollisionRect->end(); ++i) {
            if(i->collide(position)) {
                switch(scDirection) {
                    case DIR_UP:
                        position.y = i->bottom();
                        direction.x = 1.0;
                        direction.y = 0.0;
                        break;
                    case DIR_DOWN:
                        position.y = i->top();
                        direction.x = -1.0;
                        direction.y = 0.0;
                        break;
                    case DIR_LEFT:
                        position.x = i->right();
                        direction.x = 0.0;
                        direction.y = 1.0;
                        break;
                    case DIR_RIGHT:
                        position.x = i->left();
                        direction.x = 0.0;
                        direction.y = -1.0;
                        break;
                }
                obstacleHit = true;
            }
        }
    }

    return obstacleHit;
}

void MovementGenerator::flock()
{
    Vector2D separation, alignment, cohesion, toTarget;
    int NeighborCount = 0;

    for(itNeighbors = Neighbors->begin(); itNeighbors != Neighbors->end(); ++itNeighbors)
        if(position.distanceSqr(itNeighbors->second.position) < 2.5 && direction.cosAngle(itNeighbors->second.position - position) > -0.75) {
            separation += position - itNeighbors->second.position;
            alignment += itNeighbors->second.direction;
            cohesion += itNeighbors->second.position;
            ++NeighborCount;
        }

    if(NeighborCount > 0) {
        cohesion /= (double)NeighborCount;
        cohesion = cohesion - position;
        separation.normalize();
        alignment.normalize();
        cohesion.normalize();
    }
    toTarget = target - position;
    toTarget.normalize();

    direction = separation * 0.4 + alignment * 0.1 + cohesion * 0.35 + toTarget * 0.25;
    direction.normalize();
}

void MovementGenerator::generateScenery(unsigned int seed)
{
    int dimension = (int)(areaDimension - 10.0);
    int sceneryType;
    double sceneryX, sceneryY;

    srand(seed);

    for(int i = 0; i < dimension; i += 10) {
        for(int j = 0; j < dimension; j+= 10) {
            sceneryType = rand() % 3;
            switch(sceneryType) {
                case 0: { // mud
                    // do nothing except calling rand() for deterministic scenery generation
                    sceneryX = rand();
                    sceneryY = rand();
                } break;
                case 1: { // rock
                    sceneryX = 1 + (rand() % 8);
                    sceneryY = 1 + (rand() % 8);
                    CollisionRect->insert(CollisionRect->begin(), BoundingBox2D(i + sceneryX - 0.25, j + sceneryY - 0.5, i + sceneryX + 1.25, j + sceneryY + 0.25));
                } break;
                case 2: { // tree
                    sceneryX = 1 + (rand() % 6);
                    sceneryY = 1 + (rand() % 5);
                    CollisionRect->insert(CollisionRect->begin(), BoundingBox2D(i + sceneryX, j + sceneryY + 2.0, i + sceneryX + 3.0, j + sceneryY + 3.5));
                } break;
            }
        }
    }
}
