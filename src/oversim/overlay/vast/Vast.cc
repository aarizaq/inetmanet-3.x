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
 * @file Vast.cc
 * @author Helge Backhaus
 */

#include "Vast.h"

Define_Module(Vast);

#include <NotifierConsts.h>

#include <GlobalNodeList.h>
#include <GlobalStatistics.h>
#include <BootstrapList.h>

void Vast::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if(stage != MIN_STAGE_OVERLAY) return;

    // fetch parameters
    debugVoronoiOutput = par("debugVastOutput");
    areaDimension = par("areaDimension");
    AOI_size = par("AOIWidth");
    joinTimeout = par("joinTimeout");
    pingTimeout = par("pingTimeout");
    discoveryIntervall = par("discoveryIntervall");
    checkCriticalIntervall = par("criticalCheckIntervall");
    criticalThreshold = par("criticalThreshold");
    stockListSize = par("stockListSize");

    // set node key
    thisNode.setKey(OverlayKey::random());
    thisSite.type = THIS;
    thisSite.addr = thisNode;

    geom.setDebug(debugOutput);

    // self-messages
    join_timer = new cMessage("join_timer");
    ping_timer = new cMessage("ping_timer");
    sec_timer = new cMessage("sec_timer");
    discovery_timer = new cMessage("discovery_timer");
    checkcritical_timer = new cMessage("checkcritical_timer");

    // statistics
    joinRequestBytesSent = 0;
    joinAcknowledgeBytesSent = 0;
    nodeMoveBytesSent = 0;
    newNeighborsBytesSent = 0;
    nodeLeaveBytesSent = 0;
    enclosingNeighborsRequestBytesSent = 0;
    pingBytesSent = 0;
    pongBytesSent = 0;
    discardNodeBytesSent = 0;

    maxBytesPerSecondSent = 0;
    averageBytesPerSecondSent = 0;
    bytesPerSecond = 0;
    secTimerCount = 0;

    // watch some variables
    WATCH(AOI_size);
    WATCH(thisSite);
    WATCH_MAP(Sites);
    WATCH_SET(Positions);

    WATCH(joinRequestBytesSent);
    WATCH(joinAcknowledgeBytesSent);
    WATCH(nodeMoveBytesSent);
    WATCH(newNeighborsBytesSent);
    WATCH(nodeLeaveBytesSent);
    WATCH(enclosingNeighborsRequestBytesSent);
    WATCH(pingBytesSent);
    WATCH(pongBytesSent);
    WATCH(discardNodeBytesSent);

    WATCH(maxBytesPerSecondSent);
    WATCH(bytesPerSecond);

    // set initial state
    changeState(INIT);
    changeState(JOINING);
}

void Vast::changeState(int state)
{
    switch(state) {
        case INIT: {
            this->state = INIT;
            globalNodeList->removePeer(thisSite.addr);
            cancelEvent(join_timer);
            cancelEvent(ping_timer);
            cancelEvent(sec_timer);
            cancelEvent(discovery_timer);
            cancelEvent(checkcritical_timer);
        } break;
        case JOINING: {
            this->state = JOINING;
            scheduleAt(simTime(), join_timer);
            scheduleAt(simTime() + 1.0, sec_timer);
        } break;
        case READY: {
            this->state = READY;
            cancelEvent(join_timer);
            globalNodeList->registerPeer(thisSite.addr);
            scheduleAt(simTime() + pingTimeout, ping_timer);
            if(checkCriticalIntervall > 0.0 && discoveryIntervall > 0.0) {
                scheduleAt(simTime() + checkCriticalIntervall, checkcritical_timer);
                scheduleAt(simTime() + discoveryIntervall, discovery_timer);
            }
            // tell the application we are ready
            CompReadyMessage* readyMsg = new CompReadyMessage("OVERLAY_READY");
            readyMsg->setReady(true);
            readyMsg->setComp(getThisCompType());
            sendToApp(readyMsg);
            GameAPIResizeAOIMessage* gameMsg = new GameAPIResizeAOIMessage("RESIZE_AOI");
            gameMsg->setCommand(RESIZE_AOI);
            gameMsg->setAOIsize(AOI_size);
            sendToApp(gameMsg);
        } break;
    }
    setBootstrapedIcon();
    // debug output
    if(debugOutput) {
        EV << "[Vast::changeState() @ " << thisSite.addr.getIp()
           << " (" << thisSite.addr.getKey().toString(16) << ")]\n"
           << "VAST: Node " << thisSite.addr.getIp() << " entered ";
        switch(state) {
            case INIT: ev << "INIT"; break;
            case JOINING: ev << "JOINING"; break;
            case READY: ev << "READY"; break;
        }
        ev << " state." << endl;
    }
}

void Vast::handleTimerEvent(cMessage* msg)
{
    if(msg->isName("join_timer")) {
        //reset timer
        cancelEvent(join_timer);
        scheduleAt(simTime() + joinTimeout, msg);
        // handle event
        processJoinTimer();
    }
    else if(msg->isName("ping_timer")) {
        //reset timer
        cancelEvent(ping_timer);
        scheduleAt(simTime() + pingTimeout, msg);
        // handle event
        processPingTimer();
    }
    else if(msg->isName("sec_timer")) {
        //reset timer
        cancelEvent(sec_timer);
        scheduleAt(simTime() + 1, msg);
        // handle event
        processSecTimer();
    }
    else if(msg->isName("checkcritical_timer")) {
        //reset timer
        cancelEvent(checkcritical_timer);
        scheduleAt(simTime() + checkCriticalIntervall, msg);
        // handle event
        processCheckCriticalTimer();
    }
    else if(msg->isName("discovery_timer")) {
        //reset timer
        cancelEvent(discovery_timer);
        scheduleAt(simTime() + discoveryIntervall, msg);
        // handle event
        processDiscoveryTimer();
    }
}

