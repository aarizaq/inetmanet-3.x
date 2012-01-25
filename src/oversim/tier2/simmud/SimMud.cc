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
 * @file SimMud.cc
 * @author Stephan Krause
 */

#include "SimMud.h"
#include "ScribeMessage_m.h"
#include <limits.h>
#include <GlobalStatistics.h>

Define_Module(SimMud);

using namespace std;

SimMud::SimMud()
{
    currentRegionX = currentRegionY = INT_MIN;
    currentRegionID = OverlayKey::UNSPECIFIED_KEY;
    playerTimer = new cMessage("playerTimeout");
}

SimMud::~SimMud()
{
    cancelAndDelete(playerTimer);
}

void SimMud::finishApp()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->addStdDev("SimMUD: Lost or too long delayed MoveLists/s",
                                lostMovementLists / time);
    globalStatistics->addStdDev("SimMUD: Received valid MoveLists/s",
                                receivedMovementLists / time);
}

void SimMud::initializeApp(int stage)
{
    if( stage != (numInitStages()-1)) {
        return;
    }

    // FIXME: areaDimension is not used consistently between all overlays!
    fieldSize = par("areaDimension");
    numSubspaces = par("numSubspaces");
    regionSize = fieldSize / numSubspaces;
    playerTimeout = par("playerTimeout");

    receivedMovementLists = 0;
    lostMovementLists = 0;

    maxMoveDelay = par("maxMoveDelay");
    AOIWidth = par("AOIWidth");

    WATCH(currentRegionX);
    WATCH(currentRegionY);
    WATCH(currentRegionID);
//    WATCH_MAP( playerMap );
}

bool SimMud::handleRpcCall( BaseCallMessage* msg )
{
//    RPC_SWITCH_START(msg);
//    RPC_SWITCH_END( );
//    return RPC_HANDLED;
      return false;
}

void SimMud::handleRpcResponse( BaseResponseMessage* msg,
                                cPolymorphic* context,
                                int rpcId, simtime_t rtt )
{
//    RPC_SWITCH_START(msg);
//    RPC_SWITCH_END( );
}

void SimMud::handleTimerEvent( cMessage *msg )
{
    if( msg == playerTimer ) {
        // Check if any player didn't send any updates since last check
        map<NodeHandle, PlayerInfo>::iterator it;
        list<NodeHandle> erasePlayers;
        for( it = playerMap.begin(); it != playerMap.end(); ++it ) {
            if( it->second.update == false ) {
                erasePlayers.push_back( it->first );
            }
            it->second.update = false;
        }
        for( list<NodeHandle>::iterator it = erasePlayers.begin(); it != erasePlayers.end(); ++it) {
            // Delete all neighbors that didn't update (and inform app)
            GameAPIListMessage *scMsg = new GameAPIListMessage("NEIGHBOR_UPDATE");
            scMsg->setCommand(NEIGHBOR_UPDATE);
            scMsg->setRemoveNeighborArraySize(1);
            scMsg->setRemoveNeighbor(0, *it);
            send(scMsg, "to_upperTier");

            playerMap.erase( *it );
        }
        erasePlayers.clear();

        scheduleAt( simTime() + playerTimeout, msg );
    }
}

void SimMud::handleLowerMessage( cMessage *msg )
{
    if (ALMMulticastMessage* mcastMsg =
            dynamic_cast<ALMMulticastMessage*>(msg) ){

        cMessage* innerMsg = mcastMsg->decapsulate();
        SimMudMoveMessage* moveMsg = NULL;
        if( innerMsg ) {
                moveMsg = dynamic_cast<SimMudMoveMessage*>(innerMsg);
        }
        if( moveMsg ) {
            handleOtherPlayerMove( moveMsg );
        }
        delete innerMsg;
        delete mcastMsg;
    }
}

