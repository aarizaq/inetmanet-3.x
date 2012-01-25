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
* @file SimpleGameClient.cc
* @author Helge Backhaus
* @author Stephan Krause
*/

#include "SimpleGameClient.h"

Define_Module(SimpleGameClient);

void SimpleGameClient::initializeApp(int stage)
{
    // all initialization is done in the first stage
    if (stage != MIN_STAGE_APP) {
        return;
    }

    // fetch parameters
    areaDimension = par("areaDimension");
    useScenery = par("useScenery");
    movementRate = par("movementRate");
    movementSpeed = par("movementSpeed");
    movementDelay= 1.0 / movementRate;
    AOIWidth = 0.0;
    logAOI = false;
    int groupSize = (int)par("groupSize");
    if(groupSize < 1) {
        groupSize = 1;
    }
    GeneratorType = par("movementGenerator").stdstringValue();

    WATCH_MAP(Neighbors);
    WATCH_LIST(CollisionRect);
    WATCH(position);
    WATCH(GeneratorType);
    WATCH(overlayReady);

    doRealworld = false;
    coordinator = check_and_cast<GlobalCoordinator*>(simulation.getModuleByPath("globalObserver.globalFunctions[0].function.coordinator"));
    scheduler = NULL;
    packetNotification = NULL;

    frozen = false;

    useHotspots = false;
    lastInHotspot = false;
    lastFarFromHotspot = false;
    lastAOImeasure = simTime();
    startAOI = 0;
    avgAOI = 0;
    hotspotTime = 0;
    avgHotspotAOI = 0;
    nonHotspotTime = 0;
    avgFarFromHotspotAOI = 0;
    farFromHotspotTime = 0;

    CollisionList* listPtr = NULL;
    if(useScenery == true) {
        listPtr = &CollisionRect;
    }

    double speed = SIMTIME_DBL(movementDelay) * movementSpeed;
    if(strcmp(GeneratorType.c_str(), "greatGathering") == 0) {
        if(debugOutput) {
            EV << "[SimpleGameClient::initializeApp() @ " << overlay->getThisNode().getIp()
               << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
               << "    SIMPLECLIENT: Movement generator: greatGathering"
               << endl;
        }
        Generator = new greatGathering(areaDimension, speed, &Neighbors, coordinator, listPtr);
    }
    else if(strcmp(GeneratorType.c_str(), "groupRoaming") == 0) {
        if(debugOutput) {
            EV << "[SimpleGameClient::initializeApp() @ " << overlay->getThisNode().getIp()
               << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
               << "    SIMPLECLIENT: Movement generator: groupRoaming"
               << endl;
        }
        Generator = new groupRoaming(areaDimension, speed, &Neighbors, coordinator, listPtr, groupSize);
    }
    else if(strcmp(GeneratorType.c_str(), "hotspotRoaming") == 0) {
        if(debugOutput) {
            EV << "[SimpleGameClient::initializeApp() @ " << overlay->getThisNode().getIp()
               << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
               << "    SIMPLECLIENT: Movement generator: hotspotRoaming"
               << endl;
        }
        Generator = new hotspotRoaming(areaDimension, speed, &Neighbors, coordinator, listPtr);
        useHotspots = true;
    }
    else if(strcmp(GeneratorType.c_str(), "traverseRoaming") == 0) {
        if(debugOutput) {
            EV << "[SimpleGameClient::initializeApp() @ " << overlay->getThisNode().getIp()
               << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
               << "    SIMPLECLIENT: Movement generator: traverseRoaming"
               << endl;
        }
        logAOI = true;
        Generator = new traverseRoaming(areaDimension, speed, &Neighbors, coordinator, listPtr);
    }
    else if(strcmp(GeneratorType.c_str(), "realWorldRoaming") == 0) {
        if(debugOutput) {
            EV << "[SimpleGameClient::initializeApp() @ " << overlay->getThisNode().getIp()
               << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
               << "    SIMPLECLIENT: Movement generator: realWorldRoaming"
               << endl;
        }
        Generator = new realWorldRoaming(areaDimension, movementSpeed, &Neighbors, coordinator, listPtr);
        mtu = par("mtu");
        packetNotification = new cMessage("packetNotification");
        scheduler = check_and_cast<RealtimeScheduler*>(simulation.getScheduler());
        scheduler->setInterfaceModule(this, packetNotification, &packetBuffer, mtu, true);
        appFd = INVALID_SOCKET;
    }
    else {
        // debug output
        if(debugOutput) {
            if(strcmp(GeneratorType.c_str(), "randomRoaming") == 0) {
                EV << "[SimpleGameClient::initializeApp() @ " << overlay->getThisNode().getIp()
                   << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                   << "    SIMPLECLIENT: Movement generator: randomRoaming"
                   << endl;
            }
            else {
                EV << "[SimpleGameClient::initializeApp() @ " << overlay->getThisNode().getIp()
                   << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                   << "    SIMPLECLIENT: Movement generator: <unknown>. Defaulting to: randomRoaming"
                   << endl;
            }
        }
        Generator = new randomRoaming(areaDimension, speed, &Neighbors, coordinator, listPtr);
    }

    // self-messages
    move_timer = new cMessage("move_timer");
    overlayReady = false;
}