void Vast::handleAppMessage(cMessage* msg)
{
    if(dynamic_cast<GameAPIMessage*>(msg)) {
        GameAPIMessage* gameAPIMsg = check_and_cast<GameAPIMessage*>(msg);
        // debug output
        if(debugOutput) EV << "[Vast::handleAppMessage() @ " << thisSite.addr.getIp()
                           << " (" << thisSite.addr.getKey().toString(16) << ")]\n"
                           << "    Node " << thisSite.addr.getIp() << " received " << gameAPIMsg->getName() << " from application."
                           << endl;
        switch(gameAPIMsg->getCommand()) {
            case MOVEMENT_INDICATION: {
                GameAPIPositionMessage* gameAPIPosMsg = check_and_cast<GameAPIPositionMessage*>(msg);
                if(state == JOINING) {
                    handleJoin(gameAPIPosMsg);
                    delete msg;
                }
                else if(state == READY) {
                    handleMove(gameAPIPosMsg);
                    delete msg;
                }
            } break;
            case GAMEEVENT_CHAT:
            case GAMEEVENT_SNOW:
            case GAMEEVENT_FROZEN:
                handleEvent( gameAPIMsg );
                delete msg;
                break;
            default: {
                delete msg;
            }
        }
    }
    else delete msg;
}

void Vast::handleUDPMessage(BaseOverlayMessage* msg)
{
    if(state == INIT) {
        delete msg;
        return;
    }
    if(dynamic_cast<VastMessage*>(msg)) {
        VastMessage* vastMsg = check_and_cast<VastMessage*>(msg);
        if(vastMsg->getDestKey().isUnspecified() || 
           thisSite.addr.getKey().isUnspecified() ||
           vastMsg->getDestKey() == thisSite.addr.getKey()) {
            // debug output
            if(debugOutput) EV << "[Vast::handleUDPMessage() @ " << thisSite.addr.getIp()
                               << " (" << thisSite.addr.getKey().toString(16) << ")]\n"
                               << "    Node " << thisSite.addr.getIp() << " received " << vastMsg->getName() << " from " << vastMsg->getSourceNode().getIp()
                               << endl;
            bool doUpdate = true;
            if(state == READY) {
                switch(vastMsg->getCommand()) {
                    case JOIN_REQUEST: {
                        handleJoinRequest(vastMsg);
                        doUpdate = false;
                    } break;
                    case NODE_MOVE: {
                        VastMoveMessage* vastMoveMsg = check_and_cast<VastMoveMessage*>(msg);
                        handleNodeMove(vastMoveMsg);
                    } break;
                    case NEW_NEIGHBORS: {
                        VastListMessage* vastListMsg = check_and_cast<VastListMessage*>(msg);
                        handleNewNeighbors(vastListMsg);
                    } break;
                    case NODE_LEAVE: {
                        VastListMessage* vastListMsg = check_and_cast<VastListMessage*>(msg);
                        handleNodeLeave(vastListMsg);
                    } break;
                    case ENCLOSING_NEIGHBORS_REQUEST: {
                        handleEnclosingNeighborsRequest(vastMsg);
                    } break;
                    case BACKUP_NEIGHBORS: {
                        VastListMessage* vastListMsg = check_and_cast<VastListMessage*>(msg);
                        handleBackupNeighbors(vastListMsg);
                    } break;
                    case PING: {
                        handlePing(vastMsg);
                    } break;
                    case PONG: {
                        handlePong(vastMsg);
                    } break;
                    case DISCARD_NODE: {
                        VastDiscardMessage* vastDiscardMsg = check_and_cast<VastDiscardMessage*>(msg);
                        handleDiscardNode(vastDiscardMsg);
                    } break;
                    case VAST_EVENT: {
                        sendToApp(vastMsg->decapsulate());
                        doUpdate = false;
                        delete vastMsg;
                    } break;
                }
                // update timestamp
                if(doUpdate) {
                    SiteMap::iterator itSites = Sites.find(vastMsg->getSourceNode());
                    if(itSites != Sites.end()) {
                        itSites->second->tstamp = simTime();
                    }
                    delete msg;
                }
            }
            else if(state == JOINING && vastMsg->getCommand() == JOIN_ACKNOWLEDGE) {
                VastListMessage* vastListMsg = check_and_cast<VastListMessage*>(msg);
                handleJoinAcknowledge(vastListMsg);
                delete msg;
            }
            else delete msg;
        }
        else {
            sendDiscardNode(vastMsg);
            delete msg;
        }
    }
    else delete msg;
}

void Vast::addNode(Vector2D p, NodeHandle node, int NeighborCount)
{
    if(node != thisSite.addr) {
        if(Sites.find(node) == Sites.end()) {
            Site* temp_site = new Site();
            temp_site->coord = p;
            temp_site->addr = node;
            temp_site->neighborCount = NeighborCount;

            Sites.insert(std::make_pair(temp_site->addr, temp_site));
            Positions.insert(temp_site->coord);
        }
        else {
            SiteMap::iterator itSites = Sites.find(node);
            Positions.erase(itSites->second->coord);
            itSites->second->coord = p;
            Positions.insert(itSites->second->coord);
            if(NeighborCount != 0) {
                itSites->second->neighborCount = NeighborCount;
            }
        }
    }
}

void Vast::addNodeToStock(NodeHandle node)
{
    if(node != thisSite.addr) {
        for(StockList::iterator itTemp = Stock.begin(); itTemp != Stock.end(); ++itTemp) {
            if(*itTemp == node) {
                return;
            }
        }
        Stock.push_front(node);
        if(Stock.size() > stockListSize) {
            Stock.pop_back();
        }
    }
}

