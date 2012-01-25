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
 * @file Quon.cc
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#include <Quon.h>
#include <BootstrapList.h>
#include <limits>

Define_Module(Quon);

void Quon::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if(stage != MIN_STAGE_OVERLAY) {
        return;
    }

    // fetch parameters
    minAOI = (double)par("minAOIWidth") + (double)par("AOIBuffer"); // FIXME: use buffer only where required
    maxAOI = (double)par("AOIWidth") + (double)par("AOIBuffer"); // FIXME: use buffer only where required
    AOIWidth = maxAOI;
    connectionLimit = par("connectionLimit");
    areaDimension = par("areaDimension");
    joinTimeout = par("joinTimeout");
    deleteTimeout = par("deleteTimeout");
    aliveTimeout = (double)par("aliveTimeout") / 2.0;
    backupIntervall = par("contactBackupIntervall");
    numBackups = par("numBackups");
    linearAdaption = par("AOIAdaptLinear");
    adaptionSensitivity = par("AOIAdaptionSensitivity");
    gossipSensitivity = par("AOIGossipSensitivity");
    useSquareMetric = par("useSquareMetric");

    bindingBackup = new NodeHandle[numBackups][4];

    // determine wether we want dynamic AOI or not
    useDynamicAOI = connectionLimit > 0 && minAOI < maxAOI;

    // set node key and thisSite pointer
    thisNode.setKey(OverlayKey::random());
    thisSite = new QuonSite();
    thisSite->address = thisNode;
    thisSite->type = QTHIS;

    // initialize self-messages
    join_timer = new cMessage("join_timer");
    sec_timer = new cMessage("sec_timer");
    alive_timer = new cMessage("alive_timer");
    backup_timer = new cMessage("backup_timer");

    // statistics
    joinRequestBytesSend = 0.0;
    joinAcknowledgeBytesSend = 0.0;
    nodeMoveBytesSend = 0.0;
    newNeighborsBytesSend = 0.0;
    nodeLeaveBytesSend = 0.0;
    maxBytesPerSecondSend = 0.0;
    averageBytesPerSecondSend = 0.0;
    bytesPerSecond = 0.0;
    softConnections = 0;
    softNeighborCount = 0;
    bindingNeighborCount = 0;
    directNeighborCount = 0;
    secTimerCount = 0;
    //rejoinCount = 0;
    avgAOI= 0 ;

    // watch some variables
    WATCH(thisSite->address);
    WATCH(thisSite->position);
    WATCH(AOIWidth);
    //WATCH_POINTER_MAP(Sites);
    //WATCH_POINTER_MAP(deletedSites);
    //WATCH_SET(Positions);
    WATCH(joinRequestBytesSend);
    WATCH(joinAcknowledgeBytesSend);
    WATCH(nodeMoveBytesSend);
    WATCH(newNeighborsBytesSend);
    WATCH(nodeLeaveBytesSend);
    WATCH(maxBytesPerSecondSend);
    WATCH(bytesPerSecond);
    WATCH(softConnections);
    WATCH(softNeighborCount);
    WATCH(bindingNeighborCount);
    WATCH(directNeighborCount);
    //WATCH(rejoinCount);

    // set initial state
    aloneInOverlay = false;
    changeState(QUNINITIALIZED);
    changeState(QJOINING);
}

void Quon::changeState(QState qstate)
{
    this->qstate = qstate;
    switch(qstate) {
        case QUNINITIALIZED:
            globalNodeList->removePeer(thisSite->address);
            cancelEvent(join_timer);
            cancelEvent(sec_timer);
            cancelEvent(alive_timer);
            cancelEvent(backup_timer);
            break;
        case QJOINING:
            scheduleAt(simTime(), join_timer);
            scheduleAt(simTime() + 1.0, sec_timer);
            break;
        case QREADY:
            cancelEvent(join_timer);
            globalNodeList->registerPeer(thisSite->address);
            // tell the application we are ready unless we are rejoining the overlay
            //if(rejoinCount == 0) {
            CompReadyMessage* readyMsg = new CompReadyMessage("OVERLAY_READY");
            readyMsg->setReady(true);
            readyMsg->setComp(getThisCompType());
            // TODO/FIXME: use overlay->sendMessageToAllComp(msg, getThisCompType())?
            sendToApp(readyMsg);
            //}
            // set initial AOI size
            AOIWidth = maxAOI;
            GameAPIResizeAOIMessage* gameMsg = new GameAPIResizeAOIMessage("RESIZE_AOI");
            gameMsg->setCommand(RESIZE_AOI);
            gameMsg->setAOIsize(AOIWidth);
            sendToApp(gameMsg);
            if(aliveTimeout > 0.0) {
                scheduleAt(simTime() + aliveTimeout, alive_timer);
            }
            if(backupIntervall > 0.0) {
                scheduleAt(simTime() + backupIntervall, backup_timer);
            }
            break;
    }
    setBootstrapedIcon();
    // debug output
    if(debugOutput) {
        EV << "[Quon::changeState() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Node " << thisSite->address.getIp() << " entered ";
        switch(qstate) {
            case QUNINITIALIZED:
                EV << "UNINITIALIZED";
                break;
            case QJOINING:
                EV << "JOINING";
                break;
            case QREADY:
                EV << "READY";
                break;
        }
        EV << " state." << endl;
    }
}

