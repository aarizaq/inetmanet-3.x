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
 * @file Quon.h
 * @author Helge Backhaus, Stephan Krause
 * @author Stephan Krause
 */

#ifndef __QUON_H_
#define __QUON_H_

#include <BaseOverlay.h>
#include <GlobalNodeList.h>
#include <GlobalStatistics.h>
#include <float.h>

#include <Quon_m.h>
#include <QuonHelper.h>

/**
 * QuON: An overlay network based on quadtrees.
 *
 * @author Helge Backhaus, Stephan Krause
 */
class Quon : public BaseOverlay
{
    public:
        // OMNeT++
        ~Quon();
        void initializeOverlay(int stage);
        void finishOverlay();
        void handleUDPMessage(BaseOverlayMessage* msg);
        void handleTimerEvent(cMessage* msg);
        void handleAppMessage(cMessage* msg);
        void handleNodeGracefulLeaveNotification();

        QState getState();
        double getAOI();
        Vector2D getPosition();
        double getAreaDimension();
        OverlayKey getKey();
        long getSoftNeighborCount();

        QuonSiteMap Sites;

    private:
        // parameters
        simtime_t joinTimeout;
        simtime_t deleteTimeout;
        simtime_t aliveTimeout;
        double AOIWidth;
        double minAOI;
        double maxAOI;
        unsigned int connectionLimit;
        double areaDimension;
        simtime_t backupIntervall;
        bool useDynamicAOI;
        bool useSquareMetric;

        bool linearAdaption;
        double adaptionSensitivity;
        double gossipSensitivity;

        // timers
        cMessage* join_timer;
        cMessage* sec_timer;
        cMessage* alive_timer;
        cMessage* backup_timer;

        void sendToApp(cMessage* msg);
        void sendMessage(QuonMessage* quonMsg, NodeHandle destination);
        void setBootstrapedIcon();
        void changeState(QState qstate);

        // timer processing
        void processJoinTimer();
        void processSecTimer();
        void processDeleteTimer(cMessage* msg);
        void processAliveTimer();
        void processBackupTimer();

        // app handlers
        void handleJoin(GameAPIPositionMessage* gameMsg);
        void handleMove(GameAPIPositionMessage* gameMsg);
        void handleEvent(GameAPIMessage* msg);

        // overlay handlers
        void handleJoinRequest(QuonMessage* quonMsg);
        void handleJoinAcknowledge(QuonListMessage* quonListMsg);
        void handleNodeMove(QuonMoveMessage* quonMoveMsg);
        void handleNewNeighbors(QuonListMessage* quonListMsg);
        void handleNodeLeave(QuonListMessage* quonListMsg);
        void handleInvalidNode(QuonListMessage* quonListMsg);

        // app<->overlay synchronisation
        void synchronizeAppNeighbors(QPurgeType purgeSoftSites = QKEEPSOFT);
        void deleteAppNeighbor(NodeHandle node);

        // statistics
        double joinRequestBytesSend;
        double joinAcknowledgeBytesSend;
        double nodeMoveBytesSend;
        double newNeighborsBytesSend;
        double nodeLeaveBytesSend;
        double maxBytesPerSecondSend;
        double averageBytesPerSecondSend;
        double bytesPerSecond;
        long softConnections;
        long softNeighborCount;
        long bindingNeighborCount;
        long directNeighborCount;
        unsigned int secTimerCount;
        long rejoinCount;
        unsigned long avgAOI;

        // quon functions
        bool addSite(Vector2D p, NodeHandle node, double AOI, bool isSoft = false, QUpdateType update = QFOREIGN);
        void updateThisSite(Vector2D p);
        void classifySites();
        bool deleteSite(NodeHandle node);
        int purgeSites(QPurgeType purgeSoftSites = QKEEPSOFT);
        void adaptAoI();

        // node references
        QuonSite* thisSite;
        QDeleteMap deletedSites;
        NodeHandle (*bindingBackup)[4];
        int numBackups;

        // node state
        QState qstate;
        bool aloneInOverlay;
};

#endif
