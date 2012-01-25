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
 * @file PubSubMMOG.cc
 * @author Stephan Krause
 */

#include <NotifierConsts.h>

#include "PubSubMMOG.h"

#include "GlobalNodeListAccess.h"
#include <GlobalStatistics.h>
#include <BootstrapList.h>

Define_Module(PubSubMMOG);

using namespace std;

void PubSubMMOG::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if(stage != MIN_STAGE_OVERLAY) return;

    state = INIT;
    setBootstrapedIcon();
    // TODO: use BootstrapList instead (but this currently preferes
    //       nodes from the same partition)
    lobbyServer = globalNodeList->getBootstrapNode();

    joinTimer = new cMessage("join timer");
    simtime_t joinTime = ceil(simTime() + (simtime_t) par("joinDelay"));
    scheduleAt( joinTime, joinTimer );

    movementRate = par("movementRate");
    eventDeliveryTimer = new PubSubTimer("event delivery timer");
    eventDeliveryTimer->setType( PUBSUB_EVENTDELIVERY );
    scheduleAt( joinTime + 1.0/(2*movementRate), eventDeliveryTimer );

    numSubspaces = par("numSubspaces");
    subspaceSize = (int) ( (unsigned int) par("areaDimension") / numSubspaces);
    thisNode.setKey( OverlayKey::random() );

    maxChildren = par("maxChildren");
    PubSubSubspaceResponsible::maxChildren = maxChildren;

    AOIWidth = par("AOIWidth");
    maxMoveDelay = par("maxMoveDelay");

    parentTimeout = par("parentTimeout");
    heartbeatTimer = new PubSubTimer("HeartbeatTimer");
    heartbeatTimer->setType( PUBSUB_HEARTBEAT );
    startTimer( heartbeatTimer );
    childPingTimer = new PubSubTimer("ChildPingTimer");
    childPingTimer->setType( PUBSUB_CHILDPING );
    startTimer( childPingTimer );

    allowOldMoveMessages  = par("allowOldMoveMessages");

    numEventsWrongTimeslot = numEventsCorrectTimeslot = 0;
    numPubSubSignalingMessages = 0;
    pubSubSignalingMessagesSize = 0;
    numMoveMessages = 0;
    moveMessagesSize = 0;
    numMoveListMessages = 0;
    moveListMessagesSize = 0;
    respMoveListMessagesSize = 0;
    lostMovementLists = 0;
    receivedMovementLists = 0;
    WATCH( numPubSubSignalingMessages );
    WATCH( pubSubSignalingMessagesSize );
    WATCH( numMoveMessages );
    WATCH( moveMessagesSize );
    WATCH( numMoveListMessages );
    WATCH( moveListMessagesSize );
    WATCH( numEventsWrongTimeslot );
    WATCH( numEventsCorrectTimeslot );
    WATCH( lostMovementLists );
    WATCH( receivedMovementLists );
    WATCH_LIST( subscribedSubspaces );
    WATCH_MAP( responsibleSubspaces );
    WATCH_MAP( backupSubspaces );
    WATCH_MAP( intermediateSubspaces );
}

bool PubSubMMOG::handleRpcCall(BaseCallMessage* msg)
{
    // delegate messages
    RPC_SWITCH_START( msg )
    RPC_DELEGATE( PubSubSubscription, handleSubscriptionCall );
    RPC_DELEGATE( PubSubTakeOverSubspace, handleTakeOver );
    RPC_DELEGATE( PubSubBackup, handleBackupCall );
    RPC_DELEGATE( PubSubIntermediate, handleIntermediateCall );
    RPC_DELEGATE( PubSubAdoptChild, handleAdoptChildCall );
    RPC_DELEGATE( PubSubPing, handlePingCall );
    RPC_SWITCH_END( )

    return RPC_HANDLED;

}