void Quon::handleTimerEvent(cMessage* msg)
{
    if(msg->isName("join_timer")) {
        //reset timer
        cancelEvent(join_timer);
        if(qstate != QREADY) {
            scheduleAt(simTime() + joinTimeout, msg);
            // handle event
            processJoinTimer();
        }
    }
    else if(msg->isName("sec_timer")) {
        //reset timer
        cancelEvent(sec_timer);
        scheduleAt(simTime() + 1, msg);
        // handle event
        processSecTimer();
    }
    else if(msg->isName("delete_timer")) {
        // handle event
        processDeleteTimer(msg);
    }
    else if(msg->isName("alive_timer")) {
        //reset timer
        cancelEvent(alive_timer);
        scheduleAt(simTime() + aliveTimeout, msg);
        // handle event
        processAliveTimer();
    }
    else if(msg->isName("backup_timer")) {
        //reset timer
        cancelEvent(backup_timer);
        scheduleAt(simTime() + backupIntervall, msg);
        // handle event
        processBackupTimer();
    }
}

void Quon::handleAppMessage(cMessage* msg)
{
    GameAPIMessage* gameAPIMsg = dynamic_cast<GameAPIMessage*>(msg);
    if(gameAPIMsg != NULL) {
        // debug output
        if(debugOutput) {
            EV << "[Quon::handleAppMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Node " << thisSite->address.getIp() << " received " << gameAPIMsg->getName() << " from application."
               << endl;
        }
        switch(gameAPIMsg->getCommand()) {
            case MOVEMENT_INDICATION: {
                GameAPIPositionMessage* gameAPIPositionMsg = dynamic_cast<GameAPIPositionMessage*>(msg);
                if(qstate == QJOINING) {
                    handleJoin(gameAPIPositionMsg);
                }
                else if(qstate == QREADY) {
                    handleMove(gameAPIPositionMsg);
                }
            } break;
            case GAMEEVENT_CHAT:
            case GAMEEVENT_SNOW:
            case GAMEEVENT_FROZEN: {
                handleEvent(gameAPIMsg);
            } break;
        }
    }
    delete msg;
}

void Quon::handleUDPMessage(BaseOverlayMessage* msg)
{
    if(qstate == QUNINITIALIZED) {
        delete msg;
        return;
    }
    QuonMessage* quonMsg = dynamic_cast<QuonMessage*>(msg);
    if(quonMsg != NULL) {
        // debug output
        if(debugOutput) {
            EV << "[Quon::handleUDPMessage() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Node " << thisSite->address.getIp() << " received " << quonMsg->getName() << " from " << quonMsg->getSender().getIp() << "."
               << endl;
        }
        if(qstate == QREADY) {
            switch(quonMsg->getCommand()) {
                case JOIN_REQUEST: {
                    handleJoinRequest(quonMsg);
                } break;
                case NODE_MOVE: {
                    QuonMoveMessage* quonMoveMsg = dynamic_cast<QuonMoveMessage*>(msg);
                    handleNodeMove(quonMoveMsg);
                    delete msg;
                } break;
                case NEW_NEIGHBORS: {
                    QuonListMessage* quonListMsg = dynamic_cast<QuonListMessage*>(msg);
                    handleNewNeighbors(quonListMsg);
                    delete msg;
                } break;
                case NODE_LEAVE: {
                    QuonListMessage* quonListMsg = dynamic_cast<QuonListMessage*>(msg);
                    handleNodeLeave(quonListMsg);
                    delete msg;
                } break;
                case QUON_EVENT: {
                    sendToApp(quonMsg->decapsulate());
                    delete quonMsg;
                } break;
            }
        }
        else if(qstate == QJOINING && quonMsg->getCommand() == JOIN_ACKNOWLEDGE) {
            QuonListMessage* quonListMsg = dynamic_cast<QuonListMessage*>(msg);
            handleJoinAcknowledge(quonListMsg);
            delete msg;
        }
        else {
            delete msg;
        }
    }
    else {
        delete msg;
    }
}

bool Quon::addSite(Vector2D p, NodeHandle node, double AOI, bool isSoft, QUpdateType update)
{
    aloneInOverlay = false;
    QuonSiteMap::iterator itSites = Sites.find(node.getKey());
    QDeleteMap::iterator delIt = deletedSites.find(node.getKey());
    // add node if he is not in the delete list OR has changed position since 
    // put in the delete list. don't add node if he has signled his leave himself
    // (i.e. his position in the delete list is 0,0)
    if(node.getKey() != thisSite->address.getKey() && 
            (delIt == deletedSites.end() || (delIt->second != Vector2D(0,0) && delIt->second != p) )){
        if(itSites == Sites.end()) {
            if(debugOutput) {
                EV << "[Quon::addSite() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Site " << node.getIp() << " at " << p << " has been added to the list."
                   << endl;
            }
            QuonSite* temp = new QuonSite();
            temp->position = p;
            temp->address = node;
            if(update == QDIRECT) {
                temp->dirty = true;
            }
            temp->alive = true;
            temp->type = QUNDEFINED;
            temp->softNeighbor = isSoft;
            temp->AOIwidth = AOI;

            Sites.insert(std::make_pair(temp->address.getKey(), temp));
        }
        else if(update == QDIRECT || !itSites->second->alive) {
            if(debugOutput) {
                EV << "[Quon::addSite() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Site " << node.getIp() << " at " << p << " has been updated in the list."
                   << endl;
            }
            itSites->second->position = p;
            itSites->second->dirty = true;
            itSites->second->alive = true;
            itSites->second->softNeighbor = isSoft;
            itSites->second->type = QUNDEFINED;
            itSites->second->AOIwidth = AOI;
        }
        return true;
    }
    return false;
}