void Vast::removeNode(NodeHandle node)
{
    SiteMap::iterator itSites = Sites.find(node);
    if(itSites != Sites.end()) {
        Positions.erase(itSites->second->coord);
        delete itSites->second;
        Sites.erase(itSites);
    }
}

void Vast::buildVoronoi(Vector2D old_pos, Vector2D new_pos, NodeHandle enclosingCheck)
{
    int sqrt_nsites = 1;
    double xmin, xmax, ymin, ymax;
    double deltax, deltay;

    // check wether there are any neighbors
    if(Sites.size() == 0) return;

    xmin = xmax = thisSite.coord.x;
    ymin = ymax = thisSite.coord.y;

    std::map<Vector2D, Site*> sortedSites;
    sortedSites.insert(std::make_pair(thisSite.coord, &thisSite));

    for(SiteMap::iterator itTemp = Sites.begin(); itTemp != Sites.end(); ++itTemp) {
        // determine min/max site coordinates
        if(itTemp->second->coord.x < xmin) xmin = itTemp->second->coord.x;
        if(itTemp->second->coord.x > xmax) xmax = itTemp->second->coord.x;
        if(itTemp->second->coord.y < ymin) ymin = itTemp->second->coord.y;
        if(itTemp->second->coord.y > ymax) ymax = itTemp->second->coord.y;
        // reset all sites to UNDEF
        itTemp->second->type = UNDEF;
        // reset enclosing neighbors set
        itTemp->second->enclosingSet.clear();
        // fill sorted List
        sortedSites.insert(std::make_pair(itTemp->second->coord, itTemp->second));
        sqrt_nsites++;
    }

    // needed to determine appropriate hashtable size
    deltax = xmax - xmin;
    deltay = ymax - ymin;
    sqrt_nsites = (int)sqrt((double)(sqrt_nsites+4));

    // start to calculate the voronoi
    Site *newsite, *bot, *top, *temp, *p, *v, *bottomsite;
    Vector2D newintstar;
    int pm;
    Halfedge *lbnd, *rbnd, *llbnd, *rrbnd, *bisector;
    Edge *e;

    newintstar.x = newintstar.y = 0.0;

    std::map<Vector2D, Site*>::iterator itSortedSites = sortedSites.begin();

    geom.initialize(deltax, deltay, thisSite.coord, old_pos, new_pos, AOI_size);
    heap.PQinitialize(sqrt_nsites, ymin, deltay);
    bottomsite = itSortedSites->second;
    ++itSortedSites;
    edgelist.initialize(sqrt_nsites, xmin, deltax, bottomsite);

    newsite = itSortedSites->second;
    ++itSortedSites;
    while(true) {
        if(!heap.PQempty()) newintstar = heap.PQ_min();

        if(newsite != NULL && (heap.PQempty() ||
           newsite->coord.y < newintstar.y ||
           (newsite->coord.y == newintstar.y && newsite->coord.x < newintstar.x))) {
            lbnd = edgelist.ELleftbnd(&(newsite->coord));
            rbnd = edgelist.ELright(lbnd);
            bot = edgelist.rightreg(lbnd);
            e = geom.bisect(bot, newsite);
            bisector = edgelist.HEcreate(e, le);
            edgelist.ELinsert(lbnd, bisector);
            if ((p = geom.intersect(lbnd, bisector)) != NULL) {
                heap.PQdelete(lbnd);
                heap.PQinsert(lbnd, p, geom.dist(p, newsite));
            }
            lbnd = bisector;
            bisector = edgelist.HEcreate(e, re);
            edgelist.ELinsert(lbnd, bisector);
            if ((p = geom.intersect(bisector, rbnd)) != NULL) heap.PQinsert(bisector, p, geom.dist(p, newsite));
            if(itSortedSites != sortedSites.end()) {
                newsite = itSortedSites->second;
                ++itSortedSites;
            }
            else newsite = NULL;
        }
        else if (!heap.PQempty()) {
            lbnd = heap.PQextractmin();
            llbnd = edgelist.ELleft(lbnd);
            rbnd = edgelist.ELright(lbnd);
            rrbnd = edgelist.ELright(rbnd);
            bot = edgelist.leftreg(lbnd);
            top = edgelist.rightreg(rbnd);
            v = lbnd->vertex;
            geom.endpoint(lbnd->ELedge, lbnd->ELpm, v);
            geom.endpoint(rbnd->ELedge, rbnd->ELpm, v);
            edgelist.ELdelete(lbnd);
            heap.PQdelete(rbnd);
            edgelist.ELdelete(rbnd);
            pm = le;
            if (bot->coord.y > top->coord.y) {
                temp = bot;
                bot = top;
                top = temp;
                pm = re;
            }
            e = geom.bisect(bot, top);
            bisector = edgelist.HEcreate(e, pm);
            edgelist.ELinsert(llbnd, bisector);
            geom.endpoint(e, re-pm, v);
            if((p = geom.intersect(llbnd, bisector)) != NULL) {
                heap.PQdelete(llbnd);
                heap.PQinsert(llbnd, p, geom.dist(p, bot));
            }
            if ((p = geom.intersect(bisector, rrbnd)) != NULL) heap.PQinsert(bisector, p, geom.dist(p, bot));
        }
        else break;
    }

    // process the generated edgelist
    for(lbnd = edgelist.ELright(edgelist.ELleftend); lbnd != edgelist.ELrightend; lbnd = edgelist.ELright(lbnd)) {
        e = lbnd -> ELedge;
        geom.processEdge(e);
    }
    // process sites in order to determine our neighbors
    for(SiteMap::iterator itTemp = Sites.begin(); itTemp != Sites.end(); ++itTemp) {
        if(itTemp->second->innerEdge[0]) {
            if(itTemp->second->outerEdge) {
                itTemp->second->type |= BOUNDARY;
                // Debug output
                if(debugOutput)
                    EV << "[NeighborsList::buildVoronoi()]\n"
                       << "    Site at [" << itTemp->second->coord.x << ", "
                       << itTemp->second->coord.y << "] is a boundary neighbor."
                       << endl;
            }
            else {
                itTemp->second->type |= NEIGHBOR;
                // Debug output
                if(debugOutput)
                    EV << "[NeighborsList::buildVoronoi()]\n"
                       << "    Site at [" << itTemp->second->coord.x << ", "
                       << itTemp->second->coord.y << "] is a neighbor."
                       << endl;
            }
        }
        // Treat enclosing neighbors whose voronoi region is outside our AOI as boundary neighbors
        if((!itTemp->second->type & NEIGHBOR) && (itTemp->second->type & ENCLOSING)) {
            itTemp->second->type |= BOUNDARY;
            // Debug output
            if(debugOutput)
                EV << "[NeighborsList::buildVoronoi()]\n"
                << "    Site at [" << itTemp->second->coord.x << ", "
                << itTemp->second->coord.y << "] is a boundary neighbor."
                << endl;
        }
        if(!itTemp->second->innerEdge[1] && itTemp->second->innerEdge[2]) {
            itTemp->second->type |= NEW;
            // Debug output
            if(debugOutput)
                EV << "[NeighborsList::buildVoronoi()]\n"
                   << "    Site at [" << itTemp->second->coord.x << ", "
                   << itTemp->second->coord.y << "] is a new neighbor for site at " << new_pos.x << ":" << new_pos.y << "."
                   << endl;
        }
        // enhanced enclosing check
        if(!enclosingCheck.isUnspecified() && (Sites.find(enclosingCheck) != Sites.end())) {
            Site* tempSite = Sites.find(enclosingCheck)->second;
            for(EnclosingSet::iterator itSet = tempSite->enclosingSet.begin(); itSet != tempSite->enclosingSet.end(); ++itSet) {
                if(tempSite->oldEnclosingSet.find(*itSet) == tempSite->oldEnclosingSet.end()
                    && Sites.find(*itSet) != Sites.end()) {
                    Sites.find(*itSet)->second->type |= NEW;
                }
            }
            tempSite->enclosingSet.swap(tempSite->oldEnclosingSet);
        }
        // reset inner- and outeredge indicator
        itTemp->second->innerEdge[0] = false;
        itTemp->second->innerEdge[1] = false;
        itTemp->second->innerEdge[2] = false;
        itTemp->second->outerEdge = false;
    }
    // clean up
    edgelist.reset();
    heap.PQreset();
    geom.reset();
}