void SimpleGameClient::handleTimerEvent(cMessage* msg)
{
    if(msg->isName("move_timer")) {
        //reset timer
        cancelEvent(move_timer);
        if(overlayReady) {
            scheduleAt(simTime() + movementDelay, msg);
        }
        // handle event
        updatePosition();
    }
    else if(msg->isName("packetNotification")) {
        while(packetBuffer.size() > 0) {
            // get packet from buffer and parse it
            RealtimeScheduler::PacketBufferEntry packet = *(packetBuffer.begin());
            packetBuffer.pop_front();
            switch (packet.func) {
                case RealtimeScheduler::PacketBufferEntry::PACKET_DATA: {
                    if(packet.fd != appFd) {
                        error("SimpleClient::handleMessage(): Received packet from unknown socket!");
                    }
                    handleRealworldPacket(packet.data, packet.length);
                } break;
                case RealtimeScheduler::PacketBufferEntry::PACKET_FD_NEW: {
                    if(appFd != INVALID_SOCKET) {
                        error("SimpleClient::handleMessage(): Multiple connections not supported yet!");
                    }
                    if (packet.addr != NULL) {
                        delete packet.addr;
                        packet.addr = NULL;
                    }
                    appFd = packet.fd;
                } break;
                case RealtimeScheduler::PacketBufferEntry::PACKET_FD_CLOSE: {
                    if(packet.fd != appFd) {
                        error("SimpleClient::handleMessage(): Trying to close unknown connection!");
                    }
                    appFd = INVALID_SOCKET;
                } break;
                default:
                    // Unknown socket function, ignore
                    break;
            }
            delete packet.data;
        }
    }
    else if(msg->isName("Snowball landing")) {
        SCSnowTimer* snowTimer = check_and_cast<SCSnowTimer*>(msg);
        if(!frozen && position.distanceSqr(snowTimer->getPosition() / 32 ) < 2) {
            // freeze me!
            frozen = true;
            cMessage* unfreeze = new cMessage("unfreeze me");
            scheduleAt(simTime() + 5, unfreeze);

            GameAPIFrozenMessage* freeze = new GameAPIFrozenMessage("I'm frozen");
            freeze->setCommand(GAMEEVENT_FROZEN);
            freeze->setSrc(overlay->getThisNode());
            freeze->setThrower(snowTimer->getIp());
            freeze->setTimeSec(5);
            freeze->setTimeUsec(0);
            sendMessageToLowerTier(freeze);

            if(doRealworld) {
                SCFrozenPacket scfreeze;
                scfreeze.packetType = SC_EV_FROZEN;
                scfreeze.ip = overlay->getThisNode().getIp().get4().getInt();
                scfreeze.source = snowTimer->getIp();
                scfreeze.time_sec = 5;
                scfreeze.time_usec = 0;
                int packetlen = sizeof(scfreeze);
                scheduler->sendBytes((const char*)&packetlen, sizeof(int), 0, 0, true, appFd);
                scheduler->sendBytes((const char*)&scfreeze, packetlen, 0, 0, true, appFd);
            }
        }
        delete msg;
    }
    else if(msg->isName("unfreeze me")) {
        frozen = false;
        delete msg;
    }
}