void Quon::updateThisSite(Vector2D p)
{
    if(debugOutput) {
        EV << "[Quon::updateThisSite() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    This Site position has been updated to " << p << "."
           << endl;
    }
    thisSite->position = p;
}

void Quon::classifySites()
{
    if(Sites.size() > 0) {
        QuonAOI AOI(thisSite->position, AOIWidth, useSquareMetric);
        QuonSite* (*bindingCandidates)[4] = new QuonSite*[numBackups+1][4];
        for( int i = 0; i <= numBackups; ++i ){
            for( int ii = 0; ii < 4; ++ii ){
                bindingCandidates[i][ii] = 0;
            }
        }
        double (*bindingDistance)[4] = new double[numBackups+1][4];
        for( int i = 0; i <= numBackups; ++i ){
            for( int ii = 0; ii < 4; ++ii ){
                bindingDistance[i][ii] = std::numeric_limits<double>::infinity();
            }
        }

        for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            QuonAOI NeighborAOI(itSites->second->position, itSites->second->AOIwidth, useSquareMetric);
            if(AOI.collide(itSites->second->position) || NeighborAOI.collide(thisSite->position)) {
                if(itSites->second->type != QNEIGHBOR) {
                    itSites->second->type = QNEIGHBOR;
                    itSites->second->dirty = true;
                }
            }
            else if(itSites->second->type != QUNDEFINED) {
                itSites->second->type = QUNDEFINED;
                itSites->second->dirty = true;
            }
            int quad = thisSite->position.getQuadrant( itSites->second->position );
            double dist;
            if( useSquareMetric ) {
                dist = thisSite->position.xyMaxDistance(itSites->second->position);
            } else {
                dist = thisSite->position.distanceSqr(itSites->second->position);
            }

            // if dist is smaller than the most far binding candidate
            if( dist < bindingDistance[numBackups][quad] ){
            // Go through list of binding candidates until distance to current candidate
            // is greater than distance to new candidate (i.e. look where in the binding
            // candidate list the node belongs)
                int backupPos = numBackups-1;
                while( backupPos >= 0 && dist < bindingDistance[backupPos][quad] ){
                    // move old candidate one position back in the queue to make
                    // room for new candidate
                    bindingCandidates[backupPos+1][quad] = bindingCandidates[backupPos][quad];
                    bindingDistance[backupPos+1][quad] = bindingDistance[backupPos][quad];
                    --backupPos;
                }
                // place new candidate at appropriate position in candidate list
                bindingCandidates[backupPos+1][quad] = itSites->second;
                bindingDistance[backupPos+1][quad] = dist;
            }

        }
        for( int i = 0; i < 4; ++i ){
            if( bindingCandidates[0][i] ){
                bindingCandidates[0][i]->type = QBINDING;
                bindingCandidates[0][i]->dirty = true;
                for( int ii = 1; ii <= numBackups; ++ii ){
                    if( bindingCandidates[ii][i] ){
                        bindingBackup[ii-1][i] = bindingCandidates[ii][i]->address;
                    }
                }
            }
        }

        delete[] bindingCandidates;
        delete[] bindingDistance;
    }
    else {
        if( !aloneInOverlay) {
            ++rejoinCount;

            changeState(QUNINITIALIZED);
            changeState(QJOINING);
        }
    }
}

bool Quon::deleteSite(NodeHandle node)
{
    QuonSiteMap::iterator itSites = Sites.find(node.getKey());
    if(itSites != Sites.end()) {
        if(debugOutput) {
            EV << "[Quon::deleteSite() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Site " << node.getIp() << " at " << itSites->second->position << " has been removed from the list."
               << endl;
        }
        delete itSites->second;
        Sites.erase(itSites);
        return true;
    }
    return false;
}

int Quon::purgeSites(QPurgeType purgeSoftSites)
{
    int purged = 0;
    QuonSiteMap::iterator itSites = Sites.begin();
    while(itSites != Sites.end()) {
        // Purge softNeighbors only if QPURGESOFT is set
        if(itSites->second->type == QUNDEFINED && ( purgeSoftSites == QPURGESOFT || !itSites->second->softNeighbor) ) {
            if(debugOutput) {
                EV << "[Quon::purgeSites() @ " << thisNode.getIp()
                   << " (" << thisNode.getKey().toString(16) << ")]\n"
                   << "    Site " << itSites->second->address.getIp() << " at " << itSites->second->position << " has been removed from the list.\n"
                   << "    Status: " << ((itSites->second->type == QUNDEFINED) ? "QUNDEFINED" : "QSOFT")
                   << endl;
            }
            delete itSites->second;
            Sites.erase(itSites++);
            ++purged;
        }
        else {
            ++itSites;
        }
    }
    return purged;
}