void Vast::buildVoronoi()
{
    buildVoronoi(thisSite.coord, thisSite.coord);
}

void Vast::removeNeighbors()
{
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end();) {
        // if current site is no neighbor remove it else go on to next site
        if(itSites->second->type == UNDEF) {
            // Debug output
            if(debugOutput)  EV << "[NeighborsList::removeNeighbors()]\n"
                                << "    Site at [" << itSites->second->coord.x << ", " << itSites->second->coord.y
                                << "] has been removed from list."
                                << endl;
            Positions.erase(itSites->second->coord);
            delete itSites->second;
            Sites.erase(itSites++);
        }
        else ++itSites;
    }
}

void Vast::handleNodeLeaveNotification()
{
    if(state == READY) {
        // debug output
        if(debugOutput) {
            EV << "[Vast::receiveChangeNotification() @ " << thisSite.addr.getIp()
               << " (" << thisSite.addr.getKey().toString(16) << ")]\n"
               << "    Node " << thisSite.addr.getIp() << " is leaving the overlay."
               << endl;
        }

        CompReadyMessage* readyMsg = new CompReadyMessage("OVERLAY_FINISHED");
        readyMsg->setReady(false);
        readyMsg->setComp(getThisCompType());
        sendToApp(readyMsg);
    }
}

void Vast::handleNodeGracefulLeaveNotification()
{
     if(state == READY) {
        // generate node leave messages
        VastListMessage *vastListMsg = new VastListMessage("NODE_LEAVE");
        vastListMsg->setCommand(NODE_LEAVE);
        // fill neighbors list
        vastListMsg->setNeighborNodeArraySize(Sites.size());
        vastListMsg->setNeighborPosArraySize(Sites.size());
        int i = 0;
        for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            if(itSites->second->type & ENCLOSING) {
                vastListMsg->setNeighborNode(i, itSites->second->addr);
                vastListMsg->setNeighborPos(i, itSites->second->coord);
                ++i;
            }
        }
        vastListMsg->setNeighborNodeArraySize(i);
        vastListMsg->setNeighborPosArraySize(i);

        vastListMsg->setBitLength(VASTLIST_L(vastListMsg));
        if(vastListMsg->getNeighborNodeArraySize() > 0) {
            for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
                VastListMessage *vastCopyMsg = new VastListMessage(*vastListMsg);
               sendMessage(vastCopyMsg, itSites->second->addr);
            }
        }
        delete vastListMsg;
        changeState(INIT);
    }
}

void Vast::processJoinTimer()
{
    GameAPIMessage *sgcMsg = new GameAPIMessage("MOVEMENT_REQUEST");
    sgcMsg->setCommand(MOVEMENT_REQUEST);
    sendToApp(sgcMsg);
}

