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
 * @file Vast.h
 * @author Helge Backhaus
 */

#ifndef __VAST_H_
#define __VAST_H_

#include <omnetpp.h>
#include <BaseOverlay.h>
#include <float.h>
#include <VastDefs.h>
#include <Vast_m.h>
//#include <IPvXAddress.h>
//#include <NodeHandle.h>
//#include <NeighborsList.h>

/// Voronoi class
/**
 * An overlay network based on voronoi diagrams.
 */

class Vast : public BaseOverlay
{
    public:
        // OMNeT++
        ~Vast();
        void initializeOverlay(int stage);
        void finishOverlay();
        void handleUDPMessage(BaseOverlayMessage* msg);
        void handleTimerEvent(cMessage* msg);
        void handleAppMessage(cMessage* msg);
        void handleNodeLeaveNotification();
        void handleNodeGracefulLeaveNotification();

        double getAOI();
        Vector2D getPosition();
        NodeHandle getHandle();
        double getAreaDimension();

        SiteMap Sites;
        Site thisSite;

    protected:
        // node references
        double AOI_size;
        double areaDimension;
        PositionSet Positions;
        StockList Stock;

        // statistics
        long joinRequestBytesSent;
        long joinAcknowledgeBytesSent;
        long nodeMoveBytesSent;
        long newNeighborsBytesSent;
        long nodeLeaveBytesSent;
        long enclosingNeighborsRequestBytesSent;
        long pingBytesSent;
        long pongBytesSent;
        long discardNodeBytesSent;

        long maxBytesPerSecondSent, averageBytesPerSecondSent, bytesPerSecond;
        unsigned int secTimerCount;

        // parameters
        bool debugVoronoiOutput;
        simtime_t joinTimeout, pingTimeout, discoveryIntervall, checkCriticalIntervall;
        double criticalThreshold;
        unsigned long stockListSize;

        // Voronoi parameters
        Geometry geom;
        EdgeList edgelist;
        HeapPQ heap;

        void addNode(Vector2D p, NodeHandle node, int NeighborCount = 0);
        void addNodeToStock(NodeHandle node);
        void removeNode(NodeHandle node);
        void buildVoronoi();
        void buildVoronoi(Vector2D old_pos, Vector2D new_pos, NodeHandle enclosingCheck = NodeHandle::UNSPECIFIED_NODE);
        void removeNeighbors();

        // timers
        cMessage* join_timer;
        cMessage* ping_timer;
        cMessage* discovery_timer;
        cMessage* checkcritical_timer;
        cMessage* sec_timer;

        void sendToApp(cMessage* msg);
        void sendMessage(VastMessage* vastMsg, NodeHandle destAddr);
        void setBootstrapedIcon();
        void changeState(int state);

        // timer processing
        void processJoinTimer();
        void processPingTimer();
        void processSecTimer();
        void processCheckCriticalTimer();
        void processDiscoveryTimer();

        // app handlers
        void handleJoin(GameAPIPositionMessage* sgcMsg);
        void handleMove(GameAPIPositionMessage* sgcMsg);
        void handleEvent(GameAPIMessage* msg);

        // overlay handlers
        void handleJoinRequest(VastMessage *vastMsg);
        void handleJoinAcknowledge(VastListMessage *vastListMsg);
        void handleNodeMove(VastMoveMessage *vastMoveMsg);
        void handleNewNeighbors(VastListMessage *vastListMsg);
        void handleNodeLeave(VastListMessage *vastListMsg);
        void handleEnclosingNeighborsRequest(VastMessage *vastMsg);
        void handleBackupNeighbors(VastListMessage *vastListMsg);
        void handlePing(VastMessage *vastMsg);
        void handlePong(VastMessage *vastMsg);
        void handleDiscardNode(VastDiscardMessage *vastMsg);
        void sendDiscardNode(VastMessage *vastMsg);
        void synchronizeApp(VastMoveMessage *vastMoveMsg = NULL);
};

#endif
