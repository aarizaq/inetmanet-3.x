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
 * @file Scribe.cc
 * @author Stephan Krause
 */

#include <assert.h>

#include <BaseApp.h>
#include "Scribe.h"
#include <GlobalStatistics.h>

#include "Comparator.h"

Define_Module(Scribe);

using namespace std;

Scribe::Scribe()
{
    subscriptionTimer = new ScribeTimer("Subscription timer");
    subscriptionTimer->setTimerType( SCRIBE_SUBSCRIPTION_REFRESH );
    numJoins = 0;
    numChildTimeout = 0;
    numParentTimeout = 0;
    numForward = 0;
    forwardBytes = 0;
    numReceived = 0;
    receivedBytes = 0;
    numHeartbeat = 0;
    heartbeatBytes = 0;
    numSubscriptionRefresh = 0;
    subscriptionRefreshBytes = 0;
}

Scribe::~Scribe()
{
    groupList.clear();
    cancelAndDelete(subscriptionTimer);
    // TODO: clear childTimeoutList
}

void Scribe::initializeApp(int stage)
{
    if( stage != (numInitStages()-1))
    {
        return;
    }
    WATCH(groupList);
    WATCH(numJoins);
    WATCH(numForward);
    WATCH(forwardBytes);
    WATCH(numReceived);
    WATCH(receivedBytes);
    WATCH(numHeartbeat);
    WATCH(heartbeatBytes);
    WATCH(numSubscriptionRefresh);
    WATCH(subscriptionRefreshBytes);
    WATCH(numChildTimeout);
    WATCH(numParentTimeout);

    childTimeout = par("childTimeout");
    parentTimeout = par("parentTimeout");

}

void Scribe::finishApp()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->addStdDev("Scribe: Received JOIN Messages/s",
            numJoins / time);
    globalStatistics->addStdDev("Scribe: Forwarded Multicast Messages/s",
            numForward / time);
    globalStatistics->addStdDev("Scribe: Forwarded Multicast Bytes/s",
            forwardBytes / time);
    globalStatistics->addStdDev("Scribe: Received Multicast Messages/s (subscribed groups only)",
            numReceived / time);
    globalStatistics->addStdDev("Scribe: Received Multicast Bytes/s (subscribed groups only)",
            receivedBytes / time);
    globalStatistics->addStdDev("Scribe: Send Heartbeat Messages/s",
            numHeartbeat / time);
    globalStatistics->addStdDev("Scribe: Send Heartbeat Bytes/s",
            heartbeatBytes / time);
    globalStatistics->addStdDev("Scribe: Send Subscription Refresh Messages/s",
            numSubscriptionRefresh / time);
    globalStatistics->addStdDev("Scribe: Send Subscription Refresh Bytes/s",
            subscriptionRefreshBytes / time);
    globalStatistics->addStdDev("Scribe: Number of Child Timeouts/s",
            numChildTimeout / time);
    globalStatistics->addStdDev("Scribe: Number of Parent Timeouts/s",
            numParentTimeout / time);
}

void Scribe::forward(OverlayKey* key, cPacket** msg,
                NodeHandle* nextHopNode)
{
    ScribeJoinCall* joinMsg = dynamic_cast<ScribeJoinCall*> (*msg);
    if( joinMsg == NULL ) {
        // nothing to be done
        return;
    }

    if( joinMsg->getSrcNode() == overlay->getThisNode() ) return;

    handleJoinMessage( joinMsg, false );

    *msg = NULL;
}

void Scribe::update( const NodeHandle& node, bool joined )
{
    // if node is closer to any group i'm root of, subscribe
    for( GroupList::iterator it = groupList.begin(); it != groupList.end(); ++it ){
        // if I'm root ...
        if( !it->second.getParent().isUnspecified()
                && it->second.getParent() == overlay->getThisNode() ) {
            KeyDistanceComparator<KeyRingMetric> comp( it->second.getGroupId() );
            // ... and new node is closer to groupId
            if( comp.compare(node.getKey(), overlay->getThisNode().getKey()) < 0){
                // then the new node is new group root, so send him a subscribe
                ScribeJoinCall* m = new ScribeJoinCall;
                m->setGroupId( it->second.getGroupId() );
                m->setBitLength( SCRIBE_JOINCALL_L(m) );
                sendRouteRpcCall(TIER1_COMP, node, m);
            }
        }
    }
}