void Vast::processPingTimer()
{
    bool abnormalLeave = false;
    bool boundaryLeave = false;
    std::set<NodeHandle> removeSet;
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        if(itSites->second->tstamp < 0.0) { // node is dropped cause no pong has been received see below
            abnormalLeave = true;
            if(!(itSites->second->type & NEIGHBOR)) boundaryLeave = true;
            itSites->second->type = UNDEF;
            removeSet.insert( itSites->first );
        }
        else if(itSites->second->tstamp < simTime() - pingTimeout) { // node showed no activity for some time request pong and mark it to be dropped next time
            VastMessage *vastMsg = new VastMessage("PING");
            vastMsg->setCommand(PING);
            vastMsg->setBitLength(VAST_L(vastMsg));
            sendMessage(vastMsg, itSites->second->addr);
            itSites->second->tstamp = -1.0;
        }
    }
    if(abnormalLeave) {
        synchronizeApp();
        for( std::set<NodeHandle>::iterator it = removeSet.begin(); it != removeSet.end(); ++it) {
            removeNode( *it );
        }
        // removeNeighbors();
        if(boundaryLeave) {
            for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
                if(itSites->second->type & BOUNDARY) {
                    VastMessage *vastMsg = new VastMessage("ENCLOSING_NEIGHBORS_REQUEST");
                    vastMsg->setCommand(ENCLOSING_NEIGHBORS_REQUEST);
                    vastMsg->setBitLength(VAST_L(vastMsg));
                    sendMessage(vastMsg, itSites->second->addr);
                }
            }
        }
        //buildVoronoi();
        //removeNeighbors(); should be superfluous
    }
}

void Vast::processSecTimer()
{
    RECORD_STATS(
        if(bytesPerSecond > maxBytesPerSecondSent) {
            maxBytesPerSecondSent = bytesPerSecond;
        }
        averageBytesPerSecondSent += bytesPerSecond;
        ++secTimerCount;
    );
    bytesPerSecond = 0;
}

void Vast::processCheckCriticalTimer()
{
    double NeighborLevel;
    int NeighborSum = 0;
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        if(itSites->second->neighborCount > 0) {
            NeighborSum += itSites->second->neighborCount;
        }
    }
    NeighborLevel = (double)(Sites.size() * Sites.size()) / (double)NeighborSum;

    if(NeighborLevel < criticalThreshold) {
        VastListMessage *vastListMsg = new VastListMessage("BACKUP_NEIGHBORS");
        vastListMsg->setCommand(BACKUP_NEIGHBORS);
        // fill neighbors list
        vastListMsg->setNeighborNodeArraySize(Sites.size());
        vastListMsg->setNeighborPosArraySize(Sites.size());
        int i = 0;
        for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            vastListMsg->setNeighborNode(i, itSites->second->addr);
            vastListMsg->setNeighborPos(i, itSites->second->coord);
            ++i;
        }
        vastListMsg->setBitLength(VASTLIST_L(vastListMsg));
        for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            VastListMessage *vastCopyMsg = new VastListMessage(*vastListMsg);
            sendMessage(vastCopyMsg, itSites->second->addr);
        }
        delete vastListMsg;
    }
}

void Vast::processDiscoveryTimer()
{
    for(StockList::iterator itStock = Stock.begin(); itStock != Stock.end(); ++itStock) {
        VastMoveMessage *vastMoveMsg = new VastMoveMessage("NODE_MOVE");
        vastMoveMsg->setCommand(NODE_MOVE);
        vastMoveMsg->setNewPos(thisSite.coord);
        vastMoveMsg->setRequest_list(true);
        vastMoveMsg->setBitLength(VASTMOVE_L(vastMoveMsg));
        sendMessage(vastMoveMsg, *itStock);
    }
}

void Vast::handleJoin(GameAPIPositionMessage *sgcMsg)
{
    TransportAddress joinNode = bootstrapList->getBootstrapNode();
    thisSite.coord = sgcMsg->getPosition();
    // check if this is the only node in the overlay
    if(joinNode.isUnspecified()) {
        changeState(READY);
    }
    else {
        VastMessage *vastMsg = new VastMessage("JOIN_REQUEST");
        vastMsg->setCommand(JOIN_REQUEST);
        vastMsg->setBitLength(VAST_L(vastMsg));
        sendMessage(vastMsg, joinNode);
    }
}

void Vast::handleMove(GameAPIPositionMessage* sgcMsg)
{
    Vector2D pos = sgcMsg->getPosition();
    // test if new position is legal
    if(Positions.find(pos) != Positions.end()) {
        GameAPIMessage *gameMsg = new GameAPIMessage("MOVEMENT_REQUEST");
        gameMsg->setCommand(MOVEMENT_REQUEST);
        sendToApp(gameMsg);
        return;
    }
    // set new position
    thisSite.coord = pos;
    // update voronoi
    buildVoronoi();
    synchronizeApp();
    removeNeighbors();
    // send position update to neighbors
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        VastMoveMessage *vastMoveMsg = new VastMoveMessage("NODE_MOVE");
        vastMoveMsg->setCommand(NODE_MOVE);
        vastMoveMsg->setNewPos(pos);
        vastMoveMsg->setIs_boundary(itSites->second->type & BOUNDARY);
        vastMoveMsg->setBitLength(VASTMOVE_L(vastMoveMsg));
        sendMessage(vastMoveMsg, itSites->second->addr);
    }
}

void Vast::handleEvent( GameAPIMessage* msg )
{
    // send event to neighbors
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        VastEventMessage *vastMsg = new VastEventMessage("EVENT");
        vastMsg->setCommand(VAST_EVENT);
        vastMsg->encapsulate((cPacket*)msg->dup());
        // FIXME: Message length!
        sendMessage(vastMsg, itSites->second->addr);
    }
}