void Quon::adaptAoI()
{
    // adjust AOIWidth
    double oldAOI = AOIWidth;
    if( linearAdaption ) {
        AOIWidth -= (maxAOI - minAOI) * ((double) Sites.size() - (double) connectionLimit) * adaptionSensitivity / (double) connectionLimit;
    } else if( Sites.size() > 0 ){
        AOIWidth *= (1-adaptionSensitivity) + (double) connectionLimit * adaptionSensitivity / (double) Sites.size();
    }
    if( gossipSensitivity > 0  && Sites.size() > 0 ) {
        double avgNeighborAOI = 0;
        for( QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites ){
            avgNeighborAOI += itSites->second->AOIwidth;
        }
        avgNeighborAOI /= Sites.size();
        AOIWidth = AOIWidth*(1-gossipSensitivity) + avgNeighborAOI*gossipSensitivity;
    }
    if(AOIWidth > maxAOI) {
        AOIWidth = maxAOI;
    }
    else if(AOIWidth < minAOI) {
        AOIWidth = minAOI;
    }
    
    if( oldAOI != AOIWidth ){
        GameAPIResizeAOIMessage* gameMsg = new GameAPIResizeAOIMessage("RESIZE_AOI");
        gameMsg->setCommand(RESIZE_AOI);
        gameMsg->setAOIsize(AOIWidth);
        sendToApp(gameMsg);
    }
}

void Quon::handleNodeGracefulLeaveNotification()
{
    if(qstate == QREADY) {
        CompReadyMessage* readyMsg = new CompReadyMessage("OVERLAY_FINISHED");
        readyMsg->setReady(false);
        readyMsg->setComp(getThisCompType());
        // TODO/FIXME: use overlay->sendMessageToAllComp(msg, getThisCompType())?
        sendToApp(readyMsg);
        if(Sites.size() > 0) {
            // generate node leave messages
            QuonListMessage* quonListMsg = new QuonListMessage("NODE_LEAVE");
            quonListMsg->setCommand(NODE_LEAVE);
            quonListMsg->setSender(thisSite->address);
            quonListMsg->setPosition(thisSite->position);
            quonListMsg->setAOIsize(AOIWidth);
            // fill neighbors list
            quonListMsg->setNeighborHandleArraySize(Sites.size());
            quonListMsg->setNeighborPositionArraySize(Sites.size());
            int i = 0;
            for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
                quonListMsg->setNeighborHandle(i, itSites->second->address);
                quonListMsg->setNeighborPosition(i, itSites->second->position);
                ++i;
            }
            quonListMsg->setBitLength(QUONLIST_L(quonListMsg));

            for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
                QuonListMessage* quonCopyMsg = new QuonListMessage(*quonListMsg);
                sendMessage(quonCopyMsg, itSites->second->address);
            }
            delete quonListMsg;
        }
        changeState(QUNINITIALIZED);
    }
}

void Quon::processJoinTimer()
{
    GameAPIMessage* gameMsg = new GameAPIMessage("MOVEMENT_REQUEST");
    gameMsg->setCommand(MOVEMENT_REQUEST);
    sendToApp(gameMsg);
}

void Quon::processSecTimer()
{
    RECORD_STATS(
        if(bytesPerSecond > maxBytesPerSecondSend) {
            maxBytesPerSecondSend = bytesPerSecond;
        }
        avgAOI += AOIWidth;
        averageBytesPerSecondSend += bytesPerSecond;
        for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            switch(itSites->second->type) {
                case QNEIGHBOR:
                    directNeighborCount++;
                    break;
                case QBINDING:
                    bindingNeighborCount++;
                    break;
                case QUNDEFINED:
                    if( itSites->second->softNeighbor ){
                        softNeighborCount++;
                    }
                    break;
                case QTHIS:
                    break;
            }
        }
        ++secTimerCount;
    );
    bytesPerSecond = 0.0;
}

void Quon::processDeleteTimer(cMessage* msg)
{
    QuonSelfMessage* quonMsg = dynamic_cast<QuonSelfMessage*>(msg);
    QDeleteMap::iterator itSite = deletedSites.find(quonMsg->getKey());
    if(itSite != deletedSites.end()) {
        deletedSites.erase(itSite);
    }
    cancelAndDelete(quonMsg);
}

void Quon::processAliveTimer()
{
    bool rebuild = false;
    QuonSiteMap::iterator itSites = Sites.begin();
    while(itSites != Sites.end()) {
        if(itSites->second->alive) {
            itSites->second->alive = false;
            ++itSites;
        }
        else {
            NodeHandle node = itSites->second->address;
            QuonSelfMessage* msg = new QuonSelfMessage("delete_timer");
            msg->setKey(node.getKey());
            scheduleAt(simTime() + deleteTimeout, msg);
            deletedSites.insert(std::make_pair(node.getKey(), itSites->second->position));
            ++itSites;
            deleteSite(node);
            // update simple client
            deleteAppNeighbor(node);
            if(!rebuild) {
                rebuild = true;
            }
        }
    }
    if(rebuild) {
        classifySites();
        // update simple client
        synchronizeAppNeighbors();
        purgeSites();
    }
}