void PubSubMMOG::handleRpcResponse(BaseResponseMessage *msg,
                                   cPolymorphic* context, int rpcId,
                                   simtime_t rtt)
{
    RPC_SWITCH_START(msg);
    RPC_ON_RESPONSE( PubSubJoin ) {
        handleJoinResponse( _PubSubJoinResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubJoin RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubJoinResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( PubSubSubscription ) {
        handleSubscriptionResponse( _PubSubSubscriptionResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubSubscription RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubSubscriptionResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( PubSubResponsibleNode ) {
        handleResponsibleNodeResponse( _PubSubResponsibleNodeResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubResponsibleNode RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubResponsibleNodeResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( PubSubHelp ) {
        handleHelpResponse( _PubSubHelpResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubHelp RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubHelpResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( PubSubBackup ) {
        handleBackupResponse( _PubSubBackupResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubBackup RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubBackupResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( PubSubIntermediate ) {
        handleIntermediateResponse( _PubSubIntermediateResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubIntermediate RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubIntermediateResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( PubSubAdoptChild ) {
        handleAdoptChildResponse( _PubSubAdoptChildResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubAdoptChild RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubAdoptChildResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( PubSubPing ) {
        handlePingResponse( _PubSubPingResponse );
        EV << "[PubSubMMOG::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a PubSubPing RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_PubSubPingResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_SWITCH_END( );
}

void PubSubMMOG::handleRpcTimeout (BaseCallMessage *msg,
                                   const TransportAddress &dest,
                                   cPolymorphic* context, int rpcId,
                                   const OverlayKey &destKey)
{
    RPC_SWITCH_START(msg)
    RPC_ON_CALL( PubSubBackup ) {
        handleBackupCallTimeout( _PubSubBackupCall, dest );
        EV << "[PubSubMMOG::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Backup RPC Call timed out: id=" << rpcId << "\n"
           << "    msg=" << *_PubSubBackupCall
           << "    oldNode=" << dest
           << endl;
        break;
    }
    RPC_ON_CALL( PubSubPing ) {
        handlePingCallTimeout( _PubSubPingCall, dest );
        EV << "[PubSubMMOG::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Ping RPC Call timed out: id=" << rpcId << "\n"
           << "    msg=" << *_PubSubPingCall
           << "    oldNode=" << dest
           << endl;
        break;
    }
    RPC_ON_CALL( PubSubSubscription ) {
        handleSubscriptionCallTimeout( _PubSubSubscriptionCall, dest );
        EV << "[PubSubMMOG::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Subscription RPC Call timed out: id=" << rpcId << "\n"
           << "    msg=" << *_PubSubSubscriptionCall
           << "    oldNode=" << dest
           << endl;
        break;
    }
    RPC_SWITCH_END( )

    // FIXME:
    //   AdoptCall missing!
    //   IntermediateCall missing!
    //   (ResponsibleNodeCall missing)
    //   (HelpCall missing)
}

void PubSubMMOG::handleUDPMessage(BaseOverlayMessage* msg)
{
    if( PubSubMoveListMessage* moveMsg = dynamic_cast<PubSubMoveListMessage*>(msg) ){
        handleMoveListMessage( moveMsg );
        delete moveMsg;
    } else if( PubSubMoveMessage* moveMsg = dynamic_cast<PubSubMoveMessage*>(msg) ){
        handleMoveMessage( moveMsg );
    } else if( PubSubUnsubscriptionMessage* unsMsg = dynamic_cast<PubSubUnsubscriptionMessage*>(msg) ){
        handleUnsubscriptionMessage( unsMsg );
        delete unsMsg;
    } else if( PubSubNodeLeftMessage* leftMsg = dynamic_cast<PubSubNodeLeftMessage*>(msg) ){
        handleNodeLeftMessage( leftMsg );
        delete leftMsg;
    } else if( PubSubReplacementMessage* replaceMsg = dynamic_cast<PubSubReplacementMessage*>(msg) ){
        handleReplacementMessage( replaceMsg );
        delete replaceMsg;
    } else if( PubSubBackupSubscriptionMessage* backupMsg = dynamic_cast<PubSubBackupSubscriptionMessage*>(msg) ){
        handleSubscriptionBackup( backupMsg );
        delete backupMsg;
    } else if( PubSubBackupUnsubscribeMessage* backupMsg = dynamic_cast<PubSubBackupUnsubscribeMessage*>(msg) ){
        handleUnsubscribeBackup( backupMsg );
        delete backupMsg;
    } else if( PubSubBackupIntermediateMessage* backupMsg = dynamic_cast<PubSubBackupIntermediateMessage*>(msg) ){
        handleIntermediateBackup( backupMsg );
        delete backupMsg;
    } else if( PubSubReleaseIntermediateMessage* releaseMsg = dynamic_cast<PubSubReleaseIntermediateMessage*>(msg) ){
        handleReleaseIntermediate( releaseMsg );
        delete releaseMsg;
    }
}

void PubSubMMOG::handleTimerEvent(cMessage* msg)
{
    if( PubSubTimer* timer = dynamic_cast<PubSubTimer*>(msg) ) {
        switch( timer->getType() ) {
            case PUBSUB_HEARTBEAT:
                sendHearbeatToChildren();
                startTimer( timer );
                break;
            case PUBSUB_CHILDPING:
                sendPingToChildren();
                startTimer( timer );
                break;
            case PUBSUB_PARENT_TIMEOUT:
                handleParentTimeout( timer );
                break;
            case PUBSUB_EVENTDELIVERY:
                publishEvents();
                startTimer( timer );
                break;
        }
    } else if( msg == joinTimer ) {
        // send a fake ready message to app to get initial position
        // Note: This is not consistent to the paper, where the lobby server
        // positions player. But it is needed for consistency with other MMOG protocols
        CompReadyMessage* msg = new CompReadyMessage("fake READY");
        msg->setReady(true);
        msg->setComp(getThisCompType());
        send( msg, "appOut");
        delete joinTimer;
        joinTimer = NULL;
        // send initial AOI size to the application
        // Note: This is not consistent to the paper.
        // But it is needed for this nodes movement generation within the application layer.
        GameAPIResizeAOIMessage* gameMsg = new GameAPIResizeAOIMessage("RESIZE_AOI");
        gameMsg->setCommand(RESIZE_AOI);
        gameMsg->setAOIsize(AOIWidth);
        send( gameMsg, "appOut");
    }
}

void PubSubMMOG::handleAppMessage(cMessage* msg)
{
    if( GameAPIPositionMessage *posMsg = dynamic_cast<GameAPIPositionMessage*>(msg) ) {
        if( state == READY ) {
            handleMove( posMsg );
        } else if ( state == JOINING ) {
            // We are not connected to our responsible node, inform app
            CompReadyMessage* msg = new CompReadyMessage("Overlay not READY!");
            msg->setReady(false);
            msg->setComp(getThisCompType());
            send( msg, "appOut");
        } else if ( state == INIT ) {
            // This is only called for the first MOVE message
            // Trigger login
            PubSubJoinCall* joinMsg = new PubSubJoinCall("Login");
            joinMsg->setPosition( posMsg->getPosition() );
            // FIXME: Ressource handling not yet supported!
            joinMsg->setRessources( 4 );
            sendUdpRpcCall( lobbyServer, joinMsg );

            state = JOINING;
            setBootstrapedIcon();

            // tell app to wait until login is confirmed...
            CompReadyMessage* readyMsg = new CompReadyMessage("Overlay not READY!");
            readyMsg->setReady(false);
            readyMsg->setComp(getThisCompType());
            send( readyMsg, "appOut");

            currentRegionX = (unsigned int) (posMsg->getPosition().x/subspaceSize);
            currentRegionY = (unsigned int) (posMsg->getPosition().y/subspaceSize);
        }
        delete msg;
    }
}

void PubSubMMOG::handleSubscriptionResponse( PubSubSubscriptionResponse* subResp )
{
    if( subResp->getFailed() ) {
        // TODO: get new resp node...
    } else {
        if( state != READY ){
            state = READY;
            setBootstrapedIcon();
            CompReadyMessage* readyMsg = new CompReadyMessage("Overlay READY!");
            readyMsg->setReady(true);
            readyMsg->setComp(getThisCompType());
            sendDelayed( readyMsg, ceil(simTime()) - simTime(), "appOut" );
        }
    }
}

void PubSubMMOG::handleJoinResponse( PubSubJoinResponse* joinResp )
{
    state = JOINING;
    setBootstrapedIcon();
    PubSubSubspaceId region( currentRegionX, currentRegionY, numSubspaces);

    NodeHandle respNode = joinResp->getResponsibleNode();
    PubSubSubspace sub(region);
    sub.setResponsibleNode( respNode );
    subscribedSubspaces.push_back( sub );
    if( respNode.isUnspecified() ) {
        PubSubResponsibleNodeCall* respCall = new PubSubResponsibleNodeCall("Request Responsible NodeHandle");
        respCall->setSubspacePos( Vector2D(currentRegionX, currentRegionY) );
        respCall->setBitLength( PUBSUB_RESPONSIBLENODECALL_L( respCall ) );
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= respCall->getByteLength()
                );
        sendUdpRpcCall( lobbyServer, respCall, NULL, 5, 5 ); // FIXME: Make it a parameter...
    } else {
        PubSubSubscriptionCall* subCall = new PubSubSubscriptionCall("JoinSubspace");
        subCall->setSubspaceId( region.getId() );
        subCall->setBitLength( PUBSUB_SUBSCRIPTIONCALL_L( subCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= subCall->getByteLength()
                );
        sendUdpRpcCall( respNode, subCall );
    }
}

void PubSubMMOG::handleResponsibleNodeResponse( PubSubResponsibleNodeResponse* subResp )
{
    int subspaceId = subResp->getSubspaceId();
    NodeHandle respNode = subResp->getResponsibleNode();

    std::list<PubSubSubspace>::iterator it = subscribedSubspaces.begin();
    while( it != subscribedSubspaces.end() ) {
        if( it->getId().getId() == subspaceId) break;
        ++it;
    }
    if( it != subscribedSubspaces.end() ) {
        it->setResponsibleNode( respNode );

        PubSubSubscriptionCall* subCall = new PubSubSubscriptionCall("JoinSubspace");
        subCall->setSubspaceId( subspaceId );
        subCall->setBitLength( PUBSUB_SUBSCRIPTIONCALL_L( subCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= subCall->getByteLength()
                );
        sendUdpRpcCall( respNode, subCall );
    }
}

void PubSubMMOG::handleUnsubscriptionMessage( PubSubUnsubscriptionMessage* unsMsg )
{
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = responsibleSubspaces.find( PubSubSubspaceId(unsMsg->getSubspaceId(), numSubspaces) );

    if( it != responsibleSubspaces.end() ) {
        unsubscribeChild( unsMsg->getSrc(), it->second );
    }
}

void PubSubMMOG::handleNodeLeftMessage( PubSubNodeLeftMessage* leftMsg )
{
    std::map<PubSubSubspaceId, PubSubSubspaceIntermediate>::iterator it;
    it = intermediateSubspaces.find( PubSubSubspaceId(leftMsg->getSubspaceId(), numSubspaces) );

    if( it == intermediateSubspaces.end() ) return;

    it->second.removeChild( leftMsg->getNode() );
}

void PubSubMMOG::handleSubscriptionCall( PubSubSubscriptionCall* subCall )
{
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = responsibleSubspaces.find( PubSubSubspaceId(subCall->getSubspaceId(), numSubspaces) );

    PubSubBackupSubscriptionMessage* backupMsg = 0;
    PubSubSubscriptionResponse* resp;
    if( it == responsibleSubspaces.end() ) {
        resp = new PubSubSubscriptionResponse("Subscription failed");
        resp->setFailed( true );
    } else {
        resp = new PubSubSubscriptionResponse("Subscription successful");
        backupMsg = new PubSubBackupSubscriptionMessage("Backup: new subscription");
        backupMsg->setSubspaceId( subCall->getSubspaceId() );
        backupMsg->setChild( subCall->getSrcNode() );

        if( it->second.addChild( subCall->getSrcNode() )) {
            // We have still room for the child
            backupMsg->setParent( thisNode );
        } else {
            // Child has to go to an intermediate node...
            if( PubSubSubspaceResponsible::IntermediateNode* iNode = it->second.getNextFreeIntermediate() ){
                // find intermediate node with free slots
                PubSubAdoptChildCall* adoptCall = new PubSubAdoptChildCall("Adopt child");
                adoptCall->setChild( subCall->getSrcNode() );
                adoptCall->setSubspaceId( subCall->getSubspaceId() );
                adoptCall->setBitLength( PUBSUB_ADOPTCHILDCALL_L( adoptCall ));
                sendUdpRpcCall( iNode->node, adoptCall );
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= adoptCall->getByteLength()
                        );
                iNode->waitingChildren++;
            } else {
                // no free slots available, create new intermediate node
                // FIXME: when getting two subscriptions at once, we're requesting too many intermediates
                PubSubHelpCall* helpCall = new PubSubHelpCall("I need an intermediate node");
                helpCall->setHelpType( PUBSUB_INTERMEDIATE );
                helpCall->setSubspaceId( subCall->getSubspaceId() );
                helpCall->setBitLength( PUBSUB_HELPCALL_L( helpCall ));
                sendUdpRpcCall( lobbyServer, helpCall );
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= helpCall->getByteLength()
                        );
            }
        }
    }
    resp->setBitLength( PUBSUB_SUBSCRIPTIONRESPONSE_L( resp ));
    sendRpcResponse( subCall, resp );
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= resp->getByteLength()
            );

    if( it == responsibleSubspaces.end() ) return;

// FIXME: just for testing
PubSubSubspaceResponsible& subspace = it->second;
int iii = subspace.getTotalChildrenCount();
subspace.fixTotalChildrenCount();
if( iii != subspace.getTotalChildrenCount() ){
    opp_error("Huh?");
}

    if( !it->second.getBackupNode().isUnspecified() ){
        backupMsg->setBitLength( PUBSUB_BACKUPSUBSCRIPTION_L( backupMsg ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                );
        sendMessageToUDP( it->second.getBackupNode(), backupMsg );
    } else {
        delete backupMsg;
    }
}

void PubSubMMOG::handleTakeOver( PubSubTakeOverSubspaceCall* toCall )
{
    PubSubSubspaceId region((int) toCall->getSubspacePos().x, (int) toCall->getSubspacePos().y, numSubspaces);

    takeOverNewSubspace( region );

    PubSubTakeOverSubspaceResponse* toResp = new PubSubTakeOverSubspaceResponse("Accept subspace responsibility");
    toResp->setSubspacePos( toCall->getSubspacePos() );
    toResp->setBitLength( PUBSUB_TAKEOVERSUBSPACERESPONSE_L( toResp ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= toResp->getByteLength()
            );
    sendRpcResponse( toCall, toResp );
}

void PubSubMMOG::receiveChangeNotification(int category, const cPolymorphic *details)
{
    if(category == NF_OVERLAY_NODE_GRACEFUL_LEAVE && state == READY) {
    }
}

void PubSubMMOG::handleMove( GameAPIPositionMessage* posMsg )
{
    currentRegionX = (unsigned int) (posMsg->getPosition().x/subspaceSize);
    currentRegionY = (unsigned int) (posMsg->getPosition().y/subspaceSize);

    PubSubSubspaceId region( currentRegionX, currentRegionY, numSubspaces);

    set<PubSubSubspaceId> expectedRegions;
    int minX = (int) ((posMsg->getPosition().x - AOIWidth)/subspaceSize);
    if( minX < 0 ) minX = 0;
    int maxX = (int) ((posMsg->getPosition().x + AOIWidth)/subspaceSize);
    if( maxX >= numSubspaces ) maxX = numSubspaces -1;
    int minY = (int) ((posMsg->getPosition().y - AOIWidth)/subspaceSize);
    if( minY < 0 ) minY = 0;
    int maxY = (int) ((posMsg->getPosition().y + AOIWidth)/subspaceSize);
    if( maxY >= numSubspaces ) maxY = numSubspaces -1;

    // FIXME: make parameter: unsubscription size
    int minUnsubX = (int) ((posMsg->getPosition().x - 1.5*AOIWidth)/subspaceSize);
    if( minUnsubX < 0 ) minUnsubX = 0;
    int maxUnsubX = (int) ((posMsg->getPosition().x + 1.5*AOIWidth)/subspaceSize);
    if( maxUnsubX >= numSubspaces ) maxUnsubX = numSubspaces -1;
    int minUnsubY = (int) ((posMsg->getPosition().y - 1.5*AOIWidth)/subspaceSize);
    if( minUnsubY < 0 ) minUnsubY = 0;
    int maxUnsubY = (int) ((posMsg->getPosition().y + 1.5+AOIWidth)/subspaceSize);
    if( maxUnsubY >= numSubspaces ) maxUnsubY = numSubspaces -1;

    for( int x = minX; x <= maxX; ++x ){
        for( int y = minY; y <= maxY; ++y ){
            expectedRegions.insert( PubSubSubspaceId( x, y, numSubspaces ));
        }
    }

    list<PubSubSubspace>::iterator subIt = subscribedSubspaces.begin();
    PubSubSubspace* subspace = NULL;
    while( subIt != subscribedSubspaces.end() ){
        if( subIt->getId() == region ){
            subspace = &*subIt;
        }
        expectedRegions.erase( subIt->getId() );

        // unsubscribe region if to far away
        if( subIt->getId().getX() < minX || subIt->getId().getX() > maxX ||
                subIt->getId().getY() < minY || subIt->getId().getY() > maxY ){
            if( !subIt->getResponsibleNode().isUnspecified() ){
                PubSubUnsubscriptionMessage* unsubMsg = new PubSubUnsubscriptionMessage("Unsubscribe from subspace");
                unsubMsg->setSubspaceId( subIt->getId().getId() );
                unsubMsg->setSrc( thisNode );
                unsubMsg->setBitLength( PUBSUB_UNSUBSCRIPTION_L( unsubMsg ));
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= unsubMsg->getByteLength()
                        );
                sendMessageToUDP( subIt->getResponsibleNode(), unsubMsg );
            }
            // Erase subspace from subscribedList and increase iterator
            subscribedSubspaces.erase( subIt++ );
        } else {
            ++subIt;
        }
    }

    // if any "near" region is not yet subscribed, subscribe
    for( set<PubSubSubspaceId>::iterator regionIt = expectedRegions.begin(); regionIt != expectedRegions.end(); ++regionIt ){
        PubSubSubspace sub( *regionIt );
        subscribedSubspaces.push_back( sub );
        PubSubResponsibleNodeCall* respCall = new PubSubResponsibleNodeCall("Request Responsible NodeHandle");
        respCall->setSubspacePos( Vector2D(regionIt->getX(), regionIt->getY()) );
        respCall->setBitLength( PUBSUB_RESPONSIBLENODECALL_L( respCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= respCall->getByteLength()
                );
        sendUdpRpcCall( lobbyServer, respCall, NULL, 5, 5 ); // FIXME: Make it a parameter...
    }

    if( subspace && !subspace->getResponsibleNode().isUnspecified() ){
        PubSubMoveMessage* moveMsg = new PubSubMoveMessage("Player move");
        moveMsg->setSubspaceId( region.getId() );
        moveMsg->setPlayer( thisNode );
        moveMsg->setPosition( posMsg->getPosition() );
        moveMsg->setTimestamp( simTime() );
        moveMsg->setBitLength( PUBSUB_MOVE_L( moveMsg ));
        RECORD_STATS(
                ++numMoveMessages;
                moveMessagesSize+= moveMsg->getByteLength()
                );
        sendMessageToUDP( subspace->getResponsibleNode(), moveMsg );
    } else {
        // trying to move to not-yet subscribed region
        // FIXME: change state to JOINING?
    }
}

void PubSubMMOG::handleMoveMessage( PubSubMoveMessage* moveMsg )
{
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = responsibleSubspaces.find( PubSubSubspaceId(moveMsg->getSubspaceId(), numSubspaces) );
    if( it == responsibleSubspaces.end() ){
         EV << "[PubSubMMOG::handleMoveMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "  received moveMessage for unknown subspace" << moveMsg->getSubspaceId() << "\n"
            << endl;
        return;
    }

    // If message arrived in the correct timeslot, store move message until deadline
    // Note: This assumes, we get no messages with future timestamps. At least in
    // the simulation, this assumption will hold.
    // The allowOldMoveMessages parameter allows overriding the timeslot barriers and forward all
    // messages.
    if( allowOldMoveMessages || moveMsg->getTimestamp() >= eventDeliveryTimer->getArrivalTime() - 1.0/(2*movementRate) ){
        it->second.waitingMoveMessages.push_back( moveMsg );
        ++numEventsCorrectTimeslot;
    } else {
         EV << "[PubSubMMOG::handleMoveMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "  received moveMesage with Timestamp: " << moveMsg->getTimestamp() << "\n"
            << "  deadline was: " << eventDeliveryTimer->getArrivalTime() - 1.0/(2*movementRate) << "\n"
            << endl;
        ++numEventsWrongTimeslot;
        cancelAndDelete( moveMsg );
    }
}

void PubSubMMOG::handleMoveListMessage( PubSubMoveListMessage* moveMsg )
{
    simtime_t timestamp = moveMsg->getTimestamp();

    // If I'm intermediate node for this subspace, forward message to children
    std::map<PubSubSubspaceId, PubSubSubspaceIntermediate>::iterator it;
    it = intermediateSubspaces.find( PubSubSubspaceId(moveMsg->getSubspaceId(), numSubspaces) );
    if( it != intermediateSubspaces.end() ){
        // Forward only if the message has not already been forwarded
        if( it->second.getLastTimestamp() < moveMsg->getTimestamp() ){
            set<NodeHandle>::iterator childIt;
            for( childIt = it->second.children.begin(); childIt != it->second.children.end(); ++childIt ){
                sendMessageToUDP( *childIt, (BaseOverlayMessage*) moveMsg->dup() );
                RECORD_STATS(
                        ++numMoveListMessages;
                        moveListMessagesSize+= moveMsg->getByteLength()
                        );
            }
            it->second.setTimestamp( timestamp );
        }
    }

    // If I'm subscribed to the subspace, transfer a GameAPIMoveList to app
    std::list<PubSubSubspace>::iterator subIt;
    for( subIt = subscribedSubspaces.begin(); subIt != subscribedSubspaces.end(); ++subIt ){
        if( subIt->getId().getId() == moveMsg->getSubspaceId() ){
            if( subIt->getLastTimestamp() < moveMsg->getTimestamp() ){
                GameAPIListMessage* moveList = new GameAPIListMessage("player position update");
                moveList->setCommand( NEIGHBOR_UPDATE );
                moveList->setAddNeighborArraySize( moveMsg->getPlayerArraySize() );
                moveList->setNeighborPositionArraySize( moveMsg->getPositionArraySize() );
                for( unsigned int i = 0; i < moveMsg->getPlayerArraySize(); ++i ){
                    moveList->setAddNeighbor( i, moveMsg->getPlayer(i) );
                    moveList->setNeighborPosition( i, moveMsg->getPosition(i) );
                    RECORD_STATS(
                        globalStatistics->addStdDev("PubSubMMOG: MoveDelay",
                                SIMTIME_DBL(simTime() - timestamp + moveMsg->getPositionAge(i)) );
                        );
                }
                send( moveList, "appOut" );
                RECORD_STATS(
                        if( timestamp < simTime() - maxMoveDelay ){
                            ++lostMovementLists;
                        } else {
                            ++receivedMovementLists;
                        }
                        if( subIt->getLastTimestamp() != 0) lostMovementLists += (int)(SIMTIME_DBL(timestamp - subIt->getLastTimestamp())*movementRate -1);

                        );
                subIt->setTimestamp( timestamp );
            }
            return;
        }
    }
}

void PubSubMMOG::handleHelpResponse( PubSubHelpResponse* helpResp )
{
   // lobby server answered our call for help
   // (i.e. he sends us a candidate for backup/intermediate nodes
    if( helpResp->getHelpType() == PUBSUB_BACKUP ){
        PubSubBackupCall* backupCall = new PubSubBackupCall("Become my backup node!");
        backupCall->setSubspaceId( helpResp->getSubspaceId() );

        // Find the subspace in the subspace map
        std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
        it = responsibleSubspaces.find( PubSubSubspaceId(helpResp->getSubspaceId(), numSubspaces) );
        if( it == responsibleSubspaces.end() ){
            EV << "[PubSubMMOG::handleHelpResponse() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "  received helpResponse for unknown subspace" << helpResp->getSubspaceId() << "\n"
               << endl;
            return;
        }
        PubSubSubspaceResponsible& subspace = it->second;

        // Assume the new backup will not refuse his task
        subspace.setBackupNode( helpResp->getNode() );

// FIXME: just for testing
int iii = subspace.getTotalChildrenCount();
subspace.fixTotalChildrenCount();
if( iii != subspace.getTotalChildrenCount() ){
    opp_error("Huh?");
}

        // backup the load balancing tree
        backupCall->setChildrenArraySize( subspace.getTotalChildrenCount() );
        backupCall->setChildrenPosArraySize( subspace.getTotalChildrenCount() );
        backupCall->setIntermediatesArraySize( subspace.intermediateNodes.size() );

        set<NodeHandle>::iterator childIt;
        map<NodeHandle, bool>::iterator childMapIt;
        unsigned int i = 0;
        for( childMapIt = subspace.cachedChildren.begin(); childMapIt != subspace.cachedChildren.end(); ++childMapIt ){
            backupCall->setChildren(i, childMapIt->first);
            backupCall->setChildrenPos(i, -2);
            ++i;
        }
        for( childIt = subspace.children.begin(); childIt != subspace.children.end(); ++childIt ){
            backupCall->setChildren(i, *childIt);
            backupCall->setChildrenPos(i, -1);
            ++i;
        }
        for( unsigned int ii = 0; ii < subspace.intermediateNodes.size(); ++ii ){
            PubSubSubspaceResponsible::IntermediateNode& iNode =  subspace.intermediateNodes[ii];
            backupCall->setIntermediates(ii, iNode.node);
            for( childIt = iNode.children.begin(); childIt != iNode.children.end(); ++childIt ){
                backupCall->setChildren(i, *childIt);
                backupCall->setChildrenPos(i, ii);
                ++i;
            }
        }

        backupCall->setBitLength( PUBSUB_BACKUPCALL_L( backupCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= backupCall->getByteLength()
                );
        sendUdpRpcCall( helpResp->getNode(), backupCall );

    } else if( helpResp->getHelpType() == PUBSUB_INTERMEDIATE ){
        PubSubIntermediateCall* intermediateCall = new PubSubIntermediateCall("Become my intermediate node!");
        intermediateCall->setSubspaceId( helpResp->getSubspaceId() );
        intermediateCall->setBitLength( PUBSUB_INTERMEDIATECALL_L( intermediateCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= intermediateCall->getByteLength()
                );
        sendUdpRpcCall( helpResp->getNode(), intermediateCall );
    }
}

void PubSubMMOG::handleBackupCall( PubSubBackupCall* backupCall )
{
    int intId = backupCall->getSubspaceId();
    PubSubSubspaceId subspaceId(intId, numSubspaces);

    // Start Heartbeat Timer
    PubSubTimer* parentTimeout = new PubSubTimer("ParentTimeout");
    parentTimeout->setType( PUBSUB_PARENT_TIMEOUT );
    parentTimeout->setSubspaceId( intId );
    startTimer( parentTimeout );

    // insert subspace into responsible list
    PubSubSubspaceResponsible subspace( subspaceId );
    subspace.setResponsibleNode( backupCall->getSrcNode() );
    subspace.setHeartbeatTimer( parentTimeout );

    // recounstruct load balancing tree
    for( unsigned int i = 0; i < backupCall->getIntermediatesArraySize(); ++i ){
        PubSubSubspaceResponsible::IntermediateNode iNode;
        iNode.node = backupCall->getIntermediates(i);
        subspace.intermediateNodes.push_back( iNode );
    }
    for( unsigned int i = 0; i < backupCall->getChildrenArraySize(); ++i ){
        int pos = backupCall->getChildrenPos( i );
        if( pos == -2 ){
            subspace.cachedChildren.insert( make_pair( backupCall->getChildren(i), false ));
        } else if( pos == -1 ){
            subspace.children.insert( backupCall->getChildren(i) );
        } else {
            subspace.intermediateNodes[pos].children.insert( backupCall->getChildren(i) );
        }
    }

    backupSubspaces.insert( make_pair(subspaceId, subspace) );

    PubSubBackupResponse* backupResp = new PubSubBackupResponse("I'll be your backup");
    backupResp->setSubspaceId( intId );
    backupResp->setBitLength( PUBSUB_BACKUPRESPONSE_L( backupResp ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= backupResp->getByteLength()
            );
    sendRpcResponse( backupCall, backupResp );
}

void PubSubMMOG::handleBackupResponse( PubSubBackupResponse* backupResp )
{
    // Nothing to be done
    // HandleHelpResponse() already did everything important
}

void PubSubMMOG::handleIntermediateCall( PubSubIntermediateCall* intermediateCall )
{
    // insert subspace into intermediate list
    PubSubSubspaceId subspaceId(intermediateCall->getSubspaceId(), numSubspaces);
    PubSubSubspaceIntermediate subspace( subspaceId );
    subspace.setResponsibleNode( intermediateCall->getSrcNode() );
    subspace.setTimestamp(0);
    intermediateSubspaces.insert( make_pair(subspaceId, subspace) );

    PubSubIntermediateResponse* iResp = new PubSubIntermediateResponse("I'll be your intermediate node");
    iResp->setSubspaceId( intermediateCall->getSubspaceId() );
    iResp->setBitLength( PUBSUB_INTERMEDIATERESPONSE_L( iResp ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= iResp->getByteLength()
            );
    sendRpcResponse( intermediateCall, iResp );
}

void PubSubMMOG::handleIntermediateResponse( PubSubIntermediateResponse* intermediateResp )
{
    // we found a new intermediate node for a subspace
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = responsibleSubspaces.find( PubSubSubspaceId(intermediateResp->getSubspaceId(), numSubspaces) );
    if( it == responsibleSubspaces.end() ) {
        EV << "[PubSubMMOG::handleIntermediateResponse() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "  Received Intermediate Response for unknown Subspace!\n"
           << endl;
        return;
    }
    PubSubSubspaceResponsible& subspace = it->second;
    PubSubSubspaceResponsible::IntermediateNode iNode;
    iNode.node = intermediateResp->getSrcNode();

    // if there is any broken intermediate node in list, replace it
    bool newIntermediate = true;
    deque<PubSubSubspaceResponsible::IntermediateNode>::iterator iit;
    for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit ){
        if( iit->node.isUnspecified() ){
            iit->node = iNode.node;
            newIntermediate = false;
            break;
        }
    }
    if( iit == subspace.intermediateNodes.end() ){
        subspace.intermediateNodes.push_back( iNode );
        iit = subspace.intermediateNodes.end() - 1;
    }

    // inform Backup
    if( !subspace.getBackupNode().isUnspecified() ){
        PubSubBackupIntermediateMessage* backupMsg = new PubSubBackupIntermediateMessage("Backup: new Intermediate");
        backupMsg->setSubspaceId( intermediateResp->getSubspaceId() );
        backupMsg->setNode( iNode.node );
        backupMsg->setPos( iit - subspace.intermediateNodes.begin() );
        backupMsg->setBitLength( PUBSUB_BACKUPINTERMEDIATE_L( backupMsg ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                );
        sendMessageToUDP( subspace.getBackupNode(), backupMsg );
    }

    // if needed, send adopt to parent
    int intermediatePos = iit - subspace.intermediateNodes.begin();
    int parentPos = intermediatePos/maxChildren -1;
    if( parentPos >= 0 && !subspace.intermediateNodes[parentPos].node.isUnspecified() ){
        PubSubAdoptChildCall* adoptCall = new PubSubAdoptChildCall("Adopt (intermediate) Node");
        adoptCall->setSubspaceId( intermediateResp->getSubspaceId() );
        adoptCall->setChild( iit->node );
        adoptCall->setBitLength( PUBSUB_ADOPTCHILDCALL_L( adoptCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= adoptCall->getByteLength()
                );
        sendUdpRpcCall( subspace.intermediateNodes[parentPos].node, adoptCall );
    }

    if( newIntermediate ){
        // move one child from iNodes's parent to cache
        if( parentPos >= 0 ) {
            // parent is an intermediate node
            PubSubSubspaceResponsible::IntermediateNode& parent = subspace.intermediateNodes[parentPos];
            if( parent.children.begin() != parent.children.end() ){
                bool fixNeeded = false;
                if( !subspace.cachedChildren.insert( make_pair( *(parent.children.begin()), false )).second ){
                    fixNeeded = true;
                }
                if( !subspace.getBackupNode().isUnspecified() ){
                    PubSubBackupSubscriptionMessage* backupMsg = new PubSubBackupSubscriptionMessage("Backup: nodes moved to cache");
                    backupMsg->setSubspaceId( intermediateResp->getSubspaceId() );
                    backupMsg->setChild( *(parent.children.begin()) );
                    backupMsg->setOldParent( parent.node );
                    backupMsg->setBitLength( PUBSUB_BACKUPSUBSCRIPTION_L( backupMsg ));
                    RECORD_STATS(
                            ++numPubSubSignalingMessages;
                            pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                            );
                    sendMessageToUDP( subspace.getBackupNode(), backupMsg );
                }
                PubSubNodeLeftMessage* goneMsg = new PubSubNodeLeftMessage("Node left: moved");
                goneMsg->setNode( *(parent.children.begin()) );
                goneMsg->setSubspaceId( intermediateResp->getSubspaceId() );
                goneMsg->setBitLength( PUBSUB_NODELEFT_L( goneMsg ));
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= goneMsg->getByteLength()
                        );
                sendMessageToUDP( parent.node, goneMsg );
                parent.children.erase( parent.children.begin() );
                if( fixNeeded ){
                    subspace.fixTotalChildrenCount();
                }
            }

        } else {
            // we are parent
            if( subspace.children.begin() != subspace.children.end() ){
                bool fixNeeded = false;
                if( !subspace.cachedChildren.insert( make_pair( *(subspace.children.begin()), false )).second ){
                    fixNeeded = true;
                }
                if( !subspace.getBackupNode().isUnspecified() ){
                    PubSubBackupSubscriptionMessage* backupMsg = new PubSubBackupSubscriptionMessage("Backup: nodes moved to cache");
                    backupMsg->setSubspaceId( intermediateResp->getSubspaceId() );
                    backupMsg->setChild( *(subspace.children.begin()) );
                    backupMsg->setOldParent( thisNode );
                    backupMsg->setBitLength( PUBSUB_BACKUPSUBSCRIPTION_L( backupMsg ));
                    RECORD_STATS(
                            ++numPubSubSignalingMessages;
                            pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                            );
                    sendMessageToUDP( subspace.getBackupNode(), backupMsg );
                }
                subspace.children.erase( *(subspace.children.begin()) );
                if( fixNeeded ){
                    subspace.fixTotalChildrenCount();
                }
            }
        }
    } else {
        // send adopt for all children intermediates
        for( int pos = (intermediatePos+1) * maxChildren; pos < (int) subspace.intermediateNodes.size() &&
                pos < (intermediatePos+2) * maxChildren; ++pos ){
            if( subspace.intermediateNodes[pos].node.isUnspecified() ) continue;
            PubSubAdoptChildCall* adoptCall = new PubSubAdoptChildCall("Adopt (intermediate) Node");
            adoptCall->setSubspaceId( intermediateResp->getSubspaceId() );
            adoptCall->setChild( subspace.intermediateNodes[pos].node );
            adoptCall->setBitLength( PUBSUB_ADOPTCHILDCALL_L( adoptCall ));
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize+= adoptCall->getByteLength()
                    );
            sendUdpRpcCall( iit->node, adoptCall );
        }
    }

    // move as many cached children to the new node as possible
    std::map<NodeHandle,bool>::iterator childIt;
    for( childIt = subspace.cachedChildren.begin(); childIt != subspace.cachedChildren.end(); ++childIt ){
        if( childIt->second ) continue;
        PubSubAdoptChildCall* adoptCall = new PubSubAdoptChildCall("Adopt Node");
        adoptCall->setSubspaceId( intermediateResp->getSubspaceId() );
        adoptCall->setChild( childIt->first );
        adoptCall->setBitLength( PUBSUB_ADOPTCHILDCALL_L( adoptCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= adoptCall->getByteLength()
                );
        sendUdpRpcCall( intermediateResp->getSrcNode(), adoptCall );
        childIt->second = true;
        if( (unsigned int) maxChildren == ++(iit->waitingChildren) ) break;
    }
}

void PubSubMMOG::handleAdoptChildCall( PubSubAdoptChildCall* adoptCall )
{
    std::map<PubSubSubspaceId, PubSubSubspaceIntermediate>::iterator it;
    it = intermediateSubspaces.find( PubSubSubspaceId(adoptCall->getSubspaceId(), numSubspaces) );
    if( it == intermediateSubspaces.end() ) {
        EV << "[PubSubMMOG::handleAdoptChildCall() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "  Received Adopt Child Call for unknown Subspace!\n"
           << endl;
        cancelAndDelete( adoptCall );
        return;
    }

    it->second.addChild( adoptCall->getChild() );
    PubSubAdoptChildResponse* adoptResp = new PubSubAdoptChildResponse("I adopted child");
    adoptResp->setSubspaceId( adoptCall->getSubspaceId() );
    adoptResp->setChild( adoptCall->getChild() );
    adoptResp->setBitLength( PUBSUB_ADOPTCHILDRESPONSE_L( adoptResp ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= adoptResp->getByteLength()
            );
    sendRpcResponse( adoptCall, adoptResp );
}

void PubSubMMOG::handleAdoptChildResponse( PubSubAdoptChildResponse* adoptResp )
{
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = responsibleSubspaces.find( PubSubSubspaceId(adoptResp->getSubspaceId(), numSubspaces) );
    if( it == responsibleSubspaces.end() ) {
        EV << "[PubSubMMOG::handleAdoptChildResponse() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "  Received AdoptChild Response for unknown Subspace!\n"
           << endl;
        return;
    }

// FIXME: just for testing
PubSubSubspaceResponsible& subspace = it->second;
int iii = subspace.getTotalChildrenCount();
subspace.fixTotalChildrenCount();
if( iii != subspace.getTotalChildrenCount() ){
    opp_error("Huh?");
}

    // Find intermediate node in subspace
    deque<PubSubSubspaceResponsible::IntermediateNode>::iterator iit;
    for( iit = it->second.intermediateNodes.begin(); iit !=  it->second.intermediateNodes.end(); ++iit ){
        if( !iit->node.isUnspecified() && iit->node == adoptResp->getSrcNode() ){

            // if adoption was for a child intermediate node, nothing is to be done
            int intermediatePos = iit - it->second.intermediateNodes.begin();
            for( int pos = (intermediatePos+1) * maxChildren; pos < (int) it->second.intermediateNodes.size() &&
                    pos < (intermediatePos+2) * maxChildren; ++pos )
            {
                if( !it->second.intermediateNodes[pos].node.isUnspecified() &&
                        adoptResp->getChild() == it->second.intermediateNodes[pos].node ){
                    return;
                }
            }

            // child is a "real" child->remove it from cache
            if( !it->second.cachedChildren.erase( adoptResp->getChild() ) ){
                // if node got deleted in the meantime, inform parent...
                PubSubNodeLeftMessage* goneMsg = new PubSubNodeLeftMessage("Node left Subspace");
                goneMsg->setNode( adoptResp->getChild() );
                goneMsg->setSubspaceId( it->second.getId().getId() );
                goneMsg->setBitLength( PUBSUB_NODELEFT_L( goneMsg ));
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= goneMsg->getByteLength()
                        );
                sendMessageToUDP( adoptResp->getSrcNode(), goneMsg );
                return;
            }

            // move child to intermediate node's childrenlist
            if( !iit->children.insert( adoptResp->getChild() ).second ){
                // Node was already in children list, fix children count
                subspace.fixTotalChildrenCount();
            }
            iit->waitingChildren--;

// FIXME: just for testing
PubSubSubspaceResponsible& subspace = it->second;
int iii = subspace.getTotalChildrenCount();
subspace.fixTotalChildrenCount();
if( iii != subspace.getTotalChildrenCount() ){
    opp_error("Huh?");
}

            // Inform Backup
            if( !it->second.getBackupNode().isUnspecified() ){
                PubSubBackupSubscriptionMessage* backupMsg = new PubSubBackupSubscriptionMessage("Backup: node got a new parent");
                backupMsg->setSubspaceId( adoptResp->getSubspaceId() );
                backupMsg->setChild( adoptResp->getChild() );
                backupMsg->setParent( adoptResp->getSrcNode() );
                backupMsg->setBitLength( PUBSUB_BACKUPSUBSCRIPTION_L( backupMsg ));
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                       pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                       );
                sendMessageToUDP( it->second.getBackupNode(), backupMsg );
                return;
            }
        }
    }

    EV << "[PubSubMMOG::handleAdoptChildResponse() @ " << thisNode.getIp()
       << " (" << thisNode.getKey().toString(16) << ")]\n"
       << "  Received AdoptChild Response for unknown child!\n"
       << endl;
}

void PubSubMMOG::handlePingCall( PubSubPingCall* pingCall )
{
    int subspaceId = pingCall->getSubspaceId();

    if( pingCall->getPingType() == PUBSUB_PING_BACKUP ){
        // reset heartbeat timer
        std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
        it = backupSubspaces.find( PubSubSubspaceId(pingCall->getSubspaceId(), numSubspaces) );
        if( it == backupSubspaces.end() ) {
            EV << "[PubSubMMOG::handlePingCall() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "  Received PingCall for unknown Subspace!\n"
               << endl;
            // FIXME: Somebody thinks we are his backup. What shall we do?
        } else {
            it->second.resetHeartbeatFailCount();
            startTimer( it->second.getHeartbeatTimer() );
        }
    }

    PubSubPingResponse* pingResp = new PubSubPingResponse("PingResponse");
    pingResp->setSubspaceId( subspaceId );
    pingResp->setBitLength( PUBSUB_PINGRESPONSE_L( pingResp ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= pingResp->getByteLength()
            );
    sendRpcResponse( pingCall, pingResp );
}

void PubSubMMOG::handlePingResponse( PubSubPingResponse* pingResp )
{
}

void PubSubMMOG::takeOverNewSubspace( PubSubSubspaceId subspaceId )
{
    // create a new subspace
    PubSubSubspaceResponsible subspace( subspaceId );
    takeOverSubspace( subspace, true );
}

void PubSubMMOG::takeOverSubspace( PubSubSubspaceResponsible& subspace, bool isNew = false )
{
    const PubSubSubspaceId& subspaceId = subspace.getId();
    int intId = subspaceId.getId();

    subspace.fixTotalChildrenCount();

    NodeHandle oldNode = subspace.getResponsibleNode();

    // insert subspace into responsible list
    subspace.setResponsibleNode( thisNode );
    responsibleSubspaces.insert( make_pair(subspaceId, subspace) );

    // request backup
    PubSubHelpCall* helpCall = new PubSubHelpCall("I need a backup node");
    helpCall->setHelpType( PUBSUB_BACKUP );
    helpCall->setSubspaceId( intId );
    helpCall->setBitLength( PUBSUB_HELPCALL_L( helpCall ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= helpCall->getByteLength()
            );
    sendUdpRpcCall( lobbyServer, helpCall );

    if( !isNew ) {
        PubSubReplacementMessage* repMsg = new PubSubReplacementMessage("I replaced the responsible node");
        repMsg->setSubspaceId( intId );
        repMsg->setNewResponsibleNode( thisNode );
        repMsg->setBitLength( PUBSUB_REPLACEMENT_L( repMsg ));

        // Inform children and lobbyserver about takeover
        sendMessageToChildren( subspace, repMsg, NULL, repMsg );
        sendMessageToUDP( lobbyServer, repMsg );

        // inform lobby server over failed node
        PubSubFailedNodeMessage* failedNode = new PubSubFailedNodeMessage("Node failed");
        failedNode->setFailedNode( oldNode );
        failedNode->setBitLength( PUBSUB_FAILEDNODE_L( failedNode ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= failedNode->getByteLength()
                );
        sendMessageToUDP( lobbyServer, failedNode );
   }
}

void PubSubMMOG::sendHearbeatToChildren()
{
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    for( it = responsibleSubspaces.begin(); it != responsibleSubspaces.end(); ++it) {
        PubSubPingCall* bHeartbeat = new PubSubPingCall("Heartbeat Backup");
        bHeartbeat->setPingType( PUBSUB_PING_BACKUP );
        bHeartbeat->setSubspaceId( it->second.getId().getId() );
        bHeartbeat->setBitLength( PUBSUB_PINGCALL_L( bHeartbeat ));

        PubSubPingCall* iHeartbeat = new PubSubPingCall("Heartbeat Intermediate");
        iHeartbeat->setPingType( PUBSUB_PING_INTERMEDIATE );
        iHeartbeat->setSubspaceId( it->second.getId().getId() );
        iHeartbeat->setBitLength( PUBSUB_PINGCALL_L( iHeartbeat ));

        sendMessageToChildren( it->second, iHeartbeat, bHeartbeat, NULL);
        delete bHeartbeat;
        delete iHeartbeat;
    }
}

void PubSubMMOG::sendPingToChildren()
{
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    for( it = responsibleSubspaces.begin(); it != responsibleSubspaces.end(); ++it) {
        PubSubPingCall* heartbeat = new PubSubPingCall("Ping");
        heartbeat->setPingType( PUBSUB_PING_CHILD );
        heartbeat->setSubspaceId( it->second.getId().getId() );
        heartbeat->setBitLength( PUBSUB_PINGCALL_L( heartbeat ));
        sendMessageToChildren( it->second, NULL, NULL, heartbeat );
        delete heartbeat;
    }
}

void PubSubMMOG::handleParentTimeout( PubSubTimer* timer )
{
    // our parent timed out. we have to take over the subspace...
    PubSubSubspaceId subspaceId(timer->getSubspaceId(), numSubspaces);
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = backupSubspaces.find( subspaceId );
    if( it == backupSubspaces.end() ) {
        delete timer;
        return;
    }

    // increase fail count; if to high, take over subspace
    it->second.incHeartbeatFailCount();
    if( it->second.getHeartbeatFailCount() > 1 ) { // FIXME: make it a parameter

        // Delete Timer
        cancelAndDelete( timer );
        it->second.setHeartbeatTimer( NULL );

        // Take over Subspace
        takeOverSubspace( it->second );
        backupSubspaces.erase( it );

    } else {
        startTimer( timer );
    }
}

void PubSubMMOG::handleBackupCallTimeout( PubSubBackupCall* backupCall, const TransportAddress& oldNode )
{
    // FIXME: cast oldNode to NodeHandle
    // Inform Lobbyserver over failed node
    PubSubFailedNodeMessage* failedNode = new PubSubFailedNodeMessage("Node failed");
    failedNode->setFailedNode( oldNode );
    failedNode->setBitLength( PUBSUB_FAILEDNODE_L( failedNode ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= failedNode->getByteLength()
            );
    sendMessageToUDP( lobbyServer, failedNode );

    // Request new Backup
    PubSubHelpCall* helpCall = new PubSubHelpCall("I need a backup node");
    helpCall->setHelpType( PUBSUB_BACKUP );
    helpCall->setSubspaceId( backupCall->getSubspaceId() );
    helpCall->setBitLength( PUBSUB_HELPCALL_L( helpCall ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= helpCall->getByteLength()
            );
    sendUdpRpcCall( lobbyServer, helpCall );

    // find appropriate subspace and mark backup as failed
    PubSubSubspaceId subspaceId(backupCall->getSubspaceId(), numSubspaces);
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = responsibleSubspaces.find( subspaceId );
    if( it == responsibleSubspaces.end() ) {
        return;
    }
    it->second.setBackupNode( NodeHandle::UNSPECIFIED_NODE );
}

void PubSubMMOG::handlePingCallTimeout( PubSubPingCall* pingCall, const TransportAddress& oldNode )
{
    // Inform Lobbyserver over failed node
    const NodeHandle& oldNodeHandle = dynamic_cast<const NodeHandle&>(oldNode);
    // FIXME: use oldNodeHandle instead of oldNode
    PubSubFailedNodeMessage* failedNode = new PubSubFailedNodeMessage("Node failed");
    failedNode->setBitLength( PUBSUB_FAILEDNODE_L( failedNode ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= failedNode->getByteLength()
            );
    failedNode->setFailedNode( oldNode );
    sendMessageToUDP( lobbyServer, failedNode );

    // find appropriate subspace
    PubSubSubspaceId subspaceId(pingCall->getSubspaceId(), numSubspaces);
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = responsibleSubspaces.find( subspaceId );
    if( it == responsibleSubspaces.end() ) {
        return;
    }
    PubSubSubspaceResponsible& subspace = it->second;

    if( pingCall->getPingType() == PUBSUB_PING_CHILD ){
        // remove child
        unsubscribeChild( oldNodeHandle, subspace );

    } else if( pingCall->getPingType() == PUBSUB_PING_BACKUP ){

        // if we didn't already have (or asked for) a new backup
        if( !subspace.getBackupNode().isUnspecified() &&
                subspace.getBackupNode() == oldNodeHandle )
        {
            // our backup timed out. we have to request a new one...
            // delete old backup entry
            subspace.setBackupNode( NodeHandle::UNSPECIFIED_NODE );

            // Request new Backup
            PubSubHelpCall* helpCall = new PubSubHelpCall("I need a backup node");
            helpCall->setHelpType( PUBSUB_BACKUP );
            helpCall->setSubspaceId( pingCall->getSubspaceId() );
            helpCall->setBitLength( PUBSUB_HELPCALL_L( helpCall ));
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize+= helpCall->getByteLength()
                    );
            sendUdpRpcCall( lobbyServer, helpCall );
        }

    } else if( pingCall->getPingType() == PUBSUB_PING_INTERMEDIATE ){
        // one intermediate node timed out. we have to request a new one...
        // delete old intermediate entry
        deque<PubSubSubspaceResponsible::IntermediateNode>::iterator iit;
        for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit ){
            if( !iit->node.isUnspecified() && oldNode == iit->node ) break;
        }
        if( iit == subspace.intermediateNodes.end() ) return;

        NodeHandle oldNode = iit->node;
        iit->node = NodeHandle::UNSPECIFIED_NODE;

        // inform Backup
        if( !subspace.getBackupNode().isUnspecified() ){
            PubSubBackupIntermediateMessage* backupMsg = new PubSubBackupIntermediateMessage("Backup: Intermediate failed");
            backupMsg->setSubspaceId( pingCall->getSubspaceId() );
            backupMsg->setPos( iit - it->second.intermediateNodes.begin() );
            backupMsg->setBitLength( PUBSUB_BACKUPINTERMEDIATE_L( backupMsg ));
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                    );
            sendMessageToUDP( subspace.getBackupNode(), backupMsg );
        }

        bool fixNeeded = false;
        // Take over all children until new intermediate is found.
        set<NodeHandle>::iterator childIt;
        for(  childIt = iit->children.begin(); childIt != iit->children.end(); ++childIt ){
            if( !subspace.cachedChildren.insert( make_pair(*childIt, false)).second ){
                fixNeeded = true;
            }

            // Inform Backup
            if( !subspace.getBackupNode().isUnspecified() ){
                PubSubBackupSubscriptionMessage* backupMsg = new PubSubBackupSubscriptionMessage("Backup: nodes moved to cache");
                backupMsg->setSubspaceId( pingCall->getSubspaceId() );
                backupMsg->setChild( *childIt );
                backupMsg->setOldParent( oldNodeHandle );
                backupMsg->setBitLength( PUBSUB_BACKUPSUBSCRIPTION_L( backupMsg ));
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                        );
                sendMessageToUDP( subspace.getBackupNode(), backupMsg );
            }
        }
        iit->children.clear();
        if( fixNeeded ) subspace.fixTotalChildrenCount();

        // inform parent of intermediate node
        int parentPos = (iit - subspace.intermediateNodes.begin())/maxChildren -1;
        if( parentPos >= 0 ){
            PubSubSubspaceResponsible::IntermediateNode& parent = subspace.intermediateNodes[parentPos];
            if( !parent.node.isUnspecified() ){
                PubSubNodeLeftMessage* goneMsg = new PubSubNodeLeftMessage("Intermediate left Subspace");
                goneMsg->setNode( oldNodeHandle );
                goneMsg->setSubspaceId( subspace.getId().getId() );
                goneMsg->setBitLength( PUBSUB_NODELEFT_L( goneMsg ));
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= goneMsg->getByteLength()
                        );
                sendMessageToUDP( parent.node, goneMsg );
            }
        }

        // Request new Intermediate
        PubSubHelpCall* helpCall = new PubSubHelpCall("I need an intermediate node");
        helpCall->setHelpType( PUBSUB_INTERMEDIATE );
        helpCall->setSubspaceId( pingCall->getSubspaceId() );
        helpCall->setBitLength( PUBSUB_HELPCALL_L( helpCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= helpCall->getByteLength()
                );
        sendUdpRpcCall( lobbyServer, helpCall );
    }
// FIXME: just for testing
int iii = subspace.getTotalChildrenCount();
subspace.fixTotalChildrenCount();
if( iii != subspace.getTotalChildrenCount() ){
    opp_error("Huh?");
}

}

void PubSubMMOG::handleSubscriptionCallTimeout( PubSubSubscriptionCall* subscriptionCall, const TransportAddress& oldNode )
{
    // FIXME: cast oldNode to NodeHandle
    // our subscription call timed out. This means the responsible node is dead...
    // Inform Lobbyserver over failed node
    PubSubFailedNodeMessage* failedNode = new PubSubFailedNodeMessage("Node failed");
    failedNode->setFailedNode( oldNode );
    failedNode->setBitLength( PUBSUB_FAILEDNODE_L( failedNode ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= failedNode->getByteLength()
            );
    sendMessageToUDP( lobbyServer, failedNode );

    // Ask for new responsible node
    PubSubResponsibleNodeCall* respCall = new PubSubResponsibleNodeCall("Request Responsible NodeHandle");
    PubSubSubspaceId subspaceId( subscriptionCall->getSubspaceId(), numSubspaces);
    respCall->setSubspacePos( Vector2D(subspaceId.getX(), subspaceId.getY()) );
    respCall->setBitLength( PUBSUB_RESPONSIBLENODECALL_L( respCall ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize+= respCall->getByteLength()
            );
    sendUdpRpcCall( lobbyServer, respCall, NULL, 5, 5 ); // FIXME: Make it a parameter...
}

void PubSubMMOG::handleReplacementMessage( PubSubReplacementMessage* replaceMsg )
{
    PubSubSubspaceId subspaceId(replaceMsg->getSubspaceId(), numSubspaces);

    // There's a new responsible node for a subspace
    // Replace the old one in the intermediateSubspaces map...
    std::map<PubSubSubspaceId, PubSubSubspaceIntermediate>::iterator it;
    it = intermediateSubspaces.find( subspaceId );
    if( it != intermediateSubspaces.end() ) {
        it->second.setResponsibleNode( replaceMsg->getNewResponsibleNode() );
    }

    // ... and in the subsribed subspaces list
    std::list<PubSubSubspace>::iterator iit;
    for( iit = subscribedSubspaces.begin(); iit != subscribedSubspaces.end(); ++iit ){
        if( iit->getId() == subspaceId ) {
            iit->setResponsibleNode( replaceMsg->getNewResponsibleNode() );
            return;
        }
    }
}

void PubSubMMOG::handleReleaseIntermediate( PubSubReleaseIntermediateMessage* releaseMsg )
{
    PubSubSubspaceId subspaceId(releaseMsg->getSubspaceId(), numSubspaces);
    intermediateSubspaces.erase( subspaceId );
}

void PubSubMMOG::handleIntermediateBackup( PubSubBackupIntermediateMessage* backupMsg )
{
    // find appropriate subspace
    PubSubSubspaceId subspaceId(backupMsg->getSubspaceId(), numSubspaces);
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = backupSubspaces.find( subspaceId );
    if( it == backupSubspaces.end() ) {
        return;
    }

    if( backupMsg->getPos() >= (int) it->second.intermediateNodes.size() ){
        it->second.intermediateNodes.resize( backupMsg->getPos() + 1 );
    }
    it->second.intermediateNodes[ backupMsg->getPos() ].node = backupMsg->getNode();
}

void PubSubMMOG::handleSubscriptionBackup( PubSubBackupSubscriptionMessage* backupMsg )
{
    // Note: this funktion may break subspace's tatalChildrenCall
    // You have to use fixTotalChildrenCount before using the subspace
    // find appropriate subspace
    PubSubSubspaceId subspaceId(backupMsg->getSubspaceId(), numSubspaces);
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = backupSubspaces.find( subspaceId );
    if( it == backupSubspaces.end() ) {
        return;
    }
    PubSubSubspaceResponsible& subspace = it->second;

    deque<PubSubSubspaceResponsible::IntermediateNode>::iterator iit;

    if( !backupMsg->getOldParent().isUnspecified() ){
        // oldParent set -> move child
        if( backupMsg->getOldParent() == subspace.getResponsibleNode() ){
            // direct child -> cache
            subspace.removeChild( backupMsg->getChild() );
            subspace.cachedChildren.insert(make_pair( backupMsg->getChild(), false) );

        } else {
            // from I -> chache
            for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit ){
//                if( !iit->node.isUnspecified() && iit->node == backupMsg->getOldParent() ){
                    iit->children.erase( backupMsg->getChild() );
//                }
            }
            subspace.cachedChildren.insert(make_pair( backupMsg->getChild(), false) );
        }
    } else if( backupMsg->getParent().isUnspecified() ){
        // parent not set -> new child to chache
        subspace.cachedChildren.insert(make_pair( backupMsg->getChild(), false) );

    } else if( backupMsg->getParent() == subspace.getResponsibleNode() ){
        // new direct child
        subspace.addChild( backupMsg->getChild() );
    } else {
        // move child from cache to intermediate
        subspace.cachedChildren.erase( backupMsg->getChild() );

        for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit ){
            if( !iit->node.isUnspecified() && iit->node == backupMsg->getParent() ){
                iit->children.insert( backupMsg->getChild() );
            }
        }
        // FIXME: check for errors
    }
}

void PubSubMMOG::handleUnsubscribeBackup( PubSubBackupUnsubscribeMessage* backupMsg )
{
    // Note: this funktion may break subspace's tatalChildrenCall
    // You have to use fixTotalChildrenCount before using the subspace
    // find appropriate subspace
    PubSubSubspaceId subspaceId(backupMsg->getSubspaceId(), numSubspaces);
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    it = backupSubspaces.find( subspaceId );
    if( it == backupSubspaces.end() ) {
        return;
    }
    PubSubSubspaceResponsible& subspace = it->second;

    deque<PubSubSubspaceResponsible::IntermediateNode>::iterator iit;
    set<NodeHandle>::iterator childIt;

    if( !subspace.removeChild(backupMsg->getChild()) && !subspace.cachedChildren.erase( backupMsg->getChild()) ){
        for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit ){
            iit->children.erase( backupMsg->getChild() );
        }
    }
    if( !backupMsg->getIntermediate().isUnspecified() ){
        // remove intermediate
        for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit ){
            if( !iit->node.isUnspecified() && iit->node == backupMsg->getIntermediate() ){
                for( childIt = iit->children.begin(); childIt != iit->children.end(); ++childIt ){
                    // FIXME: note really stable. let the resp node inform us about child moves
                    // remove children of last intermediate
                    if( subspace.getNumChildren() + subspace.getNumIntermediates() < maxChildren ){
                        // we have room for the child
                        subspace.children.insert( *childIt );
                    } else {
                        // Node has to go to some intermediate
                        // cache it
                        subspace.cachedChildren.insert( make_pair(*childIt, true) );
                    }
                }
                subspace.intermediateNodes.erase( iit );
                break;
            }
        }
    }
}

void PubSubMMOG::unsubscribeChild( const NodeHandle& node, PubSubSubspaceResponsible& subspace )
{
    PubSubBackupUnsubscribeMessage* backupMsg = new PubSubBackupUnsubscribeMessage("Backup: node left subspace");
    backupMsg->setChild( node );
    backupMsg->setSubspaceId( subspace.getId().getId() );
    PubSubSubspaceResponsible::IntermediateNode* iNode = subspace.removeAnyChild( node );
    if( iNode && !iNode->node.isUnspecified() ) {
        // Node is handled by an intermediate node, inform him
        PubSubNodeLeftMessage* goneMsg = new PubSubNodeLeftMessage("Node left Subspace");
        goneMsg->setNode( node );
        goneMsg->setSubspaceId( subspace.getId().getId() );
        goneMsg->setBitLength( PUBSUB_NODELEFT_L( goneMsg ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= goneMsg->getByteLength()
                );
        sendMessageToUDP( iNode->node, goneMsg );
    }
    if ( subspace.getTotalChildrenCount() < ( maxChildren - 1) * subspace.getNumIntermediates()){// FIXME: parameter when to start cleanup?
        // Too many "free" slots, remove one intermediate node
        PubSubSubspaceResponsible::IntermediateNode& liNode = subspace.intermediateNodes.back();
        if( !liNode.node.isUnspecified() ){
            // Inform node + lobby about release from intermediate node status
            PubSubReleaseIntermediateMessage* releaseMsg = new PubSubReleaseIntermediateMessage("I don't need you anymore as intermediate");
            releaseMsg->setSubspaceId( subspace.getId().getId() );
            releaseMsg->setBitLength( PUBSUB_RELEASEINTERMEDIATE_L( releaseMsg ));
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize+= releaseMsg->getByteLength()
                    );
            sendMessageToUDP( liNode.node, releaseMsg );

            PubSubHelpReleaseMessage* helpRMsg = new PubSubHelpReleaseMessage("node is not my intermediate anymore");
            helpRMsg->setSubspaceId( subspace.getId().getId() );
            helpRMsg->setNode( liNode.node );
            helpRMsg->setBitLength( PUBSUB_HELPRELEASE_L( helpRMsg ));
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize+= helpRMsg->getByteLength()
                    );
            sendMessageToUDP( lobbyServer, helpRMsg );

            // inform parent of intermediate node
            int parentPos = (subspace.intermediateNodes.size()-1)/maxChildren -1;
            if( parentPos >= 0 ){
                PubSubSubspaceResponsible::IntermediateNode& parent = subspace.intermediateNodes[parentPos];
                if( !parent.node.isUnspecified() ){
                    PubSubNodeLeftMessage* goneMsg = new PubSubNodeLeftMessage("Intermediate left Subspace");
                    goneMsg->setNode( liNode.node );
                    goneMsg->setSubspaceId( subspace.getId().getId() );
                    goneMsg->setBitLength( PUBSUB_NODELEFT_L( goneMsg ));
                    RECORD_STATS(
                            ++numPubSubSignalingMessages;
                            pubSubSignalingMessagesSize+= goneMsg->getByteLength()
                            );
                    sendMessageToUDP( parent.node, goneMsg );
                }
            }
        }

        bool fixNeeded = false;
        set<NodeHandle>::iterator childIt;
        for( childIt = liNode.children.begin(); childIt != liNode.children.end(); ++childIt ){
            // remove children of last intermediate
            if( subspace.getNumChildren() + subspace.getNumIntermediates() < maxChildren ){
                // we have room for the child
                if( !subspace.children.insert( *childIt ).second ) fixNeeded = true;

                //FIXME: send backup new->toMe
            } else {
                // Node has to go to some intermediate
                // find intermediate with free capacities
                PubSubSubspaceResponsible::IntermediateNode* newINode;
                newINode = subspace.getNextFreeIntermediate();

                if( newINode  && newINode->node != liNode.node ){
                    // cache it
                    if( !subspace.cachedChildren.insert( make_pair(*childIt, true) ).second ) fixNeeded = true;
                    //FIXME: send backup new->toCache

                    ++(newINode->waitingChildren);

                    // let him adopt the child
                    PubSubAdoptChildCall* adoptCall = new PubSubAdoptChildCall("Adopt Node");
                    adoptCall->setSubspaceId( subspace.getId().getId() );
                    adoptCall->setChild( *childIt );
                    adoptCall->setBitLength( PUBSUB_ADOPTCHILDCALL_L( adoptCall ));
                    RECORD_STATS(
                            ++numPubSubSignalingMessages;
                            pubSubSignalingMessagesSize+= adoptCall->getByteLength()
                            );
                    sendUdpRpcCall( newINode->node, adoptCall );
                } else {
                    // no intermediate found
                    // just move child to cache and wait for a new one
                    if( !subspace.cachedChildren.insert( make_pair(*childIt, false) ).second ) fixNeeded = true;
                }
            }
        }
        // delete node from subspace's intermediate node list
        subspace.intermediateNodes.pop_back();
        // inform backup about deleted intermediate
        backupMsg->setIntermediate( liNode.node );

        if( fixNeeded ) subspace.fixTotalChildrenCount();
    }

// FIXME: just for testing
int iii = subspace.getTotalChildrenCount();
subspace.fixTotalChildrenCount();
if( iii != subspace.getTotalChildrenCount() ){
    opp_error("Huh?");
}

    if( !subspace.getBackupNode().isUnspecified() ){
        backupMsg->setBitLength( PUBSUB_BACKUPUNSUBSCRIBE_L( backupMsg ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize+= backupMsg->getByteLength()
                );
        sendMessageToUDP( subspace.getBackupNode(), backupMsg );
    } else {
        delete backupMsg;
    }
}

void PubSubMMOG::sendMessageToChildren( PubSubSubspaceResponsible& subspace,
                                        BaseOverlayMessage* toIntermediates,
                                        BaseOverlayMessage* toBackup,
                                        BaseOverlayMessage* toPlayers )
{
    BaseCallMessage* intermediateCall = dynamic_cast<BaseCallMessage*>(toIntermediates);
    BaseCallMessage* backupCall = dynamic_cast<BaseCallMessage*>(toBackup);
    BaseCallMessage* playerCall = dynamic_cast<BaseCallMessage*>(toPlayers);

    std::set<NodeHandle>::iterator childIt;

    if( toPlayers ) {
        // Inform all children ...
        for( childIt = subspace.children.begin(); childIt != subspace.children.end(); ++childIt ) {
            if( playerCall ){
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= playerCall->getByteLength()
                        );
                sendUdpRpcCall( *childIt, static_cast<BaseCallMessage*>(playerCall->dup()) );
            } else {
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= toPlayers->getByteLength()
                        );
                sendMessageToUDP( *childIt, static_cast<BaseOverlayMessage*>(toPlayers->dup()) );
            }
        }
        // ... and all cached children ...
        std::map<NodeHandle, bool>::iterator cacheChildIt;
        for( cacheChildIt = subspace.cachedChildren.begin(); cacheChildIt != subspace.cachedChildren.end(); ++cacheChildIt ) {
            if( playerCall ){
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= playerCall->getByteLength()
                        );
                sendUdpRpcCall( cacheChildIt->first, static_cast<BaseCallMessage*>(playerCall->dup()) );
            } else {
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= toPlayers->getByteLength()
                        );
                sendMessageToUDP( cacheChildIt->first, static_cast<BaseOverlayMessage*>(toPlayers->dup()) );
            }
        }
    }
    deque<PubSubSubspaceResponsible::IntermediateNode>::iterator iit;
    // ... all intermediate nodes ...
    for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit ){
        if( toIntermediates && !iit->node.isUnspecified() ){
            if( intermediateCall ){
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= intermediateCall->getByteLength()
                        );
                sendUdpRpcCall( iit->node, static_cast<BaseCallMessage*>(intermediateCall->dup()) );
            } else {
                RECORD_STATS(
                        ++numPubSubSignalingMessages;
                        pubSubSignalingMessagesSize+= toIntermediates->getByteLength()
                        );
                sendMessageToUDP( iit->node, static_cast<BaseOverlayMessage*>(toIntermediates->dup()) );
            }
        }
        if( toPlayers ) {
            // .. and all intermediate node's children ...
            for( childIt = iit->children.begin(); childIt != iit->children.end(); ++childIt ){
                if( playerCall ){
                    RECORD_STATS(
                            ++numPubSubSignalingMessages;
                            pubSubSignalingMessagesSize+= playerCall->getByteLength()
                            );
                    sendUdpRpcCall( *childIt, static_cast<BaseCallMessage*>(playerCall->dup()) );
                } else {
                    RECORD_STATS(
                            ++numPubSubSignalingMessages;
                            pubSubSignalingMessagesSize+= toPlayers->getByteLength()
                            );
                    sendMessageToUDP( *childIt, static_cast<BaseOverlayMessage*>(toPlayers->dup()) );
                }
            }
        }
    }
    // ... and the backup node
    if( toBackup && !subspace.getBackupNode().isUnspecified() ) {
        if( backupCall ){
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize+= backupCall->getByteLength()
                    );
            sendUdpRpcCall( subspace.getBackupNode(), static_cast<BaseCallMessage*>(backupCall->dup()) );
        } else {
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize+= toBackup->getByteLength()
                    );
            sendMessageToUDP( subspace.getBackupNode(), static_cast<BaseOverlayMessage*>(toBackup->dup()) );
        }
    }
}


void PubSubMMOG::publishEvents()
{
    // FOr all (responsible) subspaces
    int numRespSubspaces = responsibleSubspaces.size();
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    for( it = responsibleSubspaces.begin(); it != responsibleSubspaces.end(); ++it ){
        PubSubSubspaceResponsible& subspace = it->second;

        // Prepare a movement list message aggregating all stored move messages
        PubSubMoveListMessage* moveList = new PubSubMoveListMessage("Movement list");
        moveList->setTimestamp( simTime() );
        moveList->setSubspaceId( subspace.getId().getId() );
        moveList->setPlayerArraySize( subspace.waitingMoveMessages.size() );
        moveList->setPositionArraySize( subspace.waitingMoveMessages.size() );
        moveList->setPositionAgeArraySize( subspace.waitingMoveMessages.size() );

        std::deque<PubSubMoveMessage*>::iterator msgIt;
        int pos = 0;
        for( msgIt = subspace.waitingMoveMessages.begin(); msgIt != subspace.waitingMoveMessages.end(); ++msgIt ){
            moveList->setPlayer( pos, (*msgIt)->getPlayer() );
            moveList->setPosition( pos, (*msgIt)->getPosition() );
            moveList->setPositionAge( pos, simTime() - (*msgIt)->getCreationTime() );
            pos++;
            cancelAndDelete( *msgIt );
        }
        subspace.waitingMoveMessages.clear();

        moveList->setBitLength( PUBSUB_MOVELIST_L( moveList ));
        // Send message to all direct children...
        for( set<NodeHandle>::iterator childIt = subspace.children.begin();
                childIt != subspace.children.end(); ++childIt )
        {
            RECORD_STATS(
                    ++numMoveListMessages;
                    moveListMessagesSize+= moveList->getByteLength();
                    respMoveListMessagesSize+= (int)((double) moveList->getByteLength() / numRespSubspaces)
                    );
            sendMessageToUDP( *childIt, (BaseOverlayMessage*) moveList->dup() );
        }

        //... all cached children (if messages are not too big) ...
        if( moveList->getByteLength() < 1024 ){ // FIXME: magic number. make it a parameter, or dependant on the available bandwidth
            for( map<NodeHandle, bool>::iterator childIt = subspace.cachedChildren.begin();
                    childIt != subspace.cachedChildren.end(); ++childIt )
            {
                RECORD_STATS(
                        ++numMoveListMessages;
                        moveListMessagesSize+= moveList->getByteLength();
                        respMoveListMessagesSize+= (int)((double) moveList->getByteLength() / numRespSubspaces)
                        );
                sendMessageToUDP( childIt->first, (BaseOverlayMessage*) moveList->dup() );
                // ... but don't send msgs to too many cached children, as this would exhaust our bandwidth
            }
        }

        // ... all direct intermediates and intermediates with broken parent
        deque<PubSubSubspaceResponsible::IntermediateNode>::iterator iit;
        for( iit = subspace.intermediateNodes.begin(); iit != subspace.intermediateNodes.end(); ++iit )
        {
            int intermediatePos = iit - subspace.intermediateNodes.begin();
            if( intermediatePos >= maxChildren &&
                  !subspace.intermediateNodes[intermediatePos/maxChildren -1].node.isUnspecified() ) continue;
            if( !iit->node.isUnspecified() ) {
                RECORD_STATS(
                        ++numMoveListMessages;
                        moveListMessagesSize+= moveList->getByteLength();
                        respMoveListMessagesSize+= (int)((double) moveList->getByteLength() / numRespSubspaces)
                );
                sendMessageToUDP( iit->node, (BaseOverlayMessage*) moveList->dup() );
            }
        }

        delete moveList;
    }
}

void PubSubMMOG::setBootstrapedIcon()
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

void PubSubMMOG::startTimer( PubSubTimer* timer )
{
    if( !timer ) {
        EV << "[PubSubMMOG::startTimer() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    WARNING! Trying to start NULL timer @ " << thisNode << "\n"
           << endl;
        return;
    }

    if( timer->isScheduled() ) {
        cancelEvent( timer );
    }

    simtime_t duration = 0;
    switch( timer->getType() ) {
        case PUBSUB_HEARTBEAT:
            duration = parentTimeout/2;
            break;
        case PUBSUB_CHILDPING:
            duration = parentTimeout*10; // FIXME: make it a parameter
            break;
        case PUBSUB_PARENT_TIMEOUT:
            duration = parentTimeout;
            break;
        case PUBSUB_EVENTDELIVERY:
            duration = 1.0/movementRate;
            break;
    }
    scheduleAt(simTime() + duration, timer );
}

void PubSubMMOG::finishOverlay()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->addStdDev("PubSubMMOG: Sent Signaling Messages/s",
                                numPubSubSignalingMessages / time);
    globalStatistics->addStdDev("PubSubMMOG: Sent Signaling bytes/s",
                                pubSubSignalingMessagesSize / time);
    globalStatistics->addStdDev("PubSubMMOG: Sent Move Messages/s",
                                numMoveMessages / time);
    globalStatistics->addStdDev("PubSubMMOG: Sent Move bytes/s",
                                moveMessagesSize / time);
    globalStatistics->addStdDev("PubSubMMOG: Sent MoveList Messages/s",
                                numMoveListMessages / time);
    globalStatistics->addStdDev("PubSubMMOG: Sent MoveList bytes/s",
                                moveListMessagesSize / time);
    globalStatistics->addStdDev("PubSubMMOG: Received Move Events (correct timeslot)/s",
                                numEventsCorrectTimeslot / time);
    globalStatistics->addStdDev("PubSubMMOG: Received Move Events (wrong timeslot)/s",
                                numEventsWrongTimeslot / time);
    globalStatistics->addStdDev("PubSubMMOG: Responsible Nodes: Send MoveList Bytes/s",
                                respMoveListMessagesSize / time);
    globalStatistics->addStdDev("PubSubMMOG: Lost or too long delayed MoveLists/s",
                                lostMovementLists / time);
    globalStatistics->addStdDev("PubSubMMOG: Received valid MoveLists/s",
                                receivedMovementLists / time);
}

PubSubMMOG::~PubSubMMOG()
{
    // Delete all waiting move messages
    std::map<PubSubSubspaceId, PubSubSubspaceResponsible>::iterator it;
    for( it = responsibleSubspaces.begin(); it != responsibleSubspaces.end(); ++it) {
        deque<PubSubMoveMessage*>::iterator msgIt;
        for( msgIt = it->second.waitingMoveMessages.begin(); msgIt != it->second.waitingMoveMessages.end(); ++msgIt ){
            cancelAndDelete( *msgIt );
        }
        it->second.waitingMoveMessages.clear();
    }

    cancelAndDelete(heartbeatTimer);
}
