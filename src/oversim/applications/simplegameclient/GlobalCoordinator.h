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
 * @file GlobalCoordinator.h
 * @author Helge Backhaus
 */

#ifndef __GLOBALCOORDINATOR_H__
#define __GLOBALCOORDINATOR_H__

#include <omnetpp.h>
#include <Vector2D.h>
#include <NodeHandle.h>
#include <time.h>

class GlobalCoordinator : public cSimpleModule
{
    public:
        virtual void initialize();
        virtual void handleMessage(cMessage* msg);
        virtual ~GlobalCoordinator();

        void increasePositionSize();
        void increasePeerCount();
        int getPeerCount();
        Vector2D& getPosition(int k);
        void setPosition(int k, const Vector2D& Position);
        unsigned int getSeed();

    protected:
        // ptr to an vector array
        Vector2D *Position;
        int PositionSize, PeerCount;
        unsigned int Seed;
};

#endif