void Quon::processBackupTimer()
{
    QuonMoveMessage* quonMoveMsg = new QuonMoveMessage("NODE_MOVE");
    quonMoveMsg->setCommand(NODE_MOVE);
    quonMoveMsg->setSender(thisSite->address);
    quonMoveMsg->setPosition(thisSite->position);
    quonMoveMsg->setAOIsize(AOIWidth);
    quonMoveMsg->setNewPosition(thisSite->position);
    quonMoveMsg->setIsBinding(true);
    for(unsigned int i=0; i<4; i++) {
        for( int ii = 0; ii < numBackups; ++ii ){
            if(!bindingBackup[ii][i].isUnspecified()) {
                QuonMoveMessage* copyMsg = new QuonMoveMessage(*quonMoveMsg);
                copyMsg->setBitLength(QUONMOVE_L(copyMsg));
                sendMessage(copyMsg, bindingBackup[ii][i]);
            }
        }
    }
    delete quonMoveMsg;
}

void Quon::handleJoin(GameAPIPositionMessage* gameMsg)
{
    TransportAddress joinNode = bootstrapList->getBootstrapNode();
    thisSite->position = gameMsg->getPosition();
    // check if this is the only node in the overlay
    if(joinNode.isUnspecified()) {
        changeState(QREADY);
        aloneInOverlay = true;
    }
    else {
        QuonMessage* quonMsg = new QuonMessage("JOIN_REQUEST");
        quonMsg->setCommand(JOIN_REQUEST);
        quonMsg->setSender(thisSite->address);
        quonMsg->setPosition(thisSite->position);
        quonMsg->setAOIsize(AOIWidth);
        quonMsg->setBitLength(QUON_L(quonMsg));
        sendMessage(quonMsg, joinNode);
    }
}

void Quon::handleMove(GameAPIPositionMessage* gameMsg)
{
    // adapt aoi
    if(useDynamicAOI) {
        adaptAoI();
        classifySites();
    }

    Vector2D position = gameMsg->getPosition();
    // send position update to neighbors
    QuonMoveMessage* quonMoveMsg = new QuonMoveMessage("NODE_MOVE");
    quonMoveMsg->setCommand(NODE_MOVE);
    quonMoveMsg->setSender(thisSite->address);
    quonMoveMsg->setPosition(thisSite->position);
    quonMoveMsg->setAOIsize(AOIWidth);
    quonMoveMsg->setNewPosition(position);
    quonMoveMsg->setBitLength(QUONMOVE_L(quonMoveMsg));

    QuonMoveMessage* quonMoveBindingMsg = new QuonMoveMessage(*quonMoveMsg);
    quonMoveBindingMsg->setNeighborHandleArraySize(Sites.size());
    quonMoveBindingMsg->setNeighborPositionArraySize(Sites.size());
    int i = 0;
    for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
      if(itSites->second->type == QBINDING || itSites->second->softNeighbor ) {
        quonMoveBindingMsg->setNeighborHandle(i, itSites->second->address);
        quonMoveBindingMsg->setNeighborPosition(i, itSites->second->position);
        ++i;
      }
    }
    quonMoveBindingMsg->setNeighborHandleArraySize(i);
    quonMoveBindingMsg->setNeighborPositionArraySize(i);
    if(i > 0) {
      // speedhack:
      // instead of building individual MoveMessages for every binding and softstate neighbor,
      // we just send all binding/soft to every other binding/soft neighbor and pretend we did not send  neighbors their own neighborslistentry
      quonMoveBindingMsg->setBitLength(QUONMOVE_L(quonMoveBindingMsg) - QUONENTRY_L);
    }

    for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
      QuonMoveMessage* copyMsg;
      if(itSites->second->type == QBINDING || itSites->second->softNeighbor ) {
        copyMsg = new QuonMoveMessage(*quonMoveBindingMsg);
        if(itSites->second->type == QBINDING) {
          copyMsg->setIsBinding(true);
        }
        else {
          ++softConnections;
        }
      }
      else {
        copyMsg = new QuonMoveMessage(*quonMoveMsg);
      }
      sendMessage(copyMsg, itSites->second->address);
    }
    delete quonMoveMsg;
    delete quonMoveBindingMsg;

    // update position
    updateThisSite(position);
    classifySites();
    // update simple client
    synchronizeAppNeighbors(QPURGESOFT);
    purgeSites(QPURGESOFT);
}

void Quon::handleEvent(GameAPIMessage* msg)
{
    // send event to neighbors
    for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        QuonEventMessage *quonMsg = new QuonEventMessage("EVENT");
        quonMsg->setCommand(QUON_EVENT);
        quonMsg->encapsulate((cPacket*)msg->dup());
        // FIXME: Message length!
        sendMessage(quonMsg, itSites->second->address);
    }
}