void SimMud::handleReadyMessage( CompReadyMessage *msg )
{
    // process only ready messages from the tier below
    if( getThisCompType() - msg->getComp() == 1 ){
        if( msg->getReady() ) {
            // TODO/FIXME: use overlay->sendMessageToAllComp(msg, getThisCompType())?
            msg->setComp(getThisCompType());
            send(msg, "to_upperTier");
            // also send AOI size to SimpleGameClient
            GameAPIResizeAOIMessage* gameMsg = new GameAPIResizeAOIMessage("RESIZE_AOI");
            gameMsg->setCommand(RESIZE_AOI);
            gameMsg->setAOIsize(AOIWidth);
            send(gameMsg, "to_upperTier");

            if( playerTimeout > 0 ) {
                cancelEvent( playerTimer );
                scheduleAt( simTime() + playerTimeout, playerTimer );
            }
        }
    } else {
        delete msg;
    }
}

void SimMud::handleUpperMessage( cMessage *msg )
{
    if( GameAPIPositionMessage* moveMsg =
        dynamic_cast<GameAPIPositionMessage*>(msg) ) {
        handleMove( moveMsg );
        delete msg;
    }
}

void SimMud::handleMove( GameAPIPositionMessage* msg )
{
    if( (int) (msg->getPosition().x/regionSize) != currentRegionX ||
            (int) (msg->getPosition().y/regionSize) != currentRegionY ) {
        // New position is outside of current region; change currentRegion

        // get region ID
        currentRegionX = (int) (msg->getPosition().x/regionSize);
        currentRegionY = (int) (msg->getPosition().y/regionSize);
        std::stringstream regionstr;
        regionstr << currentRegionX << ":" << currentRegionY;
        OverlayKey region = OverlayKey::sha1( BinaryValue(regionstr.str() ));
        currentRegionID = region;
    }

    set<OverlayKey> expectedRegions;
    set<OverlayKey> allowedRegions;
    int minX = (int) ((msg->getPosition().x - AOIWidth)/regionSize);
    if( minX < 0 ) minX = 0;
    int maxX = (int) ((msg->getPosition().x + AOIWidth)/regionSize);
    if( maxX >= numSubspaces ) maxX = numSubspaces -1;
    int minY = (int) ((msg->getPosition().y - AOIWidth)/regionSize);
    if( minY < 0 ) minY = 0;
    int maxY = (int) ((msg->getPosition().y + AOIWidth)/regionSize);
    if( maxY >= numSubspaces ) maxY = numSubspaces -1;

    // FIXME: make parameter: unsubscription size
    int minUnsubX = (int) ((msg->getPosition().x - 1.5*AOIWidth)/regionSize);
    if( minUnsubX < 0 ) minUnsubX = 0;
    int maxUnsubX = (int) ((msg->getPosition().x + 1.5*AOIWidth)/regionSize);
    if( maxUnsubX >= numSubspaces ) maxUnsubX = numSubspaces -1;
    int minUnsubY = (int) ((msg->getPosition().y - 1.5*AOIWidth)/regionSize);
    if( minUnsubY < 0 ) minUnsubY = 0;
    int maxUnsubY = (int) ((msg->getPosition().y + 1.5+AOIWidth)/regionSize);
    if( maxUnsubY >= numSubspaces ) maxUnsubY = numSubspaces -1;

    for( int x = minUnsubX; x <= maxUnsubX; ++x ){
        for( int y = minUnsubY; y <= maxUnsubY; ++y ){
            std::stringstream regionstr;
            regionstr << x << ":" << y;
            if( x >= minX && x <=maxX && y >= minY && y <= maxY ){
                expectedRegions.insert( OverlayKey::sha1( BinaryValue(regionstr.str() )));
            }
            allowedRegions.insert( OverlayKey::sha1( BinaryValue(regionstr.str() )));
        }
    }

    set<OverlayKey>::iterator subIt = subscribedRegions.begin();
    while( subIt != subscribedRegions.end() ){

        expectedRegions.erase( *subIt );

        // unsubscribe region if to far away
        if( allowedRegions.find( *subIt ) == allowedRegions.end() ){
            // Inform other players about region leave
            SimMudMoveMessage* moveMsg = new SimMudMoveMessage("MOVE/LEAVE_REGION");
            moveMsg->setSrc( overlay->getThisNode() );
            moveMsg->setPosX( msg->getPosition().x );
            moveMsg->setPosY( msg->getPosition().y );
            moveMsg->setTimestamp( simTime() );
            moveMsg->setLeaveRegion( true );
            ALMMulticastMessage* mcastMsg = new ALMMulticastMessage("MOVE/LEAVE_REGION");
            mcastMsg->setGroupId(*subIt);
            mcastMsg->encapsulate( moveMsg );

            send(mcastMsg, "to_lowerTier");

             // leave old region's multicastGroup
            ALMLeaveMessage* leaveMsg = new ALMLeaveMessage("LEAVE_REGION_GROUP");
            leaveMsg->setGroupId(*subIt);
            send(leaveMsg, "to_lowerTier");
            // TODO: leave old simMud region

           // Erase subspace from subscribedList and increase iterator
            subscribedRegions.erase( subIt++ );
        } else {
            ++subIt;
        }
    }

    // if any "near" region is not yet subscribed, subscribe
    for( set<OverlayKey>::iterator regionIt = expectedRegions.begin(); regionIt != expectedRegions.end(); ++regionIt ){
        // join region's multicast group
        ALMSubscribeMessage* subMsg = new ALMSubscribeMessage;
        subMsg->setGroupId(*regionIt);
        send( subMsg, "to_lowerTier" );

        subscribedRegions.insert( *regionIt );
        // TODO: join simMud region
    }

    // publish movement
    SimMudMoveMessage* moveMsg = new SimMudMoveMessage("MOVE");
    moveMsg->setSrc( overlay->getThisNode() );
    moveMsg->setPosX( msg->getPosition().x );
    moveMsg->setPosY( msg->getPosition().y );
    moveMsg->setTimestamp( simTime() );
    moveMsg->setBitLength( SIMMUD_MOVE_L( moveMsg ));
    ALMMulticastMessage* mcastMsg = new ALMMulticastMessage("MOVE");
    mcastMsg->setGroupId(currentRegionID);
    mcastMsg->encapsulate( moveMsg );

    send(mcastMsg, "to_lowerTier");
}