bool Scribe::handleRpcCall(BaseCallMessage* msg)
{
    RPC_SWITCH_START(msg);
    RPC_DELEGATE(ScribeJoin, handleJoinCall);
    RPC_DELEGATE(ScribePublish, handlePublishCall);
    RPC_SWITCH_END();
    return RPC_HANDLED;
}

void Scribe::handleRpcResponse(BaseResponseMessage* msg,
                               cPolymorphic* context, int rpcId,
                               simtime_t rtt)
{
    RPC_SWITCH_START(msg);
    RPC_ON_RESPONSE( ScribeJoin ) {
        handleJoinResponse( _ScribeJoinResponse );
        EV << "[Scribe::handleRpcResponse() @ " << overlay->getThisNode().getIp()
            << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
            << "    Received a ScribeJoin RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_ScribeJoinResponse << " rtt=" << rtt
            << endl;
        break;
    }
    RPC_ON_RESPONSE( ScribePublish ) {
        handlePublishResponse( _ScribePublishResponse );
    }
    RPC_SWITCH_END( );
}

void Scribe::deliver(OverlayKey& key, cMessage* msg)
{
    if( ScribeSubscriptionRefreshMessage* refreshMsg =
            dynamic_cast<ScribeSubscriptionRefreshMessage*>(msg) ){
        // reset child timeout
        refreshChildTimer( refreshMsg->getSrc(), refreshMsg->getGroupId() );
        delete refreshMsg;
    } else if( ScribeDataMessage* data = dynamic_cast<ScribeDataMessage*>(msg) ){
        deliverALMDataToGroup( data );
    } else if( ScribeLeaveMessage* leaveMsg = dynamic_cast<ScribeLeaveMessage*>(msg) ){
        handleLeaveMessage( leaveMsg );
    }
}

void Scribe::handleUpperMessage( cMessage *msg )
{
    if( ALMSubscribeMessage* subscribeMsg = dynamic_cast<ALMSubscribeMessage*>(msg)){
        subscribeToGroup( subscribeMsg->getGroupId() );
        delete msg;
    } else if( ALMLeaveMessage* leaveMsg = dynamic_cast<ALMLeaveMessage*>(msg)){
        leaveGroup( leaveMsg->getGroupId() );
        delete msg;
    } else if( ALMMulticastMessage* mcastMsg = dynamic_cast<ALMMulticastMessage*>(msg) ){
        deliverALMDataToRoot( mcastMsg );
    } else if( ALMAnycastMessage* acastMsg = dynamic_cast<ALMAnycastMessage*>(msg) ){
        // FIXME: anycast not implemented yet
        EV << "[Scribe::handleUpperMessage() @ " << overlay->getThisNode().getIp()
            << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
            << "    Anycast message for group " << acastMsg->getGroupId() << "\n"
            << "    ignored: Not implemented yet!"
            << endl;
        delete msg;
    } else if( ALMCreateMessage* createMsg = dynamic_cast<ALMCreateMessage*>(msg) ){
        EV << "[Scribe::handleUpperMessage() @ " << overlay->getThisNode().getIp()
            << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
            << "    Create message for group " << createMsg->getGroupId() << "\n"
            << "    ignored: Scribe has implicit create on SUBSCRIBE"
            << endl;
        delete msg;
    } else if( ALMDeleteMessage* deleteMsg = dynamic_cast<ALMDeleteMessage*>(msg) ){
        EV << "[Scribe::handleUpperMessage() @ " << overlay->getThisNode().getIp()
            << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
            << "    Delete message for group " << deleteMsg->getGroupId() << "\n"
            << "    ignored: Scribe has implicit delete on LEAVE"
            << endl;
        delete msg;
    }
}

void Scribe::handleReadyMessage( CompReadyMessage* msg )
{
    // process only ready messages from the tier below
    if( (getThisCompType() - msg->getComp() == 1) && msg->getReady() ) {

        // Send a ready message to the tier above
        CompReadyMessage* readyMsg = new CompReadyMessage;
        readyMsg->setReady( true );
        readyMsg->setComp( getThisCompType() );

        send( readyMsg, "to_upperTier" );
        
        startTimer( subscriptionTimer );
    }
    delete msg;
}

