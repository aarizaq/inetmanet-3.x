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
* @file GlobalCoordinator.cc
* @author Helge Backhaus
*/

#include "GlobalCoordinator.h"

Define_Module(GlobalCoordinator);

void GlobalCoordinator::initialize()
{
    PositionSize = 0;
    PeerCount = 0;
    Position = NULL;
    Seed = par("seed");

    WATCH(PositionSize);
}

GlobalCoordinator::~GlobalCoordinator()
{
    delete[] this->Position;
}

void GlobalCoordinator::handleMessage(cMessage* msg)
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void GlobalCoordinator::increasePositionSize()
{
    Enter_Method_Silent();
    PositionSize++;
    Vector2D *Temp = new Vector2D[PositionSize];
    for(int i=0; i<PositionSize-1; i++)
        Temp[i] = this->Position[i];

    delete[] this->Position;
    this->Position = Temp;
}

void GlobalCoordinator::increasePeerCount()
{
    Enter_Method_Silent();
    PeerCount++;
}

int GlobalCoordinator::getPeerCount()
{
    Enter_Method_Silent();
    return PeerCount;
}

Vector2D& GlobalCoordinator::getPosition(int k)
{
    Enter_Method_Silent();
    if(k >= PositionSize || k < 0) {
        throw cRuntimeError("Array out of bounds exception! getPosition(%d)", k);
    }
    return Position[k];
}

void GlobalCoordinator::setPosition(int k, const Vector2D& Position)
{
    Enter_Method_Silent();
    if(k >= PositionSize || k < 0) {
        throw cRuntimeError("Array out of bounds exception! setPosition(%d, ...)", k);
    }
    this->Position[k] = Position;
}

unsigned int GlobalCoordinator::getSeed()
{
    return Seed;
}