void SimpleGameClient::handleLowerMessage(cMessage* msg)
{
    if(dynamic_cast<GameAPIMessage*>(msg)) {
        GameAPIMessage* gameAPIMsg = check_and_cast<GameAPIMessage*>(msg);
        if(gameAPIMsg->getCommand() == MOVEMENT_REQUEST) {
            updatePosition();
        }
        else if(gameAPIMsg->getCommand() == NEIGHBOR_UPDATE) {
            GameAPIListMessage* gameAPIListMsg = check_and_cast<GameAPIListMessage*>(msg);
            updateNeighbors(gameAPIListMsg);
        }
        else if(gameAPIMsg->getCommand() == RESIZE_AOI) {
            GameAPIResizeAOIMessage* gameAPIResizeMsg = check_and_cast<GameAPIResizeAOIMessage*>(msg);
            
            // Measure average AOI sizes
            if( startAOI != 0 )
            {
                RECORD_STATS(
                    simtime_t elapsed = simTime() - lastAOImeasure;
                    if( lastInHotspot ){
                        avgHotspotAOI += AOIWidth*elapsed.dbl();
                        hotspotTime += elapsed;
                    } else {
                        avgAOI += AOIWidth*elapsed.dbl();
                        nonHotspotTime += elapsed;
                        if( lastFarFromHotspot ){
                            avgFarFromHotspotAOI += AOIWidth*elapsed.dbl();
                            farFromHotspotTime += elapsed;
                        }
                    }
                );
            } else {
                startAOI = gameAPIResizeMsg->getAOIsize();
            }
            lastAOImeasure = simTime();
            if( useHotspots ){
                lastInHotspot = dynamic_cast<hotspotRoaming*>(Generator)->getDistanceFromHotspot() <= 0;
                lastFarFromHotspot = dynamic_cast<hotspotRoaming*>(Generator)->getDistanceFromHotspot() > startAOI;
            }

            AOIWidth = gameAPIResizeMsg->getAOIsize();
            if( logAOI ){
                GlobalStatisticsAccess().get()->recordOutVector( "SimpleGameClient: AOI width", AOIWidth );
            }
            if(doRealworld) {
                SCAOIPacket packet;
                packet.packetType = SC_RESIZE_AOI;
                packet.AOI = AOIWidth;
                int packetlen = sizeof(SCAOIPacket);
                scheduler->sendBytes((const char*)&packetlen, sizeof(int), 0, 0, true, appFd);
                scheduler->sendBytes((const char*)&packet, packetlen, 0, 0, true, appFd);
            }
        }
        else if(gameAPIMsg->getCommand() == GAMEEVENT_CHAT) {
            if(doRealworld) {
                GameAPIChatMessage* chatMsg = check_and_cast<GameAPIChatMessage*>(msg);
                const char* msg = chatMsg->getMsg();
                int packetlen = sizeof(SCChatPacket) + strlen(msg) + 1;
                SCChatPacket* pack = (SCChatPacket*)malloc(packetlen);
                pack->packetType = SC_EV_CHAT;
                pack->ip = chatMsg->getSrc().getIp().get4().getInt();
                strcpy(pack->msg, msg);
                scheduler->sendBytes((const char*)&packetlen, sizeof(int), 0, 0, true, appFd);
                scheduler->sendBytes((const char*)pack, packetlen, 0, 0, true, appFd);
                free(pack);
            }
        }
        else if(gameAPIMsg->getCommand() == GAMEEVENT_SNOW) {
            GameAPISnowMessage* snowMsg = check_and_cast<GameAPISnowMessage*>(gameAPIMsg);
            if(doRealworld) {
                SCSnowPacket packet;
                packet.packetType = SC_EV_SNOWBALL;
                packet.ip = snowMsg->getSrc().getIp().get4().getInt();
                packet.startX = snowMsg->getStart().x;
                packet.startY = snowMsg->getStart().y;
                packet.endX = snowMsg->getEnd().x;
                packet.endY = snowMsg->getEnd().y;
                packet.time_sec = snowMsg->getTimeSec();
                packet.time_usec = snowMsg->getTimeUsec();
                int packetlen = sizeof(SCSnowPacket);
                scheduler->sendBytes((const char*)&packetlen, sizeof(int), 0, 0, true, appFd);
                scheduler->sendBytes((const char*)&packet, packetlen, 0, 0, true, appFd);
            }
            SCSnowTimer* snowTimer = new SCSnowTimer("Snowball landing");
            snowTimer->setPosition(snowMsg->getEnd());
            snowTimer->setIp(snowMsg->getSrc().getIp().get4().getInt());
            timeval snowTime;
            snowTime.tv_sec = snowMsg->getTimeSec();
            snowTime.tv_usec = snowMsg->getTimeUsec();
            simtime_t snow_t = snowTime.tv_sec + snowTime.tv_usec / 1000000.0;
            scheduleAt(simTime() + snow_t, snowTimer);
        }
        else if(gameAPIMsg->getCommand() == GAMEEVENT_FROZEN) {
            if(doRealworld) {
                GameAPIFrozenMessage* frozenMsg = check_and_cast<GameAPIFrozenMessage*>(gameAPIMsg);
                SCFrozenPacket scfreeze;
                scfreeze.packetType = SC_EV_FROZEN;
                scfreeze.ip = frozenMsg->getSrc().getIp().get4().getInt();
		scfreeze.source = frozenMsg->getThrower();
                scfreeze.time_sec = frozenMsg->getTimeSec();
                scfreeze.time_usec = frozenMsg->getTimeUsec();
                int packetlen = sizeof(scfreeze);
                scheduler->sendBytes((const char*)&packetlen, sizeof(int), 0, 0, true, appFd);
                scheduler->sendBytes((const char*)&scfreeze, packetlen, 0, 0, true, appFd);
            }
        }
    }
    delete msg;
}