void Quon::handleJoinRequest(QuonMessage* quonMsg)
{
    Vector2D joinPosition = quonMsg->getPosition();
    // start with this node
    double min_dist = thisSite->position.distanceSqr(joinPosition);
    QuonSite* forwardSite = thisSite;
    // iterate through all neighbors
    for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        if(itSites->second->position.distanceSqr(joinPosition) < min_dist) { //FIXME: use xy metric if desired?
            min_dist = itSites->second->position.distanceSqr(joinPosition);
            forwardSite = itSites->second;
        }
    }

    // do nothing and let node retry with new position if current position is illegal
    if(min_dist == 0.0) {
        delete quonMsg;
    }
    else if(forwardSite->type == QTHIS) {
        QuonListMessage* quonListMsg = new QuonListMessage("JOIN_ACKNOWLEDGE");
        quonListMsg->setCommand(JOIN_ACKNOWLEDGE);
        quonListMsg->setSender(thisSite->address);
        quonListMsg->setPosition(thisSite->position);
        quonListMsg->setAOIsize(AOIWidth);
        // fill neighbors list
        quonListMsg->setNeighborHandleArraySize(Sites.size());
        quonListMsg->setNeighborPositionArraySize(Sites.size());
        int i = 0;
        for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            quonListMsg->setNeighborHandle(i, itSites->second->address);
            quonListMsg->setNeighborPosition(i, itSites->second->position);
            ++i;
        }
        quonListMsg->setNeighborHandleArraySize(i);
        quonListMsg->setNeighborPositionArraySize(i);

        quonListMsg->setBitLength(QUONLIST_L(quonListMsg));
        sendMessage(quonListMsg, quonMsg->getSender());
        delete quonMsg;
    }
    else {
        sendMessage(quonMsg, forwardSite->address);
    }
}

void Quon::handleJoinAcknowledge(QuonListMessage* quonListMsg)
{
    // add acceptor node
    changeState(QREADY);
    addSite(quonListMsg->getPosition(), quonListMsg->getSender(), quonListMsg->getAOIsize(), false, QDIRECT);
    // add new neighbors
    for(unsigned int i=0; i<quonListMsg->getNeighborHandleArraySize(); i++) {
        addSite(quonListMsg->getNeighborPosition(i), quonListMsg->getNeighborHandle(i), quonListMsg->getAOIsize());
    }
    classifySites();
    // update simple client
    synchronizeAppNeighbors();
    purgeSites();
    // contact new neighbors
    for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        QuonMoveMessage* quonMoveMsg = new QuonMoveMessage("NODE_MOVE");
        quonMoveMsg->setCommand(NODE_MOVE);
        quonMoveMsg->setSender(thisSite->address);
        quonMoveMsg->setPosition(quonListMsg->getPosition());
        quonMoveMsg->setAOIsize(AOIWidth);
        quonMoveMsg->setNewPosition(thisSite->position);
        if(itSites->second->type == QBINDING) {
            quonMoveMsg->setIsBinding(true);
        }
        quonMoveMsg->setBitLength(QUONMOVE_L(quonMoveMsg));
        sendMessage(quonMoveMsg, itSites->second->address);
    }
    bytesPerSecond = 0.0;
}

void Quon::handleNodeMove(QuonMoveMessage* quonMoveMsg)
{
    RECORD_STATS(
            globalStatistics->addStdDev(
                "QuON: MoveDelay",
                SIMTIME_DBL(simTime()) - SIMTIME_DBL(quonMoveMsg->getCreationTime())
                );
            );

    // IF node was marked for deletetion, remove it from the delete list
    QDeleteMap::iterator delIt = deletedSites.find(quonMoveMsg->getSender().getKey());
    if( delIt != deletedSites.end() ){
        deletedSites.erase( delIt );
    }

    // Compute old and new AOI of moving node
    QuonAOI oldAOI(quonMoveMsg->getPosition(), quonMoveMsg->getAOIsize(), useSquareMetric);
    QuonAOI newAOI(quonMoveMsg->getNewPosition(), quonMoveMsg->getAOIsize(), useSquareMetric);
    if(useDynamicAOI) {
        QuonSiteMap::iterator itSites = Sites.find(quonMoveMsg->getSender().getKey());
        if(itSites != Sites.end() && itSites->second->AOIwidth < quonMoveMsg->getAOIsize()) {
            oldAOI.resize(itSites->second->AOIwidth);
        }
    }

    addSite(quonMoveMsg->getNewPosition(), quonMoveMsg->getSender(), quonMoveMsg->getAOIsize(), quonMoveMsg->getIsBinding(), QDIRECT);
    // add new neighbors
    handleInvalidNode(quonMoveMsg);
    for(unsigned int i=0; i<quonMoveMsg->getNeighborHandleArraySize(); i++) {
        addSite(quonMoveMsg->getNeighborPosition(i), quonMoveMsg->getNeighborHandle(i), quonMoveMsg->getAOIsize());
    }
    classifySites();
    // update simple client
    synchronizeAppNeighbors();
    purgeSites();

    // send new neighbors
    QuonListMessage* quonListMsg = new QuonListMessage("NEW_NEIGHBORS");
    quonListMsg->setCommand(NEW_NEIGHBORS);
    quonListMsg->setSender(thisSite->address);
    quonListMsg->setPosition(thisSite->position);
    quonListMsg->setAOIsize(AOIWidth);

    quonListMsg->setNeighborHandleArraySize(Sites.size());
    quonListMsg->setNeighborPositionArraySize(Sites.size());

    int i = 0;
    for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        if(quonMoveMsg->getSender() != itSites->second->address &&
           !oldAOI.collide(itSites->second->position) &&
           newAOI.collide(itSites->second->position)) {
            quonListMsg->setNeighborHandle(i, itSites->second->address);
            quonListMsg->setNeighborPosition(i, itSites->second->position);
            ++i;
        }
    }

    if(i > 0) {
        quonListMsg->setNeighborHandleArraySize(i);
        quonListMsg->setNeighborPositionArraySize(i);
        quonListMsg->setBitLength(QUONLIST_L(quonListMsg));
        sendMessage(quonListMsg, quonMoveMsg->getSender());
    }
    else {
        delete quonListMsg;
    }
}

