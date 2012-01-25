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
* @file SimpleGameClient.h
* @author Helge Backhaus
* @author Stephan Krause
*/

#ifndef __SIMPLEGAMECLIENT_H_
#define __SIMPLEGAMECLIENT_H_

#include <map>
#include <string>
#include <realtimescheduler.h>
#include <Vector2D.h>
#include <BaseApp.h>

#include <GlobalCoordinator.h>
#include <GlobalStatisticsAccess.h>
#include <MovementGenerator.h>
#include <randomRoaming.h>
#include <groupRoaming.h>
#include <hotspotRoaming.h>
#include <traverseRoaming.h>
#include <greatGathering.h>
#include <realWorldRoaming.h>
#include "SCPacket.h"
#include "SimpleGameClient_m.h"

#include <tunoutscheduler.h>


/// SimpleGameClient class
/**
A simple test application, which simulates node movement based on different movement generators.
*/

class SimpleGameClient : public BaseApp
{
    public:
        // OMNeT++
        virtual ~SimpleGameClient();
        virtual void initializeApp(int stage);
        virtual void handleTimerEvent(cMessage* msg);
        virtual void handleLowerMessage(cMessage* msg);
        virtual void handleReadyMessage(CompReadyMessage* msg);
        virtual void finishApp();

        Vector2D getPosition() {return position;};
        double getAOI() {return AOIWidth;};
        bool isOverlayReady() {return overlayReady;};
        NodeHandle getThisNode() {return overlay->getThisNode();};

        NeighborMap Neighbors;

    protected:
        GlobalCoordinator* coordinator;
        CollisionList CollisionRect;

        void updateNeighbors(GameAPIListMessage* sgcMsg);
        void updatePosition();

        // parameters
        simtime_t movementDelay;
        double areaDimension, movementSpeed, movementRate, AOIWidth;
        bool useScenery;
        bool overlayReady;
        Vector2D position;
        MovementGenerator *Generator;
        bool logAOI;

        NeighborMap::iterator itNeighbors;
        std::string GeneratorType;

        // timers
        cMessage* move_timer;

        // realworld
        void handleRealworldPacket(char *buf, uint32_t len);
        cMessage* packetNotification; // used by TunOutScheduler to notify about new packets
        RealtimeScheduler::PacketBuffer packetBuffer; // received packets are stored here
        RealtimeScheduler* scheduler;
        unsigned int mtu;
        SOCKET appFd;
        bool doRealworld;
        bool frozen;

        // AOI measuring / hotspots
        double startAOI;
        bool useHotspots;
        bool lastInHotspot;
        bool lastFarFromHotspot;
        simtime_t lastAOImeasure;
        double avgAOI;
        simtime_t nonHotspotTime;
        simtime_t farFromHotspotTime;
        double avgFarFromHotspotAOI;
        simtime_t hotspotTime;
        double avgHotspotAOI;
};

#endif