void SimMud::handleOtherPlayerMove( SimMudMoveMessage* msg )
{
    GameAPIListMessage *scMsg = new GameAPIListMessage("NEIGHBOR_UPDATE");
    scMsg->setCommand(NEIGHBOR_UPDATE);

    NodeHandle& src = msg->getSrc();
    RECORD_STATS(
            if( msg->getTimestamp() < simTime() - maxMoveDelay ){
                ++lostMovementLists;
                globalStatistics->addStdDev("SimMUD: LOST MOVE Delay",
                    SIMTIME_DBL(simTime() - msg->getTimestamp()) );
            } else {
                ++receivedMovementLists;
            }

            if( src != overlay->getThisNode() ){
                globalStatistics->addStdDev("SimMUD: MOVE Delay",
                    SIMTIME_DBL(simTime() - msg->getTimestamp()) );
            }
            );

    if( msg->getLeaveRegion() ) {
        // Player leaves region
        scMsg->setRemoveNeighborArraySize(1);
        scMsg->setRemoveNeighbor(0, src);
        playerMap.erase( src );

    } else {
        PlayerInfo player;
        player.pos = Vector2D( msg->getPosX(), msg->getPosY() );
        player.update = true;

        pair< map<NodeHandle, PlayerInfo>::iterator, bool> inserter =
            playerMap.insert( make_pair(src, player) );

        /*    if( inserter.second ) {
            // new player

        } else {
            // move player
        }*/

        // Ordinary move
        scMsg->setAddNeighborArraySize(1);
        scMsg->setNeighborPositionArraySize(1);
        scMsg->setAddNeighbor(0, src);
        scMsg->setNeighborPosition(0, Vector2D(msg->getPosX(), msg->getPosY()) );
    }
    send(scMsg, "to_upperTier");

}