void Quon::handleNewNeighbors(QuonListMessage* quonListMsg)
{
    addSite(quonListMsg->getPosition(), quonListMsg->getSender(), quonListMsg->getAOIsize(), false, QDIRECT);

    // add new neighbors
    handleInvalidNode(quonListMsg);
    for(unsigned int i=0; i<quonListMsg->getNeighborHandleArraySize(); i++) {
        addSite(quonListMsg->getNeighborPosition(i), quonListMsg->getNeighborHandle(i), quonListMsg->getAOIsize());
    }
    classifySites();
    // update simple client
    synchronizeAppNeighbors();
    purgeSites();
}

void Quon::handleNodeLeave(QuonListMessage* quonListMsg)
{
    deleteSite(quonListMsg->getSender());
    // update simple client
    deleteAppNeighbor(quonListMsg->getSender());

    // insert into delete list
    QuonSelfMessage* msg = new QuonSelfMessage("delete_timer");
    msg->setKey(quonListMsg->getSender().getKey());
    scheduleAt(simTime() + deleteTimeout, msg);
    deletedSites.insert(std::make_pair(quonListMsg->getSender().getKey(), Vector2D(0,0)));

    // add possible new neighbors
    handleInvalidNode(quonListMsg);
    for(unsigned int i=0; i<quonListMsg->getNeighborHandleArraySize(); i++) {
        addSite(quonListMsg->getNeighborPosition(i), quonListMsg->getNeighborHandle(i), quonListMsg->getAOIsize(), true);
    }
    classifySites();
    // update simple client
    synchronizeAppNeighbors();
    purgeSites();
}

// XXX FIXME Disabled, may cause exessive traffic
void Quon::handleInvalidNode(QuonListMessage* quonListMsg)
{
    return;
    for(unsigned int i=0; i<quonListMsg->getNeighborHandleArraySize(); i++) {
        if(deletedSites.find(quonListMsg->getNeighborHandle(i).getKey()) != deletedSites.end()) {
            QuonListMessage* quonLeaveMsg = new QuonListMessage("NODE_LEAVE");
            quonLeaveMsg->setCommand(NODE_LEAVE);
            quonLeaveMsg->setSender(quonListMsg->getNeighborHandle(i));
            quonLeaveMsg->setPosition(quonListMsg->getNeighborPosition(i));
            quonLeaveMsg->setAOIsize(AOIWidth);
            quonLeaveMsg->setNeighborHandleArraySize(Sites.size());
            quonLeaveMsg->setNeighborPositionArraySize(Sites.size());
            int i = 0;
            for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
                if(itSites->second->type == QBINDING) {
                    quonLeaveMsg->setNeighborHandle(i, itSites->second->address);
                    quonLeaveMsg->setNeighborPosition(i, itSites->second->position);
                    ++i;
                }
            }
            quonLeaveMsg->setNeighborHandleArraySize(i);
            quonLeaveMsg->setNeighborPositionArraySize(i);
            quonLeaveMsg->setBitLength(QUONLIST_L(quonLeaveMsg));
            sendMessage(quonLeaveMsg, quonListMsg->getSender());
        }
    }
}

void Quon::synchronizeAppNeighbors(QPurgeType purgeSoftSites)
{
    GameAPIListMessage* gameMsg = new GameAPIListMessage("NEIGHBOR_UPDATE");
    gameMsg->setCommand(NEIGHBOR_UPDATE);

    gameMsg->setRemoveNeighborArraySize(Sites.size());
    gameMsg->setAddNeighborArraySize(Sites.size());
    gameMsg->setNeighborPositionArraySize(Sites.size());

    int remSize, addSize;
    remSize = addSize = 0;
    for(QuonSiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        if(itSites->second->type == QUNDEFINED && (purgeSoftSites == QPURGESOFT || !itSites->second->softNeighbor) && itSites->second->dirty) {
            gameMsg->setRemoveNeighbor(remSize, itSites->second->address);
            ++remSize;
        }
        else if(itSites->second->dirty) {
            gameMsg->setAddNeighbor(addSize, itSites->second->address);
            gameMsg->setNeighborPosition(addSize, itSites->second->position);
            itSites->second->dirty = false;
            ++addSize;
        }
    }

    if(remSize > 0 || addSize > 0) {
        gameMsg->setRemoveNeighborArraySize(remSize);
        gameMsg->setAddNeighborArraySize(addSize);
        gameMsg->setNeighborPositionArraySize(addSize);
        sendToApp(gameMsg);
    }
    else {
        delete gameMsg;
    }
}

void Quon::deleteAppNeighbor(NodeHandle node)
{
    GameAPIListMessage* gameMsg = new GameAPIListMessage("NEIGHBOR_UPDATE");
    gameMsg->setCommand(NEIGHBOR_UPDATE);
    gameMsg->setRemoveNeighborArraySize(1);
    gameMsg->setAddNeighborArraySize(0);
    gameMsg->setNeighborPositionArraySize(0);
    gameMsg->setRemoveNeighbor(0, node);
    sendToApp(gameMsg);
}