void Scribe::handleTimerEvent( cMessage *msg )
{
    ScribeTimer* timer = dynamic_cast<ScribeTimer*>(msg);
    assert( timer );
    switch( timer->getTimerType() ) {
        case SCRIBE_SUBSCRIPTION_REFRESH:
            // renew subscriptions for all groups
            for( GroupList::iterator it = groupList.begin(); it != groupList.end(); ++it ) {
                NodeHandle parent = it->second.getParent();
                if( !parent.isUnspecified() ){
                    ScribeSubscriptionRefreshMessage* refreshMsg = new ScribeSubscriptionRefreshMessage;
                    refreshMsg->setGroupId( it->second.getGroupId() );
                    refreshMsg->setSrc( overlay->getThisNode() );

                    refreshMsg->setBitLength(SCRIBE_SUBSCRIPTIONREFRESH_L(refreshMsg));
                    RECORD_STATS(++numSubscriptionRefresh;
                            subscriptionRefreshBytes += refreshMsg->getByteLength()
                    );
                    callRoute( OverlayKey::UNSPECIFIED_KEY, refreshMsg, parent );
                }
            }
            startTimer( subscriptionTimer );
            break;

        case SCRIBE_HEARTBEAT:
        {
            // Send heartbeat messages to all children in the group
            GroupList::iterator groupIt = groupList.find( timer->getGroup() );
            if( groupIt == groupList.end() ) {
                // FIXME: should not happen
                delete timer;
                return;
            }
            for( set<NodeHandle>::iterator it = groupIt->second.getChildrenBegin();
                    it != groupIt->second.getChildrenEnd(); ++it ) {
                ScribeDataMessage* heartbeatMsg = new ScribeDataMessage("Heartbeat");
                heartbeatMsg->setEmpty( true );
                heartbeatMsg->setGroupId( timer->getGroup() );

                heartbeatMsg->setBitLength(SCRIBE_DATA_L(heartbeatMsg));
                RECORD_STATS(++numHeartbeat; heartbeatBytes += heartbeatMsg->getByteLength());
                callRoute( OverlayKey::UNSPECIFIED_KEY, heartbeatMsg, *it );
            }
            startTimer( timer );
            break;
        }
        case SCRIBE_CHILD_TIMEOUT:
            // Child failed, remove it from group
            RECORD_STATS(++numChildTimeout);
            removeChildFromGroup( timer );
            break;

        case SCRIBE_PARENT_TIMEOUT:
            // Parent failed, send new join to rejoin group
            RECORD_STATS(++numParentTimeout);
            OverlayKey key = timer->getGroup();
            EV << "[Scribe::handleTimerEvent() @ " << overlay->getThisNode().getIp()
                << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                << "    Parent of group " << key << "\n"
                << "    failed to send heartbeat, trying to rejoin"
                << endl;

            ScribeJoinCall* newJoin = new ScribeJoinCall;
            newJoin->setGroupId( key );
            newJoin->setBitLength( SCRIBE_JOINCALL_L(newJoin) );
            sendRouteRpcCall(TIER1_COMP, key, newJoin);

            GroupList::iterator groupIt = groupList.find( timer->getGroup() );
            if( groupIt == groupList.end() ) {
                // FIXME: should not happen
                delete timer;
                return;
            }
            groupIt->second.setParentTimer( NULL );
            cancelAndDelete( timer );
            break;
    }

}

void Scribe::handleJoinCall( ScribeJoinCall* joinMsg)
{
    handleJoinMessage( joinMsg, true );
}

void Scribe::handleJoinMessage( ScribeJoinCall* joinMsg, bool amIRoot)
{
    RECORD_STATS(++numJoins);
    OverlayKey key = joinMsg->getGroupId();

    EV << "[Scribe::handleJoinMessage() @ " << overlay->getThisNode().getIp()
        << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
        << "    Received a ScribeJoin for group " << key << "\n"
        << "    msg=" << joinMsg
        << endl;

    // Insert group into grouplist, if not already there
    pair<GroupList::iterator, bool> groupInserter;
    groupInserter = groupList.insert( make_pair(key, ScribeGroup(key)) );

    // If group is new or no parent is known, send join to parent (unless I am root, so there is no parent)
    if ( !amIRoot && ( groupInserter.second || groupInserter.first->second.getParent().isUnspecified()) ) {
        ScribeJoinCall* newJoin = new ScribeJoinCall;
        newJoin->setGroupId( key );
        newJoin->setBitLength( SCRIBE_JOINCALL_L(newJoin) );
        sendRouteRpcCall(TIER1_COMP, key, newJoin);
    }

    // If group had no children, start heartbeat timer for group
    if( groupInserter.first->second.numChildren() == 0 ) {
        ScribeTimer* heartbeat = new ScribeTimer("HeartbeatTimer");
        heartbeat->setTimerType( SCRIBE_HEARTBEAT );
        heartbeat->setGroup( groupInserter.first->second.getGroupId() );
        startTimer( heartbeat );
        if( ScribeTimer* t = groupInserter.first->second.getHeartbeatTimer() ){
            // delete old timer, if any
            if( t ) cancelAndDelete( t );
        }
        groupInserter.first->second.setHeartbeatTimer( heartbeat );
    }

    // Add child to group
    addChildToGroup( joinMsg->getSrcNode(), groupInserter.first->second );

    // Send joinResponse
    ScribeJoinResponse* joinResponse = new ScribeJoinResponse;
    joinResponse->setGroupId( key );
    joinResponse->setBitLength( SCRIBE_JOINRESPONSE_L(joinResponse) );
    sendRpcResponse( joinMsg, joinResponse );
}