void Vast::handleJoinRequest(VastMessage *vastMsg)
{
    Site *forwardSite = NULL;
    // start with this node
    double min_dist = thisSite.coord.distanceSqr(vastMsg->getPos());
    forwardSite = &thisSite;
    // iterate through all neighbors
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        // dont forward to nodes which are still joining
        if(itSites->second->coord.distanceSqr(vastMsg->getPos()) < min_dist && itSites->second->neighborCount >= 0) {
            min_dist = itSites->second->coord.distanceSqr(vastMsg->getPos());
            forwardSite = itSites->second;
        }
    }
    // do nothing and let node retry with new position if current position is illegal
    if(min_dist == 0.0) {
        delete vastMsg;
    }
    else {
        // send an acknowledge or forward request if any of our neighbors is closer to joining node
        if(forwardSite->type & THIS) {
            VastListMessage *vastListMsg = new VastListMessage("JOIN_ACKNOWLEDGE");
            vastListMsg->setCommand(JOIN_ACKNOWLEDGE);
            // fill neighbors list
            vastListMsg->setNeighborNodeArraySize(Sites.size());
            vastListMsg->setNeighborPosArraySize(Sites.size());
            int i = 0;
            for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
                vastListMsg->setNeighborNode(i, itSites->second->addr);
                vastListMsg->setNeighborPos(i, itSites->second->coord);
                ++i;
            }

            vastListMsg->setBitLength(VASTLIST_L(vastListMsg));
            sendMessage(vastListMsg, vastMsg->getSourceNode());

            // add node to list to propagte its position early
            // nieghborCount is set to -1 to indicate node is still joining
            addNode(vastMsg->getPos(), vastMsg->getSourceNode(), -1);
            // update voronoi with new neighbors
            buildVoronoi();
            synchronizeApp();
            // removeNeighbors();
            delete vastMsg;
        }
        else {
            sendMessage(vastMsg, forwardSite->addr);
        }
    }
}

void Vast::handleJoinAcknowledge(VastListMessage *vastListMsg)
{
    // add acceptor node
    changeState(READY);
    addNode(vastListMsg->getPos(), vastListMsg->getSourceNode(), vastListMsg->getNeighborCount());
    // add new neighbors
    for(unsigned int i=0; i<vastListMsg->getNeighborNodeArraySize(); i++) {
        addNode(vastListMsg->getNeighborPos(i), vastListMsg->getNeighborNode(i));
    }
    // update voronoi with new neighbors
    buildVoronoi();
    synchronizeApp();
    // removeNeighbors();
    // contact new neighbors
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        VastMoveMessage *vastMoveMsg = new VastMoveMessage("NODE_MOVE");
        vastMoveMsg->setCommand(NODE_MOVE);
        vastMoveMsg->setNewPos(thisSite.coord);
        vastMoveMsg->setIs_boundary(itSites->second->type & BOUNDARY);
        vastMoveMsg->setRequest_list(true);
        vastMoveMsg->setBitLength(VASTMOVE_L(vastMoveMsg));
        sendMessage(vastMoveMsg, itSites->second->addr);
    }
}

void Vast::handleNodeMove(VastMoveMessage *vastMoveMsg)
{
    RECORD_STATS(
            globalStatistics->addStdDev(
                "Vast: MoveDelay",
                SIMTIME_DBL(simTime()) - SIMTIME_DBL(vastMoveMsg->getCreationTime())
                );
            );

    Vector2D old_p, new_p;
    old_p = vastMoveMsg->getPos();
    new_p = vastMoveMsg->getNewPos();
    addNode(new_p, vastMoveMsg->getSourceNode(), vastMoveMsg->getNeighborCount());
    // update voronoi with new neighbor detection or without
    if(vastMoveMsg->getIs_boundary() || vastMoveMsg->getRequest_list()) {
        buildVoronoi(old_p, new_p, vastMoveMsg->getSourceNode()); // enhanced enclosing check
        synchronizeApp(vastMoveMsg);
        // removeNeighbors();
        // send new neighbors
        VastListMessage *vastListMsg = new VastListMessage("NEW_NEIGHBORS");
        vastListMsg->setCommand(NEW_NEIGHBORS);

        vastListMsg->setNeighborNodeArraySize(Sites.size());
        vastListMsg->setNeighborPosArraySize(Sites.size());

        int i = 0;
        for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            if(itSites->second->type & NEW || vastMoveMsg->getRequest_list()) {
                vastListMsg->setNeighborNode(i, itSites->second->addr);
                vastListMsg->setNeighborPos(i, itSites->second->coord);
                ++i;
            }
        }
        vastListMsg->setNeighborNodeArraySize(i);
        vastListMsg->setNeighborPosArraySize(i);
        vastListMsg->setRequestEnclosingNeighbors(true);

        vastListMsg->setBitLength(VASTLIST_L(vastListMsg));
        if(vastListMsg->getNeighborNodeArraySize() > 0) {
            sendMessage(vastListMsg, vastMoveMsg->getSourceNode());
        }
        else {
            delete vastListMsg;
        }
    }
    else {
        // buildVoronoi();
        synchronizeApp(vastMoveMsg);
        // removeNeighbors();
    }
}