void Quon::sendToApp(cMessage* msg)
{
    // debug output
    if(debugOutput) {
        EV << "[Quon::sendToApp() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Node " << thisSite->address.getIp() << " sending " << msg->getName() << " to application."
           << endl;
    }
    send(msg, "appOut");
}

void Quon::sendMessage(QuonMessage* quonMsg, NodeHandle destination)
{
    // collect statistics
    RECORD_STATS(
        switch(quonMsg->getCommand()) {
            case JOIN_REQUEST:
                joinRequestBytesSend += quonMsg->getByteLength();
            break;
            case JOIN_ACKNOWLEDGE:
                joinAcknowledgeBytesSend += quonMsg->getByteLength();
            break;
            case NODE_MOVE:
                nodeMoveBytesSend += quonMsg->getByteLength();
            break;
            case NEW_NEIGHBORS:
                newNeighborsBytesSend += quonMsg->getByteLength();
            break;
            case NODE_LEAVE:
                nodeLeaveBytesSend += quonMsg->getByteLength();
            break;
        }
        if(qstate == QREADY) {
            bytesPerSecond += quonMsg->getByteLength();
        }
    );

    // debug output
    if(debugOutput) {
        EV << "[Quon::sendMessage() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Node " << thisSite->address.getIp() << " sending " << quonMsg->getName() << " to " << destination.getIp() << "."
           << endl;
    }
    sendMessageToUDP(destination, quonMsg);
}

void Quon::setBootstrapedIcon()
{
    if(ev.isGUI()) {
        switch(qstate) {
            case QUNINITIALIZED:
                getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "red");
                getDisplayString().setTagArg("i", 1, "red");
                break;
            case QJOINING:
                getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "yellow");
                getDisplayString().setTagArg("i", 1, "yellow");
                break;
            case QREADY:
                getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "green");
                getDisplayString().setTagArg("i", 1, "green");
                break;
        }
    }
}

void Quon::finishOverlay()
{
    double overallBytesSend = joinRequestBytesSend
                            + joinAcknowledgeBytesSend
                            + nodeMoveBytesSend
                            + newNeighborsBytesSend
                            + nodeLeaveBytesSend;
    if(overallBytesSend != 0.0) {
        // collect statistics in percent
        globalStatistics->addStdDev("Quon: fraction of JOIN_REQUEST bytes sent ", joinRequestBytesSend / overallBytesSend);
        globalStatistics->addStdDev("Quon: fraction of JOIN_ACKNOWLEDGE bytes sent", joinAcknowledgeBytesSend / overallBytesSend);
        globalStatistics->addStdDev("Quon: fraction of NODE_MOVE bytes sent", nodeMoveBytesSend / overallBytesSend);
        globalStatistics->addStdDev("Quon: fraction of NEW_NEIGHBORS bytes sent", newNeighborsBytesSend / overallBytesSend);
        globalStatistics->addStdDev("Quon: fraction of NODE_LEAVE bytes sent", nodeLeaveBytesSend / overallBytesSend);
    }
    globalStatistics->addStdDev("Quon: max bytes/second sent", maxBytesPerSecondSend);

//    We use our own time count to avoid rounding errors
//    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
//    if(time != 0.0) {
    if(secTimerCount != 0) {
        globalStatistics->addStdDev("Quon: average bytes/second sent", averageBytesPerSecondSend / (double) secTimerCount);
        globalStatistics->addStdDev("Quon: average direct-neighbor count", directNeighborCount / (double) secTimerCount);
        globalStatistics->addStdDev("Quon: average binding-neighbor count", bindingNeighborCount / (double) secTimerCount);
        globalStatistics->addStdDev("Quon: average soft-neighbor count", softNeighborCount / (double) secTimerCount);
        //globalStatistics->addStdDev("Quon: average rejoin count", rejoinCount);
        globalStatistics->addStdDev("Quon: average AOI width", avgAOI / (double) secTimerCount);
    }

    changeState(QUNINITIALIZED);
}

QState Quon::getState()
{
    Enter_Method_Silent();
    return qstate;
}

double Quon::getAOI()
{
    Enter_Method_Silent();
    return AOIWidth - (double)par("AOIBuffer");
}

Vector2D Quon::getPosition()
{
    Enter_Method_Silent();
    return thisSite->position;
}

double Quon::getAreaDimension()
{
    Enter_Method_Silent();
    return areaDimension;
}

OverlayKey Quon::getKey()
{
    Enter_Method_Silent();
    return thisSite->address.getKey();
}

long Quon::getSoftNeighborCount()
{
    Enter_Method_Silent();
    long temp = softConnections;
    softConnections = 0;
    return temp;
}

Quon::~Quon()
{
    // destroy self timer messages
    cancelAndDelete(join_timer);
    cancelAndDelete(sec_timer);
    cancelAndDelete(alive_timer);
    cancelAndDelete(backup_timer);
    delete thisSite;
    QuonSiteMap::iterator itSites = Sites.begin();
    while(itSites != Sites.end()) {
        delete itSites->second;
        ++itSites;
    }
}