void Scribe::handlePublishCall( ScribePublishCall* publishCall )
{
    // find group
    GroupList::iterator it = groupList.find( publishCall->getGroupId() );
    if( it == groupList.end() ||
            it->second.getParent().isUnspecified() ||
            it->second.getParent() != overlay->getThisNode() ){
        // if I don't know the group or I am not root, inform sender
        // TODO: forward message when I'm not root but know the rendevous point?
        ScribePublishResponse* msg = new ScribePublishResponse("Wrong Root");
        msg->setGroupId( publishCall->getGroupId() );
        msg->setWrongRoot( true );
        msg->setBitLength( SCRIBE_PUBLISHRESPONSE_L(msg) );
        sendRpcResponse( publishCall, msg );
    } else {
        ScribeDataMessage* data = dynamic_cast<ScribeDataMessage*>(publishCall->decapsulate());

        ScribePublishResponse* msg = new ScribePublishResponse("Publish Successful");
        msg->setGroupId( publishCall->getGroupId() );
        msg->setBitLength( SCRIBE_PUBLISHRESPONSE_L(msg) );
        sendRpcResponse( publishCall, msg );

        if( !data ) {
            // TODO: throw exception? this should never happen
            EV << "[Scribe::handlePublishCall() @ " << overlay->getThisNode().getIp()
                << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
                << "    PublishCall for group " << msg->getGroupId()
                << "    does not contain a calid ALM data message!\n"
                << endl;
            return;
        }
        deliverALMDataToGroup( data );
    }
}

void Scribe::handleJoinResponse( ScribeJoinResponse* joinResponse )
{
    GroupList::iterator it = groupList.find( joinResponse->getGroupId() );
    if( it == groupList.end() ) {
        EV << "[Scribe::handleJoinResponse() @ " << overlay->getThisNode().getIp()
            << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
            << "Getting join response for an unknown group!\n";
        return;
    }
    it->second.setParent( joinResponse->getSrcNode() );

    // Create new heartbeat timer
    ScribeTimer* parentTimeout = new ScribeTimer("ParentTimeoutTimer");
    parentTimeout->setTimerType( SCRIBE_PARENT_TIMEOUT );
    parentTimeout->setGroup( it->second.getGroupId() );
    startTimer( parentTimeout );
    if( ScribeTimer* t = it->second.getParentTimer() ){
        // delete old timer, if any
        if( t ) cancelAndDelete( t );
    }
    it->second.setParentTimer( parentTimeout );
}

void Scribe::handlePublishResponse( ScribePublishResponse* publishResponse )
{
    GroupList::iterator it = groupList.find( publishResponse->getGroupId() );
    if( it == groupList.end() ) {
        EV << "[Scribe::handlePublishResponse() @ " << overlay->getThisNode().getIp()
            << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
            << "Getting publish response for an unknown group!\n";
        return;
    }

    if( publishResponse->getWrongRoot() ) {
        it->second.setRendezvousPoint( NodeHandle::UNSPECIFIED_NODE );
    } else {
        it->second.setRendezvousPoint( publishResponse->getSrcNode() );
    }
}

void Scribe::handleLeaveMessage( ScribeLeaveMessage* leaveMsg )
{
    GroupList::iterator it = groupList.find( leaveMsg->getGroupId() );
    if( it != groupList.end() ){
        removeChildFromGroup( leaveMsg->getSrc(), it->second );
    }
    delete leaveMsg;
}