void Vast::handleNewNeighbors(VastListMessage *vastListMsg)
{
    // add new neighbors
    for(unsigned int i=0; i<vastListMsg->getNeighborNodeArraySize(); i++) {
        addNode(vastListMsg->getNeighborPos(i), vastListMsg->getNeighborNode(i));

        if(vastListMsg->getRequestEnclosingNeighbors()) {
            VastMessage *vastMsg = new VastMessage("ENCLOSING_NEIGHBORS_REQUEST");
            vastMsg->setCommand(ENCLOSING_NEIGHBORS_REQUEST);
            vastMsg->setBitLength(VAST_L(vastMsg));
            sendMessage(vastMsg, vastListMsg->getNeighborNode(i));
        }
    }
    // update voronoi with new neighbors
//    buildVoronoi();
//    synchronizeApp();
    // removeNeighbors();
}

void Vast::handleNodeLeave(VastListMessage *vastListMsg)
{
    removeNode(vastListMsg->getSourceNode());
    // add possible new neighbors
    for(unsigned int i=0; i<vastListMsg->getNeighborNodeArraySize(); i++) {
        addNode(vastListMsg->getNeighborPos(i), vastListMsg->getNeighborNode(i));
    }
    // update voronoi with new neighbors
    // buildVoronoi();
    // synchronizeApp();
    // removeNeighbors();
}

void Vast::handleEnclosingNeighborsRequest(VastMessage *vastMsg)
{
    // send new neighbors
    VastListMessage *vastListMsg = new VastListMessage("NEW_NEIGHBORS");
    vastListMsg->setCommand(NEW_NEIGHBORS);

    vastListMsg->setNeighborNodeArraySize(Sites.size());
    vastListMsg->setNeighborPosArraySize(Sites.size());

    int i = 0;
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        if((itSites->second->type & ENCLOSING) && itSites->second->addr != vastMsg->getSourceNode()) {
            vastListMsg->setNeighborNode(i, itSites->second->addr);
            vastListMsg->setNeighborPos(i, itSites->second->coord);
            ++i;
        }
    }
    vastListMsg->setNeighborNodeArraySize(i);
    vastListMsg->setNeighborPosArraySize(i);

    vastListMsg->setBitLength(VASTLIST_L(vastListMsg));
    if(vastListMsg->getNeighborNodeArraySize() > 0) {
        sendMessage(vastListMsg, vastMsg->getSourceNode());
    }
    else {
        delete vastListMsg;
    }
}

void Vast::handleBackupNeighbors(VastListMessage *vastListMsg)
{
    // add new neighbors to stock list
    for(unsigned int i=0; i<vastListMsg->getNeighborNodeArraySize(); i++) {
        if(Sites.find(vastListMsg->getNeighborNode(i)) == Sites.end()) {
            addNodeToStock(vastListMsg->getNeighborNode(i));
        }
    }
}

void Vast::handlePing(VastMessage *vastMsg)
{
    VastMessage *vastPongMsg = new VastMessage("PONG");
    vastPongMsg->setCommand(PONG);
    vastPongMsg->setBitLength(VAST_L(vastPongMsg));
    sendMessage(vastPongMsg, vastMsg->getSourceNode());
}

void Vast::handlePong(VastMessage *vastMsg)
{
    // replace entry cause it was probably outdated
    addNode(vastMsg->getPos(), vastMsg->getSourceNode(), vastMsg->getNeighborCount());
    // update voronoi
    //buildVoronoi();
    //synchronizeApp();
    // removeNeighbors();
}

void Vast::handleDiscardNode(VastDiscardMessage *vastMsg)
{
    // discard outdated entry
    removeNode(vastMsg->getDiscardNode());
    // update voronoi
    //buildVoronoi();
    //synchronizeApp();
    // removeNeighbors();
}

void Vast::sendDiscardNode(VastMessage *vastMsg)
{
    NodeHandle discardNode;
    discardNode.setIp(thisSite.addr.getIp());
    discardNode.setKey(vastMsg->getDestKey());
    // send message
    VastDiscardMessage *vastDiscardMsg = new VastDiscardMessage("DISCARD_NODE");
    vastDiscardMsg->setCommand(DISCARD_NODE);
    vastDiscardMsg->setDiscardNode(discardNode);
    // debug output
    if(debugOutput) EV << "[Vast::sendDiscardNode() @ " << thisSite.addr.getIp()
                       << " (" << thisSite.addr.getKey().toString(16) << ")]\n"
                       << "    Node " << thisSite.addr.getIp() << " is leaving the overlay."
                       << endl;
    vastDiscardMsg->setBitLength(VASTDISCARD_L(vastDiscardMsg));
    sendMessage(vastDiscardMsg, vastMsg->getSourceNode());
}

void Vast::synchronizeApp(VastMoveMessage *vastMoveMsg)
{
    GameAPIListMessage *sgcMsg = new GameAPIListMessage("NEIGHBOR_UPDATE");
    sgcMsg->setCommand(NEIGHBOR_UPDATE);

    sgcMsg->setRemoveNeighborArraySize(Sites.size());
    sgcMsg->setAddNeighborArraySize(Sites.size() + 1);
    sgcMsg->setNeighborPositionArraySize(Sites.size() + 1);

    int remSize, addSize;
    remSize = addSize = 0;
    for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
        if(itSites->second->type == UNDEF) {
            sgcMsg->setRemoveNeighbor(remSize, itSites->second->addr);
            ++remSize;
        }
        else if(!itSites->second->isAdded) {
            sgcMsg->setAddNeighbor(addSize, itSites->second->addr);
            sgcMsg->setNeighborPosition(addSize, itSites->second->coord);
            itSites->second->isAdded = true;
            ++addSize;
        }
    }

    if(vastMoveMsg &&
       Sites.find(vastMoveMsg->getSourceNode()) != Sites.end() &&
       Sites.find(vastMoveMsg->getSourceNode())->second->isAdded) {
        sgcMsg->setAddNeighbor(addSize, vastMoveMsg->getSourceNode());
        sgcMsg->setNeighborPosition(addSize, vastMoveMsg->getNewPos());
        ++addSize;
    }

    sgcMsg->setRemoveNeighborArraySize(remSize);
    sgcMsg->setAddNeighborArraySize(addSize);
    sgcMsg->setNeighborPositionArraySize(addSize);

    if(sgcMsg->getAddNeighborArraySize() || sgcMsg->getRemoveNeighborArraySize()) {
        sendToApp(sgcMsg);
    }
    else {
        delete sgcMsg;
    }
}