void SimpleGameClient::handleReadyMessage(CompReadyMessage* msg)
{
    // process only ready messages from the tier below
    if( getThisCompType() - msg->getComp() == 1 ){
        if(msg->getReady()) {
            overlayReady = true;
            if(!move_timer->isScheduled()) {
                scheduleAt(simTime() + movementDelay, move_timer);
            }
        }
        else {
            overlayReady = false;
        }
    }
    delete msg;
}

void SimpleGameClient::handleRealworldPacket(char *buf, uint32_t len)
{
    SCBasePacket *type = (SCBasePacket*)buf;
    if(type->packetType == SC_PARAM_REQUEST) {
        SCParamPacket packet;
        packet.packetType = SC_PARAM_RESPONSE;
        packet.speed = movementSpeed;
        packet.dimension = areaDimension;
        packet.AOI = AOIWidth;
        packet.delay = SIMTIME_DBL(movementDelay);
        packet.startX = position.x;
        packet.startY = position.y;
        packet.ip = thisNode.getIp().get4().getInt();
        packet.seed = coordinator->getSeed();
        int packetSize = sizeof(packet);
        scheduler->sendBytes((const char*)&packetSize, sizeof(int), 0, 0, true, appFd);
        scheduler->sendBytes((const char*)&packet, packetSize, 0, 0, true, appFd);
        doRealworld = true;
    }
    else if(type->packetType == SC_MOVE_INDICATION) {
        SCMovePacket *packet = (SCMovePacket*)type;
        Vector2D temp;
        temp.x = packet->posX;
        temp.y = packet->posY;
        dynamic_cast<realWorldRoaming*>(Generator)->setPosition(temp);
    }
    else if(type->packetType == SC_EV_CHAT) {
        SCChatPacket* packet = (SCChatPacket*)type;
        GameAPIChatMessage* chatMsg = new GameAPIChatMessage("ChatMsg");
        chatMsg->setCommand(GAMEEVENT_CHAT);
        chatMsg->setSrc(thisNode);
        chatMsg->setMsg(packet->msg);
        sendMessageToLowerTier(chatMsg);
    }
    else if(type->packetType == SC_EV_SNOWBALL) {
        SCSnowPacket* packet = (SCSnowPacket*)type;
        GameAPISnowMessage* snowMsg = new GameAPISnowMessage("Throw Snowball");
        snowMsg->setCommand(GAMEEVENT_SNOW);
        snowMsg->setSrc(thisNode);
        snowMsg->setStart(Vector2D(packet->startX, packet->startY));
        snowMsg->setEnd(Vector2D(packet->endX, packet->endY));
        snowMsg->setTimeSec(packet->time_sec);
        snowMsg->setTimeUsec(packet->time_usec);
        sendMessageToLowerTier(snowMsg);
    }
}