void Scribe::subscribeToGroup( const OverlayKey& groupId )
{
    EV << "[Scribe::subscribeToGroup() @ " << overlay->getThisNode().getIp()
        << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
        << "   Trying to join group " << groupId << "\n";

    // Insert group into grouplist, if not known yet
    pair<GroupList::iterator, bool> groupInserter;
    groupInserter = groupList.insert( make_pair(groupId, ScribeGroup(groupId)) );

    // Set subscription status
    groupInserter.first->second.setSubscription(true);

    // Send join call if I'm not already a forwarder of this group
    if( groupInserter.second || groupInserter.first->second.getParent().isUnspecified()) {
        ScribeJoinCall* m = new ScribeJoinCall;
        m->setGroupId( groupId );
        m->setBitLength( SCRIBE_JOINCALL_L(m) );
        sendRouteRpcCall(TIER1_COMP, m->getGroupId(), m);
    }
}

void Scribe::leaveGroup( const OverlayKey& group )
{
    GroupList::iterator it = groupList.find( group );
    if( it != groupList.end() ){
        it->second.setSubscription( false );
        checkGroupEmpty( it->second );
    }
}

void Scribe::addChildToGroup( const NodeHandle& child, ScribeGroup& group )
{
    if( child == overlay->getThisNode() ) {
        // Join from ourself, ignore
        return;
    }

    // add child to group's children list
    pair<set<NodeHandle>::iterator, bool> inserter =
        group.addChild( child );

    if( inserter.second ) {
        // if child has not been in the list, create new timeout msg
        ScribeTimer* timeoutMsg = new ScribeTimer;
        timeoutMsg->setTimerType( SCRIBE_CHILD_TIMEOUT );

        // Remember child and group
        timeoutMsg->setChild( *inserter.first );
        timeoutMsg->setGroup( group.getGroupId() );

        startTimer( timeoutMsg );
        childTimeoutList.insert( make_pair(child, timeoutMsg) );
    }
}

void Scribe::removeChildFromGroup( const NodeHandle& child, ScribeGroup& group )
{
    // find timer
    ScribeTimer* timer = NULL;
    pair<ChildTimeoutList::iterator, ChildTimeoutList::iterator> ret =
        childTimeoutList.equal_range( child );
    if( ret.first != childTimeoutList.end() ){
        for( ChildTimeoutList::iterator it = ret.first; it!=ret.second; ++it) {
            if( group == it->second->getGroup() ) {
                timer = it->second;
                childTimeoutList.erase( it );
                cancelAndDelete( timer );
                break;
            }
        }
    }

    // remove child from group's childrenlist
    group.removeChild( child );

    checkGroupEmpty( group );
}

void Scribe::removeChildFromGroup( ScribeTimer* timer )
{
    NodeHandle& child = timer->getChild();

    GroupList::iterator groupIt = groupList.find( timer->getGroup() );
    if( groupIt != groupList.end() ) {
        ScribeGroup& group = groupIt->second;
        // remove child from group's childrenlist
        group.removeChild( child );

        checkGroupEmpty( group );
    }

    // remove timer from timeoutlist
    pair<ChildTimeoutList::iterator, ChildTimeoutList::iterator> ret =
        childTimeoutList.equal_range( child );
    if( ret.first != childTimeoutList.end() ) {
        for( ChildTimeoutList::iterator it = ret.first; it!=ret.second; ++it) {
            if( it->second == timer ) {
                childTimeoutList.erase( it );
                cancelAndDelete( timer );
                break;
            }
        }
    }
}

void Scribe::checkGroupEmpty( ScribeGroup& group )
{
    if( !group.isForwarder() && !group.getSubscription() && !group.getAmISource()){

        if( !group.getParent().isUnspecified() && group.getParent() != overlay->getThisNode() ) {

            ScribeLeaveMessage* msg = new ScribeLeaveMessage("Leave");
            msg->setGroupId( group.getGroupId() );
            msg->setSrc( overlay->getThisNode() );
            msg->setBitLength( SCRIBE_LEAVE_L(msg) );
            callRoute( OverlayKey::UNSPECIFIED_KEY, msg, group.getParent() );
        }

        if( group.getParentTimer() ) cancelAndDelete( group.getParentTimer() );
        if( group.getHeartbeatTimer() ) cancelAndDelete( group.getHeartbeatTimer() );
        groupList.erase( group.getGroupId() );
    }
}

void Scribe::refreshChildTimer( NodeHandle& child, OverlayKey& groupId )
{
    // find timer
    pair<ChildTimeoutList::iterator, ChildTimeoutList::iterator> ret =
        childTimeoutList.equal_range( child );
    // no timer yet?
    if( ret.first == childTimeoutList.end() ) return;

    // restart timer
    for( ChildTimeoutList::iterator it = ret.first; it!=ret.second; ++it) {
        if( it->first == child && it->second->getGroup() == groupId ) {
            startTimer( it->second );
        }
    }
}