void Vast::sendToApp(cMessage *msg)
{
    // debug output
    if(debugOutput) EV << "[Vast::sendToApp() @ " << thisSite.addr.getIp()
                       << " (" << thisSite.addr.getKey().toString(16) << ")]\n"
                       << "    Node " << thisSite.addr.getIp() << " sending " << msg->getName() << " to application."
                       << endl;
    send(msg, "appOut");
}

void Vast::sendMessage(VastMessage *vastMsg, NodeHandle destAddr)
{
    // collect statistics
    RECORD_STATS(
        switch(vastMsg->getCommand()) {
            case JOIN_REQUEST: {
                joinRequestBytesSent += vastMsg->getByteLength();
            } break;
            case JOIN_ACKNOWLEDGE: {
                joinAcknowledgeBytesSent += vastMsg->getByteLength();
            } break;
            case NODE_MOVE: {
                nodeMoveBytesSent += vastMsg->getByteLength();
            } break;
            case NEW_NEIGHBORS: {
                newNeighborsBytesSent += vastMsg->getByteLength();
            } break;
            case NODE_LEAVE: {
                nodeLeaveBytesSent += vastMsg->getByteLength();
            } break;
            case ENCLOSING_NEIGHBORS_REQUEST: {
                enclosingNeighborsRequestBytesSent += vastMsg->getByteLength();
            } break;
            case PING: {
                pingBytesSent += vastMsg->getByteLength();
            } break;
            case PONG: {
                pongBytesSent += vastMsg->getByteLength();
            } break;
            case DISCARD_NODE: {
                discardNodeBytesSent += vastMsg->getByteLength();
            } break;
        }
        bytesPerSecond += vastMsg->getByteLength();
    );

    // debug output
    if(debugOutput) EV << "[Vast::sendMessage() @ " << thisSite.addr.getIp()
                       << " (" << thisSite.addr.getKey().toString(16) << ")]\n"
                       << "    Node " << thisSite.addr.getIp() << " sending " << vastMsg->getName() << " to " << destAddr.getIp() << "."
                       << endl;
    // set vastbase message stuff
    vastMsg->setDestKey(destAddr.getKey());
    // fill in sender information only if we are not forwarding a message from another node
    // e.g. a joining node
    if(vastMsg->getSourceNode().isUnspecified()) {
        vastMsg->setSourceNode(thisSite.addr);
        vastMsg->setPos(thisSite.coord);
        vastMsg->setNeighborCount(Sites.size());
    }

    sendMessageToUDP(destAddr, vastMsg);
}

void Vast::setBootstrapedIcon()
{
    if(ev.isGUI()) {
        if(state == READY) {
            getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "green");
            getDisplayString().setTagArg("i", 1, "green");
        }
        else if(state == JOINING) {
            getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "yellow");
            getDisplayString().setTagArg("i", 1, "yellow");
        }
        else {
            getParentModule()->getParentModule()->getDisplayString().setTagArg("i2", 1, "red");
            getDisplayString().setTagArg("i", 1, "red");
        }
    }
}

void Vast::finishOverlay()
{
    globalNodeList->removePeer(thisSite.addr);

//    We use our own time count to avoid rounding errors
//    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
//    if(time == 0) return;
    if(secTimerCount == 0) return;

    // collect statistics
    globalStatistics->addStdDev("Vast: JOIN_REQUEST bytes sent/s", joinRequestBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: JOIN_ACKNOWLEDGE bytes sent/s", joinAcknowledgeBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: NODE_MOVE bytes sent/s", nodeMoveBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: NEW_NEIGHBORS bytes sent/s", newNeighborsBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: NODE_LEAVE bytes sent/s", nodeLeaveBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: ENCLOSING_NEIGHBORS_REQUEST bytes sent/s", enclosingNeighborsRequestBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: PING bytes sent/s", pingBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: PONG bytes sent/s", pongBytesSent/(double) secTimerCount);
    globalStatistics->addStdDev("Vast: DISCARD_NODE bytes sent/s", discardNodeBytesSent/(double) secTimerCount);

    globalStatistics->addStdDev("Vast: max bytes/second sent", maxBytesPerSecondSent);
    globalStatistics->addStdDev("Vast: average bytes/second sent", averageBytesPerSecondSent / (double) secTimerCount);
}

double Vast::getAOI()
{
    Enter_Method_Silent();
    return AOI_size;
}

Vector2D Vast::getPosition()
{
    Enter_Method_Silent();
    return thisSite.coord;
}

NodeHandle Vast::getHandle()
{
    Enter_Method_Silent();
    return thisSite.addr;
}

double Vast::getAreaDimension()
{
    Enter_Method_Silent();
    return areaDimension;
}

Vast::~Vast()
{
    if(Sites.size()) {
        for(SiteMap::iterator itSites = Sites.begin(); itSites != Sites.end(); ++itSites) {
            delete itSites->second;
        }
        Sites.clear();
        Positions.clear();
    }
    // destroy self timer messages
    cancelAndDelete(join_timer);
    cancelAndDelete(ping_timer);
    cancelAndDelete(sec_timer);
    cancelAndDelete(discovery_timer);
    cancelAndDelete(checkcritical_timer);
}