void SimpleGameClient::updateNeighbors(GameAPIListMessage* sgcMsg)
{
    unsigned int i;

    if(doRealworld) {
        int packetSize;

        SCRemovePacket removePacket;
        removePacket.packetType = SC_REMOVE_NEIGHBOR;
        packetSize = sizeof(removePacket);

        for(i=0; i<sgcMsg->getRemoveNeighborArraySize(); ++i) {
            removePacket.ip = sgcMsg->getRemoveNeighbor(i).getIp().get4().getInt();
            scheduler->sendBytes((const char*)&packetSize, sizeof(int), 0, 0, true, appFd);
            scheduler->sendBytes((const char*)&removePacket, packetSize, 0, 0, true, appFd);
        }

        SCAddPacket addPacket;
        addPacket.packetType = SC_ADD_NEIGHBOR;
        packetSize = sizeof(addPacket);

        for(i=0; i<sgcMsg->getAddNeighborArraySize(); ++i) {
            addPacket.ip = sgcMsg->getAddNeighbor(i).getIp().get4().getInt();
            if( addPacket.ip == thisNode.getIp().get4().getInt() ) continue;
            addPacket.posX = sgcMsg->getNeighborPosition(i).x;
            addPacket.posY = sgcMsg->getNeighborPosition(i).y;
            scheduler->sendBytes((const char*)&packetSize, sizeof(int), 0, 0, true, appFd);
            scheduler->sendBytes((const char*)&addPacket, packetSize, 0, 0, true, appFd);
        }
    }
    for(i=0; i<sgcMsg->getRemoveNeighborArraySize(); ++i)
        Neighbors.erase(sgcMsg->getRemoveNeighbor(i));

    for(i=0; i<sgcMsg->getAddNeighborArraySize(); ++i) {
        Vector2D newPos;
        newPos = sgcMsg->getNeighborPosition(i);
        itNeighbors = Neighbors.find(sgcMsg->getAddNeighbor(i));
        if(itNeighbors != Neighbors.end()) {
            Vector2D temp = newPos - itNeighbors->second.position;
            temp.normalize();
            itNeighbors->second.direction = temp;
            itNeighbors->second.position = newPos;
        }
        else {
            NeighborMapEntry entry;
            entry.position = newPos;
            Neighbors.insert(std::make_pair(sgcMsg->getAddNeighbor(i), entry));
        }
    }
}

void SimpleGameClient::updatePosition()
{
    if(!frozen) {
        Generator->move();
    }
    position = Generator->getPosition();
    GameAPIPositionMessage *posMsg = new GameAPIPositionMessage("MOVEMENT_INDICATION");
    posMsg->setCommand(MOVEMENT_INDICATION);
    posMsg->setPosition(position);
    sendMessageToLowerTier(posMsg);
}

void SimpleGameClient::finishApp()
{
    if (nonHotspotTime >= GlobalStatistics::MIN_MEASURED) {
        globalStatistics->addStdDev("SimpleGameClient: average non-hotspot AOI",
                avgAOI / nonHotspotTime);
    }
    if (farFromHotspotTime >= GlobalStatistics::MIN_MEASURED) {
        globalStatistics->addStdDev("SimpleGameClient: average far-from-hotspot AOI",
                avgFarFromHotspotAOI / farFromHotspotTime);
    }
    if (hotspotTime >= GlobalStatistics::MIN_MEASURED) {
        globalStatistics->addStdDev("SimpleGameClient: average hotspot AOI",
                avgHotspotAOI / hotspotTime);
    }
}

SimpleGameClient::~SimpleGameClient()
{
    // destroy self timer messages
    cancelAndDelete(move_timer);
    cancelAndDelete(packetNotification);
    delete Generator;
}