void Scribe::startTimer( ScribeTimer* timer )
{
    if( timer->isScheduled() ) {
        cancelEvent( timer );
    }

    int duration = 0;
    switch( timer->getTimerType() ) {
        case SCRIBE_HEARTBEAT:
            duration = parentTimeout/2;
            break;
        case SCRIBE_SUBSCRIPTION_REFRESH:
            duration = childTimeout/2;
            break;
        case SCRIBE_PARENT_TIMEOUT:
            duration = parentTimeout;
            break;
        case SCRIBE_CHILD_TIMEOUT:
            duration = childTimeout;
            break;
    }
    scheduleAt(simTime() + duration, timer );
}

void Scribe::deliverALMDataToRoot( ALMMulticastMessage* mcastMsg )
{
    // find group
    pair<GroupList::iterator, bool> groupInserter;
    groupInserter = groupList.insert( make_pair(mcastMsg->getGroupId(), ScribeGroup(mcastMsg->getGroupId())) );

    // Group is not known yet
    if( groupInserter.second ) {
        groupInserter.first->second.setAmISource( true );
        // TODO: Start/Restart timer to clean up cached groups
        // If the timer expires, the flag should be cleared and checkGroupEmpty should be called
        //
        // FIXME: amISource should be set allways if app publishes messages to the group
        // As the timer is not implemented yet, we only set the flag in "sender, but not receiver" mode
        // to reduce the amount of unneccessary cached groups
    }

    ScribeDataMessage* dataMsg = new ScribeDataMessage( mcastMsg->getName() );
    dataMsg->setGroupId( mcastMsg->getGroupId() );
    dataMsg->setBitLength( SCRIBE_DATA_L( dataMsg ));
    dataMsg->encapsulate( mcastMsg->decapsulate() );

    // Send publish ...
    ScribePublishCall* msg = new ScribePublishCall( "ScribePublish" );
    msg->setGroupId( dataMsg->getGroupId() );
    msg->setBitLength( SCRIBE_PUBLISHCALL_L(msg) );
    msg->encapsulate( dataMsg );

    if( !groupInserter.first->second.getRendezvousPoint().isUnspecified() ) {
        // ... directly to the rendevouz point, if known ...
        sendRouteRpcCall(TIER1_COMP, groupInserter.first->second.getRendezvousPoint(), msg);
    } else {
        // ... else route it via KBR
        sendRouteRpcCall(TIER1_COMP, msg->getGroupId(), msg);
    }

    delete mcastMsg;
}


void Scribe::deliverALMDataToGroup( ScribeDataMessage* dataMsg )
{
    // find group
    GroupList::iterator it = groupList.find( dataMsg->getGroupId() );
    if( it == groupList.end() ) {
        EV << "[Scribe::deliverALMDataToGroup() @ " << overlay->getThisNode().getIp()
            << "Getting ALM data message response for an unknown group!\n";
        delete dataMsg;
        return;
    }
    // FIXME: ignore message if not from designated parent to avoid duplicates

    // reset parent heartbeat Timer
    ScribeTimer *timer = it->second.getParentTimer();
    if( timer ) startTimer( timer );

    // Only empty heartbeat?
    if( dataMsg->getEmpty() ) {
        delete dataMsg;
        return;
    }

    // deliver data to children
    for( set<NodeHandle>::iterator cit = it->second.getChildrenBegin();
            cit != it->second.getChildrenEnd(); ++cit ) {
        ScribeDataMessage* newMsg = new ScribeDataMessage( *dataMsg );
        RECORD_STATS(++numForward; forwardBytes += newMsg->getByteLength());
        callRoute( OverlayKey::UNSPECIFIED_KEY, newMsg, *cit );
    }

    // deliver to myself if I'm subscribed to that group
    if( it->second.getSubscription() ) {
        ALMMulticastMessage* mcastMsg = new ALMMulticastMessage( dataMsg->getName() );
        mcastMsg->setGroupId( dataMsg->getGroupId() );
        mcastMsg->encapsulate( dataMsg->decapsulate() );
        RECORD_STATS(++numReceived; receivedBytes += dataMsg->getByteLength());
        send( mcastMsg, "to_upperTier" );
    }

    delete dataMsg;
}

