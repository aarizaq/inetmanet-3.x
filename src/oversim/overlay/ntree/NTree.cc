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
 * @file NTree.cc
 * @author Stephan Krause
 */

#include <NotifierConsts.h>

#include "NTree.h"

#include "GlobalNodeListAccess.h"
#include <GlobalStatistics.h>
#include <BootstrapList.h>

Define_Module(NTree);

using namespace std;

// TODO: Use standard join() for login

// TODO: On move receive, check if sender is in member list, add
// TODO: On group leave, check for now-lost members, tell app to remove them
// TODO: On divide/collapse/etc (i.e. when deleting groups), check for lost members, tell app to remove them(?)
// TODO: Timeout players when no move received for longer period?

void NTree::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if(stage != MIN_STAGE_OVERLAY) return;

    maxChildren = par("maxChildren");
    if( maxChildren < 5 ){
        throw new cRuntimeError("To allow a proper quad-tree, maxChildren has to be 5 or bigger");
    }
    AOIWidth = par("AOIWidth");

    thisNode.setKey(OverlayKey::random());

    areaDimension = par("areaDimension");
    currentGroup = NULL;

    joinTimer = new cMessage("joinTimer");
    pingTimer = new cMessage("pingTimer");
    pingInterval = par("pingInterval");
    scheduleAt( simTime() + pingInterval, pingTimer );
    
    // FIXME: Use standard baseOverlay::join()
    changeState( INIT );

    // statistics
    joinsSend = 0;
    joinBytes = 0;
    joinTimeout = 0;

    divideCount = 0;
    collapseCount = 0;

    treeMaintenanceMessages = 0;
    treeMaintenanceBytes = 0;

    movesSend = 0;
    moveBytes = 0;

    WATCH_MAP( ntreeNodes );
    WATCH_LIST( groups );
}

bool NTree::handleRpcCall(BaseCallMessage* msg)
{
    if( state == SHUTDOWN ){
        return false;
    }
    // delegate messages
    RPC_SWITCH_START( msg )
    RPC_DELEGATE( NTreeJoin, handleJoinCall );
    RPC_DELEGATE( NTreePing, handlePingCall );
    RPC_DELEGATE( NTreeDivide, handleDivideCall );
    RPC_SWITCH_END( )

    return RPC_HANDLED;

}

void NTree::handleRpcResponse(BaseResponseMessage *msg,
                                   cPolymorphic* context, int rpcId,
                                   simtime_t rtt)
{
    if( state == SHUTDOWN ){
        return;
    }
    RPC_SWITCH_START(msg);
    RPC_ON_RESPONSE( NTreeJoin ) {
        EV << "[NTree::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a NTreeJoin RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_NTreeJoinResponse << " rtt=" << rtt
            << endl;
        handleJoinResponse( _NTreeJoinResponse );
        break;
    }
    RPC_ON_RESPONSE( NTreePing ) {
        EV << "[NTree::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a NTreePing RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_NTreePingResponse << " rtt=" << rtt
            << endl;
        handlePingResponse( _NTreePingResponse, dynamic_cast<NTreePingContext*>(context) );
        break;
    }
    RPC_ON_RESPONSE( NTreeDivide ) {
        EV << "[NTree::handleRpcResponse() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received a NTreeDivide RPC Response: id=" << rpcId << "\n"
            << "    msg=" << *_NTreeDivideResponse << " rtt=" << rtt
            << endl;
        handleDivideResponse( _NTreeDivideResponse, check_and_cast<NTreeGroupDivideContextPtr*>(context)->ptr );
        delete context;
        break;
    }
    RPC_SWITCH_END( );
}

void NTree::handleRpcTimeout (BaseCallMessage *msg,
                                   const TransportAddress &dest,
                                   cPolymorphic* context, int rpcId,
                                   const OverlayKey &destKey)
{
    if( state == SHUTDOWN ){
        return;
    }
    RPC_SWITCH_START(msg)
    RPC_ON_CALL( NTreePing ) {
        EV << "[NTree::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Ping RPC Call timed out: id=" << rpcId << "\n"
           << "    msg=" << *_NTreePingCall
           << "    oldNode=" << dest
           << endl;
        handlePingCallTimeout( _NTreePingCall, dest, dynamic_cast<NTreePingContext*>(context) );
        break;
    }
    RPC_ON_CALL( NTreeJoin ) {
        EV << "[NTree::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Join RPC Call timed out: id=" << rpcId << "\n"
           << "    msg=" << *_NTreeJoinCall
           << "    oldNode=" << dest
           << endl;
        handleJoinCallTimeout( _NTreeJoinCall, dest );
        break;
    }
    RPC_ON_CALL( NTreeDivide ) {
        EV << "[NTree::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    Ping RPC Call timed out: id=" << rpcId << "\n"
           << "    msg=" << *_NTreeDivideCall
           << "    oldNode=" << dest
           << endl;
        handleDivideCallTimeout( _NTreeDivideCall, dest, check_and_cast<NTreeGroupDivideContextPtr*>(context)->ptr );
        delete context;
        break;
    }
    RPC_SWITCH_END( )

}

void NTree::handleUDPMessage(BaseOverlayMessage* msg)
{
    if( state == SHUTDOWN ){
        delete msg;
        return;
    }
    if( NTreeMoveMessage* moveMsg = dynamic_cast<NTreeMoveMessage*>(msg) ){
        handleMoveMessage( moveMsg );
        delete moveMsg;
    } else if( NTreeGroupAddMessage* addMsg = dynamic_cast<NTreeGroupAddMessage*>(msg) ){
        handleAddMessage( addMsg );
        delete addMsg;
    } else if( NTreeGroupDeleteMessage* deleteMsg = dynamic_cast<NTreeGroupDeleteMessage*>(msg) ){
        handleDeleteMessage( deleteMsg );
        delete deleteMsg;
    } else if( NTreeLeaveMessage* leaveMsg = dynamic_cast<NTreeLeaveMessage*>(msg) ){
        handleLeaveMessage( leaveMsg );
        delete leaveMsg;
    } else if( NTreeCollapseMessage* collapseMsg = dynamic_cast<NTreeCollapseMessage*>(msg) ){
        handleCollapseMessage( collapseMsg );
        delete collapseMsg;
    } else if( NTreeReplaceNodeMessage* replaceMsg = dynamic_cast<NTreeReplaceNodeMessage*>(msg) ){
        handleReplaceMessage( replaceMsg );
        delete replaceMsg;
    } else if( NTreeTakeOverMessage* takeMsg = dynamic_cast<NTreeTakeOverMessage*>(msg) ){
        handleTakeOverMessage( takeMsg );
        delete takeMsg;
    }
}

void NTree::handleTimerEvent(cMessage* msg)
{
    if( state == SHUTDOWN ){
        return;
    }
    if( msg == joinTimer ) {
        // send a fake ready message to app to get initial position
        CompReadyMessage* msg = new CompReadyMessage("fake READY");
        msg->setReady(true);
        msg->setComp(getThisCompType());
        send( msg, "appOut");
        // send initial AOI size to the application
        GameAPIResizeAOIMessage* gameMsg = new GameAPIResizeAOIMessage("RESIZE_AOI");
        gameMsg->setCommand(RESIZE_AOI);
        gameMsg->setAOIsize(AOIWidth);
        send( gameMsg, "appOut");
    } else if( msg == pingTimer ){
        pingNodes();
        checkParentTimeout();
        scheduleAt( simTime() + pingInterval, pingTimer );
    }
}

void NTree::handleAppMessage(cMessage* msg)
{
    if( GameAPIPositionMessage *posMsg = dynamic_cast<GameAPIPositionMessage*>(msg) ) {
        if( state == READY ) {
            handleMove( posMsg );
        } else if ( state == JOINING ) {
            // We are not connected to the overlay, inform app
            CompReadyMessage* msg = new CompReadyMessage("Overlay not READY!");
            msg->setReady(false);
            msg->setComp(getThisCompType());
            send( msg, "appOut");
        } else if ( state == INIT ) {
            // This is only called for the first MOVE message
            TransportAddress bootstrapNode = bootstrapList->getBootstrapNode();
            if( bootstrapNode.isUnspecified() ){
                NTreeGroup grp(Vector2D(areaDimension/2,areaDimension/2), areaDimension);
                grp.leader = thisNode;
                grp.members.insert(thisNode);
                groups.push_front(grp);
                
                NTreeScope initialScope(Vector2D(areaDimension/2,areaDimension/2), areaDimension);
                NTreeNode root(initialScope);
                root.group = &groups.front();
                ntreeNodes.insert(make_pair(initialScope,root));
                changeState( READY );
            } else {
                // Trigger login
                changeState( JOINING );
                position = posMsg->getPosition();
                NTreeJoinCall* joinMsg = new NTreeJoinCall("Login");
                joinMsg->setPosition( position );
                joinMsg->setBitLength( NTREEJOINCALL_L(joinMsg) );
                sendUdpRpcCall( bootstrapNode, joinMsg );
            }

            setBootstrapedIcon();

        }
        delete msg;
    }
}


void NTree::handleJoinCall( NTreeJoinCall* joinCall )
{
    // Check if we know the group
    std::list<NTreeGroup>::iterator grpIt = findGroup( joinCall->getPosition() );
    if( grpIt != groups.end() ) {
        // If we are leader, acknowlede Join
        if( grpIt->leader == thisNode ) {
            // Inform group about new member
            NTreeGroupAddMessage* grpAdd = new NTreeGroupAddMessage("New group member");
            grpAdd->setPlayer( joinCall->getSrcNode() );
            grpAdd->setOrigin( grpIt->scope.origin );
            grpAdd->setBitLength( NTREEADD_L(grpAdd) );
            
            RECORD_STATS(
                    treeMaintenanceMessages += grpIt->members.size();
                    treeMaintenanceBytes += grpIt->members.size()*grpAdd->getByteLength();
            );
            sendToGroup( *grpIt, grpAdd );

            grpIt->members.insert( joinCall->getSrcNode() );
            // Acknowledge Join, send information about group
            NTreeJoinResponse* joinResp = new NTreeJoinResponse("Join ACK");
            joinResp->setOrigin( grpIt->scope.origin );
            joinResp->setSize( grpIt->scope.size );
            joinResp->setMembersArraySize( grpIt->members.size() );
            unsigned int i = 0;
            for( std::set<NodeHandle>::iterator it = grpIt->members.begin(); it != grpIt->members.end(); ++it ) {
                joinResp->setMembers( i, *it );
                ++i;
            }
            joinResp->setBitLength( NTREEJOINRESPONSE_L(joinResp) );
            sendRpcResponse( joinCall, joinResp );

            // If group is to big, divide group
            if( grpIt->members.size() > maxChildren && !grpIt->dividePending && grpIt->scope.size > 2*AOIWidth ){
                EV << "[NTree::handleJoinCall() @ " << thisNode.getIp()
                    << " (" << thisNode.getKey().toString(16) << ")]\n"
                    << "    Group got too big. Trying to divide group\n";
                grpIt->dividePending = true;
                std::map<NTreeScope,NTreeNode>::iterator nit = ntreeNodes.find( grpIt->scope );
                if( nit == ntreeNodes.end() ){
                    EV << "    Host is leader of a group, but not NTreeNode of that group.\n"
                        << "    This should not happen. Dropping group\n";
                    if( currentGroup == &(*grpIt) ){
                        currentGroup = NULL;
                    }
                    groups.erase( grpIt );
                    return;
                }
                EV << "    Sending divide calls\n";
                NTreeDivideCall* divideCall = new NTreeDivideCall("Divide Group");
                divideCall->setOrigin( nit->second.scope.origin );
                divideCall->setSize( nit->second.scope.size );
                divideCall->setBitLength( NTREEDIVIDECALL_L(divideCall) );

                NTreeGroupDivideContext* divideContext = new NTreeGroupDivideContext;
                divideContext->nodeScope = nit->second.scope;

                // select 4 random members of the group to become leader of subgroup
                // "lotto" algoritm so nobody gets elected twice
                unsigned int numMembers = grpIt->members.size();
                unsigned int numbers[numMembers];
                for( i = 0; i < numMembers; ++i ) numbers[i] = i;
                unsigned int upperbound = numMembers;
                for (i = 0; i < 4; ++i)
                {
                    upperbound--;
                    unsigned int index = intuniform(0, upperbound);
                    std::set<NodeHandle>::iterator newChild = grpIt->members.begin();
                    std::advance( newChild, numbers[index] );
                    
                    if( *newChild == thisNode ) { // don't select myself
                        --i;
                    } else {
                        NTreeDivideCall* msg = divideCall->dup();
                        msg->setQuadrant( i );
                        EV << "    ==> " << (TransportAddress) *newChild << "\n";
                        NTreeGroupDivideContextPtr* contextPtr = new NTreeGroupDivideContextPtr;
                        contextPtr->ptr = divideContext;
                        sendUdpRpcCall( *newChild, msg, contextPtr);
                        RECORD_STATS(
                                treeMaintenanceMessages++;
                                treeMaintenanceBytes += msg->getByteLength();
                        );
                    }

                    swap(numbers[index], numbers[upperbound]);
                }
                delete divideCall;
            }

        } else {
            // forward join to leader
            sendMessageToUDP( grpIt->leader, joinCall );
        }
        return;
    }
    // We don't know the group, forward join via the NTree
    routeViaNTree( joinCall->getPosition(), joinCall, true );
}

void NTree::handleJoinResponse( NTreeJoinResponse* joinResp )
{
    EV << "[NTree::handleJoinResponse() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    JOIN ACK for group " << joinResp->getOrigin()
        << "-" << joinResp->getSize() << "\n";
    std::list<NTreeGroup>::iterator git = findGroup( joinResp->getOrigin(), joinResp->getSize() );
    // Check for conflicting groups
    NTreeScope groupScope( joinResp->getOrigin(), joinResp->getSize() );
    for( std::list<NTreeGroup>::iterator it = groups.begin(); it != groups.end(); ++it ){
        if( it != git && (it->isInScope( groupScope.origin ) || groupScope.contains( it->scope.origin ) )){
            // Oops, conflicting group found. joinResp is probably outdated
            EV << "    Conflicting group found. Ignoring JOIN ACK\n";

            // if our login resulted in a divide, and we got the divide earlier that the ack,
            // we missed to update our state. fixing.
            if( state != READY ){
                changeState( READY );
            }
            return;
        }
    }

    // if group already known, clear member list (new member list will be taken from message)
    if( git != groups.end() ){
        if( git->leader != thisNode ) {
            EV << "    Group already known. Refreshing member list\n";
            git->members.clear();
        } else {
            // We are leader of the already known group. Something's fishy, ignore join
            EV << "    Group already known; and I'm leader of that group. Ignoring ACK\n";
            return;
        }

    } else {
        // Else create new group
        NTreeGroup grp(joinResp->getOrigin(), joinResp->getSize());
        groups.push_front(grp);
        git = groups.begin();
    }

    // Fill in member list and leader of the group from message
    git->leader = joinResp->getSrcNode();
    for( unsigned int i = 0; i < joinResp->getMembersArraySize(); ++i ){
        git->members.insert( joinResp->getMembers(i) );
    }

    // If we were still joining the overlay, we are now ready to go
    if( state != READY ){
        changeState( READY );
    }
}


void NTree::handleJoinCallTimeout( NTreeJoinCall* joinCall, const TransportAddress& oldNode)
{
    if( state != READY ){
        EV << "    Overlay not ready. Trying to re-join overlay\n";
        changeState( INIT );
    }
    joinTimeout++;
    // TODO: Other cases when re-try is useful?
}

void NTree::handleMove( GameAPIPositionMessage* posMsg )
{
    position = posMsg->getPosition();
    NTreeMoveMessage* moveMsg = new NTreeMoveMessage("MOVE");
    moveMsg->setPlayer( thisNode );
    moveMsg->setPosition( position );
    moveMsg->setBitLength( NTREEMOVE_L(moveMsg) );

    // Send to old group
    if( currentGroup ){
        sendToGroup( *currentGroup, moveMsg, true);
        RECORD_STATS(
                movesSend += currentGroup->members.size();
                moveBytes += moveMsg->getByteLength() * currentGroup->members.size();
                globalStatistics->addStdDev( "NTree: Area size", currentGroup->scope.size );
        );
    }

    // Find group for new position
    if( !currentGroup || !currentGroup->isInScope( position ) ){
        std::list<NTreeGroup>::iterator it = findGroup( position );
        if( it != groups.end() ) {
            sendToGroup( *it, moveMsg, true );
            currentGroup = &(*it);
            RECORD_STATS(
                    movesSend += currentGroup->members.size();
                    moveBytes += moveMsg->getByteLength() * currentGroup->members.size();
            );
        } else {
            EV << "[NTree::handleMove() @ " << thisNode.getIp()
                << " (" << thisNode.getKey().toString(16) << ")]\n"
                << "WARNING: no group for new position. Some updates may get lost!\n";
            joinGroup( position );
        }
    }

    delete moveMsg;

    if( currentGroup ){
        // Join new groups in our AoI and leave groups outside the AoI
        NTreeScope& scope = currentGroup->scope;
        NTreeScope area(Vector2D(areaDimension/2.0,areaDimension/2.0),areaDimension);
        Vector2D aoiRight(AOIWidth/2.0, 0);
        Vector2D aoiTop(0, AOIWidth/2.0);

        Vector2D groupRight(scope.size/2.0 +1, 0);
        Vector2D groupTop(0, scope.size/2.0 +1);

        if(area.contains( position + aoiRight )) joinGroup(position + aoiRight);
        if(area.contains( position - aoiRight )) joinGroup(position - aoiRight);
        if(area.contains( position + aoiTop )) joinGroup(position + aoiTop);
        if(area.contains( position - aoiTop )) joinGroup(position - aoiTop);
        if(area.contains( position + (aoiRight + aoiTop)/sqrt(2))) joinGroup(position + (aoiRight + aoiTop)/sqrt(2));
        if(area.contains( position + (aoiRight - aoiTop)/sqrt(2))) joinGroup(position + (aoiRight - aoiTop)/sqrt(2));
        if(area.contains( position - (aoiRight + aoiTop)/sqrt(2))) joinGroup(position - (aoiRight + aoiTop)/sqrt(2));
        if(area.contains( position - (aoiRight - aoiTop)/sqrt(2))) joinGroup(position - (aoiRight - aoiTop)/sqrt(2));

        if( scope.contains( position + aoiRight )) leaveGroup( scope.origin + groupRight );
        if( scope.contains( position - aoiRight )) leaveGroup( scope.origin - groupRight );
        if( scope.contains( position + aoiTop )) leaveGroup( scope.origin + groupTop );
        if( scope.contains( position - aoiTop )) leaveGroup( scope.origin - groupTop );
        if( scope.contains( position + (aoiRight + aoiTop)/sqrt(2))) leaveGroup( scope.origin + groupRight + groupTop);
        if( scope.contains( position + (aoiRight - aoiTop)/sqrt(2))) leaveGroup( scope.origin + groupRight - groupTop);
        if( scope.contains( position - (aoiRight + aoiTop)/sqrt(2))) leaveGroup( scope.origin - groupRight + groupTop);
        if( scope.contains( position - (aoiRight - aoiTop)/sqrt(2))) leaveGroup( scope.origin - groupRight - groupTop);
    }
}

void NTree::handleAddMessage( NTreeGroupAddMessage* addMsg )
{
    std::list<NTreeGroup>::iterator it = findGroup( addMsg->getOrigin() );
    if( it == groups.end() ){
        EV << "[NTree::handleAddMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Received add message for unknown group scope=" << addMsg->getOrigin() << "\n";
        return;
    }
    EV << "[NTree::handleAddMessage() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Received add message for group scope=" << addMsg->getOrigin() << "\n"
        << "    Player=" << addMsg->getPlayer().getKey().toString(16) << "\n";
    it->members.insert( addMsg->getPlayer() );
}

void NTree::handleDeleteMessage( NTreeGroupDeleteMessage* deleteMsg )
{
    std::list<NTreeGroup>::iterator it = findGroup( deleteMsg->getOrigin(), deleteMsg->getSize() );
    // Group already deleted
    if( it == groups.end() ) return;

    // Check if I believe to own the group
    // In that case, delete the corresponding node
    ntreeNodes.erase( it->scope );

    // Join new sub-group
    if( it->isInScope(position) ){
        NTreeJoinCall* joinCall = new NTreeJoinCall("JOIN GROUP");
        joinCall->setPosition(position);
        joinCall->setBitLength( NTREEJOINCALL_L(joinCall) );
        RECORD_STATS(
                joinsSend++;
                joinBytes += joinCall->getByteLength();
        );
        sendMessage( deleteMsg->getNewChild( it->scope.origin.getQuadrant( position )), joinCall);
    }

    // delete group
    if( currentGroup == &(*it) ) currentGroup = NULL;
    groups.erase( it );
}

void NTree::handleLeaveMessage( NTreeLeaveMessage* leaveMsg )
{
    std::list<NTreeGroup>::iterator it = findGroup( leaveMsg->getPosition() );
    if( it != groups.end() ){
        it->members.erase( leaveMsg->getPlayer() );
    }

    // If player not otherwise known
    for( it = groups.begin(); it != groups.end(); ++it ){
        if( it->members.find( leaveMsg->getPlayer() ) != it->members.end() ){
            return;
        }
    }

    // Inform app
    GameAPIListMessage* gameMsg = new GameAPIListMessage("NEIGHBOR_DELETE");
    gameMsg->setCommand(NEIGHBOR_UPDATE);
    gameMsg->setRemoveNeighborArraySize(1);
    gameMsg->setRemoveNeighbor(0, leaveMsg->getPlayer() );
    send(gameMsg, "appOut");
}

void NTree::handleCollapseMessage( NTreeCollapseMessage* collapseMsg )
{
    EV << "[NTree::handleCollapseMessage() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Got collapse message for group " << collapseMsg->getOrigin() << "\n";

    std::map<NTreeScope,NTreeNode>::iterator it = findNTreeNode( collapseMsg->getOrigin(), collapseMsg->getSize() );
    if( it != ntreeNodes.end() ){
        if( it->second.group ){
            EV << "    Informing group\n";
            // leaf, inform group
            NTreeGroupDeleteMessage* delMsg = new NTreeGroupDeleteMessage("Delete Group due to collapse");
            delMsg->setOrigin( collapseMsg->getOrigin() );
            delMsg->setSize( collapseMsg->getSize() );
            for( unsigned int i = 0; i < 4; ++i ){
                delMsg->setNewChild( i, collapseMsg->getPlayer() );
            }
            delMsg->setBitLength( NTREEDELETE_L(delMsg) );
            RECORD_STATS(
                    treeMaintenanceMessages += it->second.group->members.size();
                    treeMaintenanceBytes += it->second.group->members.size()*delMsg->getByteLength();
            );
            sendToGroup( *it->second.group, delMsg );
            // delete group
            if( currentGroup && *currentGroup == *it->second.group ){
                currentGroup = NULL;
            }
            groups.remove( *it->second.group );

        } else {
            // node, inform children
            EV << "    Informing children\n";
            for( unsigned int i = 0; i < 4; ++i ){
                EV << "    ==> " << it->second.children[i] << "\n";
                collapseMsg->setOrigin( it->second.scope.getSubScope(i).origin );
                collapseMsg->setSize( it->second.scope.getSubScope(i).size );
                RECORD_STATS(
                        treeMaintenanceMessages++;
                        treeMaintenanceBytes += collapseMsg->getByteLength();
                );
                sendMessage( it->second.children[i], collapseMsg->dup() );
            }
        }
        // delete node
        ntreeNodes.erase( it );
    }
}

void NTree::handleMoveMessage( NTreeMoveMessage* moveMsg )
{
    // Inform app
    GameAPIListMessage* gameMsg = new GameAPIListMessage("NEIGHBOR_UPDATE");
    gameMsg->setCommand(NEIGHBOR_UPDATE);

    gameMsg->setAddNeighborArraySize(1);
    gameMsg->setNeighborPositionArraySize(1);

    gameMsg->setAddNeighbor(0, moveMsg->getPlayer() );
    gameMsg->setNeighborPosition(0, moveMsg->getPosition() );
    send(gameMsg, "appOut");
    RECORD_STATS(
            globalStatistics->addStdDev(
                "NTree: MoveDelay",
                SIMTIME_DBL(simTime()) - SIMTIME_DBL(moveMsg->getCreationTime())
                );
            );
}

void NTree::handlePingCall( NTreePingCall* pingCall )
{
    if( NTreeNodePingCall* nodePing = dynamic_cast<NTreeNodePingCall*>(pingCall) ){
        // node Ping
        // compute my scope
        NTreeScope origScope( pingCall->getOrigin(), pingCall->getSize() );
        NTreeScope myScope;
        myScope = origScope.getSubScope( nodePing->getQuadrant() );

        std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.find( myScope );
        if( it != ntreeNodes.end() ){
            // TODO: save parentParent?

            if( nodePing->getParent().isUnspecified() ) it->second.parentIsRoot = true;
            for( unsigned int i = 0; i < 4; ++i ){
                it->second.siblings[i] = nodePing->getSiblings(i);
            }
            it->second.lastPing = simTime();

            // Answer ping
            NTreeNodePingResponse* nodeResp = new NTreeNodePingResponse("PONG");
            if( it->second.group ){
                // leaf
                nodeResp->setAggChildCount( it->second.group->members.size() );
                nodeResp->setMembersArraySize( it->second.group->members.size() );
                unsigned int i = 0;
                for( std::set<NodeHandle>::iterator childIt = it->second.group->members.begin(); 
                        childIt != it->second.group->members.end(); ++childIt ){

                    nodeResp->setMembers( i++, *childIt );
                }
            } else {
                // ordinary node
                unsigned int aggChildCount = 0;
                nodeResp->setMembersArraySize( 4 );
                for( unsigned int i = 0; i < 4; ++i ){
                    aggChildCount += it->second.aggChildCount[i];
                    nodeResp->setMembers( i, it->second.children[i] );
                }
                nodeResp->setAggChildCount( aggChildCount );
            }
            nodeResp->setBitLength( NTREENODEPINGRESPONSE_L(nodeResp) );
            sendRpcResponse( nodePing, nodeResp );
        } else {
            delete pingCall;
        }
    } else {
        // Group ping
        std::list<NTreeGroup>::iterator it = findGroup( pingCall->getOrigin(), pingCall->getSize() );
        if( it != groups.end() ){
            // TODO: save parentParent

        }
        // Answer ping
        NTreePingResponse* pingResp = new NTreePingResponse("PONG");
        pingResp->setBitLength( NTREEPINGRESPONSE_L(pingResp) );
        sendRpcResponse( pingCall, pingResp );
    }

}

void NTree::handlePingResponse( NTreePingResponse* pingResp, NTreePingContext* context )
{
    if( NTreeNodePingResponse* nodePing = dynamic_cast<NTreeNodePingResponse*>(pingResp) ){
        std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.find( context->nodeScope );
        if( it != ntreeNodes.end() && !it->second.group){
            // If child found, update children info in node
            if( it->second.children[context->quadrant] == nodePing->getSrcNode() ){
                it->second.aggChildCount[context->quadrant] = nodePing->getAggChildCount();
                it->second.childChildren[context->quadrant].clear();
                for( unsigned int ii = 0; ii < nodePing->getMembersArraySize(); ++ii ){
                    it->second.childChildren[context->quadrant].insert( nodePing->getMembers( ii ) );
                }

                // Collapse tree if aggChildCount is too low
                unsigned int aggChildCount = 0;
                for( unsigned int i = 0; i < 4; ++i ){
                    if( !it->second.aggChildCount[i] ) {
                        aggChildCount = maxChildren;
                        break;
                    }

                    aggChildCount += it->second.aggChildCount[i];
                }
                if( aggChildCount*2 < maxChildren ){
                    collapseTree( it );
                }
            } else {
                // child not found
                // TODO: what to do now?
            }
        }
    }
    delete context;
}

void NTree::handlePingCallTimeout( NTreePingCall* pingCall, const TransportAddress& oldNode, NTreePingContext* context )
{
    if( context ){
        std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.find( context->nodeScope );
        if( it != ntreeNodes.end() && !it->second.group){
            // If child found, replace it
            if( oldNode == it->second.children[context->quadrant ]){
                NTreeReplaceNodeMessage* replaceMsg = new NTreeReplaceNodeMessage("Replace failed node");
                replaceMsg->setOrigin( context->nodeScope.getSubScope(context->quadrant).origin );
                replaceMsg->setSize( context->nodeScope.getSubScope(context->quadrant).size );
                replaceMsg->setParent( thisNode );
                replaceMsg->setOldNode( oldNode );
                replaceMsg->setIsLeaf( it->second.childChildren[context->quadrant].size() != 4 ||
                        (it->second.childChildren[context->quadrant].size() == it->second.aggChildCount[context->quadrant]) );
                replaceMsg->setChildrenArraySize( it->second.childChildren[context->quadrant].size() );
                unsigned int i = 0;
                for( std::set<NodeHandle>::iterator cit = it->second.childChildren[context->quadrant].begin();
                        cit != it->second.childChildren[context->quadrant].end(); ++cit ){
                    replaceMsg->setChildren( i, *cit );
                    ++i;
                }
                replaceMsg->setBitLength( NTREEREPLACE_L(replaceMsg) );

                // Find node to replace failed node
                std::set<NodeHandle> invalidNodes;
                invalidNodes.insert( thisNode );
                if( !it->second.parent.isUnspecified() ){
                    invalidNodes.insert( it->second.parent );
                }
                if( !replaceMsg->getIsLeaf() ) {
                    for( unsigned int i = 0; i < 4; ++i ){
                        invalidNodes.insert( replaceMsg->getChildren(i) );
                    }
                }
                const NodeHandle& dest = getRandomNode( invalidNodes );
                RECORD_STATS(
                        treeMaintenanceMessages++;
                        treeMaintenanceBytes += replaceMsg->getByteLength();
                );
                sendMessage(dest, replaceMsg );
            }
        }
        delete context;
    } else {
        // Group ping. Simple delete node from all groups
        for( std::list<NTreeGroup>::iterator it = groups.begin(); it != groups.end(); ++it ){
            // we have to search the group members manually, as we only have a TransportAddress to compare
            for( std::set<NodeHandle>::iterator mit = it->members.begin(); mit != it->members.end(); ++mit ){
                if( oldNode == *mit ){
                    NTreeLeaveMessage* leaveMsg = new NTreeLeaveMessage("Failed member");
                    leaveMsg->setPlayer( *mit );
                    leaveMsg->setPosition( it->scope.origin );
                    leaveMsg->setBitLength( NTREELEAVE_L(leaveMsg) );
                    sendToGroup( *it, leaveMsg );

                    it->members.erase( mit );
                    break;
                }
            }
        }
    }
}

void NTree::handleDivideCall( NTreeDivideCall* divideCall )
{
    EV << "[NTree::handleDivide() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n";
    std::list<NTreeGroup>::iterator it = findGroup( divideCall->getOrigin(), divideCall->getSize() );
    if( it == groups.end() ){
        EV << "    Received divide message for unknown group scope=" << divideCall->getOrigin() << "\n";
    } else {
        EV << "    Divide group " << it->scope << "\n";
    }

    // Calculate new scope
    NTreeScope oldScope(divideCall->getOrigin(), divideCall->getSize() );
    NTreeScope newScope = oldScope.getSubScope( divideCall->getQuadrant() );
    
    // If we own the node that is beeing divided, something went wrong
    // However, ther is nothing we can do about it except removing the offending node
    // Sombody obviously took over without us noticing
    ntreeNodes.erase( oldScope );

    // Create new group, delete old group
    NTreeGroup newGroup(newScope);
    newGroup.leader = thisNode;
    newGroup.members.insert(thisNode);
    groups.push_front(newGroup);
    if( it != groups.end() ){
        if( currentGroup == &(*it) ){
            currentGroup = &groups.front();
        }
        groups.erase( it );
    }

    // Create new ntreeNode
    NTreeNode newNode(newScope);
    newNode.parent = divideCall->getSrcNode();
    newNode.group = &groups.front();
    ntreeNodes.insert(make_pair(newScope,newNode));

    // Acknowledge Divide
    NTreeDivideResponse* divideResp = new NTreeDivideResponse("Divide ACK");
    divideResp->setQuadrant( divideCall->getQuadrant() );
    divideResp->setBitLength( NTREEDIVIDERESPONSE_L(divideResp) );
    RECORD_STATS(
            treeMaintenanceMessages++;
            treeMaintenanceBytes += divideResp->getByteLength();
    );
    sendRpcResponse( divideCall, divideResp );
}

void NTree::handleDivideResponse( NTreeDivideResponse* divideResp, NTreeGroupDivideContext* context )
{
    context->newChild[ divideResp->getQuadrant() ] = divideResp->getSrcNode();
    for( unsigned int i = 0; i < 4; ++i ){
        if( context->newChild[i].isUnspecified() ){
            // Some responses still missing
            return;
        }
    }
    divideNode( context );
    delete context;
}

void NTree::handleDivideCallTimeout( NTreeDivideCall* divideCall, const TransportAddress& oldNode, NTreeGroupDivideContext* context )
{
    std::set<NodeHandle> invalidNodes;
    invalidNodes.insert( thisNode );
    NodeHandle dest = getRandomNode( invalidNodes );
    RECORD_STATS(
            treeMaintenanceMessages++;
            treeMaintenanceBytes += divideCall->getByteLength();
    );
    NTreeGroupDivideContextPtr* contextPtr = new NTreeGroupDivideContextPtr;
    contextPtr->ptr = context;
    sendUdpRpcCall( dest, divideCall->dup(), contextPtr );
}

void NTree::handleReplaceMessage( NTreeReplaceNodeMessage* replaceMsg )
{
    EV << "[NTree::handleReplaceMessage() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Was asked to replace " << replaceMsg->getOldNode() << " for group " << replaceMsg->getOrigin() << "\n";
    // Create new ntreeNode
    NTreeScope newScope( replaceMsg->getOrigin(), replaceMsg->getSize() );
    NTreeNode newNode(newScope);
    newNode.parent = replaceMsg->getParent();

    if( replaceMsg->getIsLeaf() ){
        std::list<NTreeGroup>::iterator it = findGroup( replaceMsg->getOrigin(), replaceMsg->getSize() );
        if( it == groups.end() ){
            NTreeGroup newGrp( newScope );
            // TODO: check for conflicting groups?
            groups.push_front( newGrp );
            it = groups.begin();
        }
        it->leader = thisNode;
        it->members.insert( thisNode );
        for( unsigned int i = 0; i < replaceMsg->getChildrenArraySize(); ++i ){
            it->members.insert( replaceMsg->getChildren(i) );
        }
        newNode.group = &(*it);
    } else {
        for( unsigned int i = 0; i < 4; ++i ){
            newNode.children[i] = replaceMsg->getChildren(i);
        }
    }

    ntreeNodes.insert(make_pair(newScope,newNode));

    // Inform parent+children
    NTreeTakeOverMessage* takeMsg = new NTreeTakeOverMessage("I took over");
    takeMsg->setPlayer( thisNode );
    takeMsg->setOrigin( newScope.origin );
    takeMsg->setSize( newScope.size );
    takeMsg->setBitLength( NTREETAKEOVER_L(takeMsg) );

    sendMessage( newNode.parent, takeMsg->dup() );
    sendMessage( replaceMsg->getOldNode(), takeMsg->dup() ); // also inform old node if is is (against expectations) still alive
    RECORD_STATS(
            treeMaintenanceMessages+=2;
            treeMaintenanceBytes += 2*takeMsg->getByteLength();
    );
    if( newNode.group ){
        RECORD_STATS(
                treeMaintenanceMessages += newNode.group->members.size();
                treeMaintenanceBytes += newNode.group->members.size()*takeMsg->getByteLength();
        );
        sendToGroup( *newNode.group, takeMsg );
    } else {
        for( unsigned int i = 0; i < 4; ++i ){
            RECORD_STATS(
                    treeMaintenanceMessages++;
                    treeMaintenanceBytes += takeMsg->getByteLength();
            );
            sendMessage( newNode.children[i], takeMsg->dup() );
        }
        delete takeMsg;
    }

}

void NTree::handleTakeOverMessage( NTreeTakeOverMessage* takeMsg )
{
    // CHeck if group leader gets replaced
    std::list<NTreeGroup>::iterator git = findGroup( takeMsg->getOrigin(), takeMsg->getSize() );
    if( git != groups.end() ){
        git->leader = takeMsg->getPlayer();
    }

    // Check if a node gets replaced
    NTreeScope takeScope( takeMsg->getOrigin(), takeMsg->getSize() );
    for(std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.begin(); it != ntreeNodes.end(); ++it ){
        // Check if parent is replaced
        if( takeMsg->getSize() == it->second.scope.size*2.0 ){
            for( unsigned int i = 0; i < 4; ++i ){
                if( takeScope.getSubScope(i) == it->first ){
                    it->second.parent = takeMsg->getPlayer();
                }
            }
            // Else check if child is replaced
        } else if( takeMsg->getSize()*2.0 == it->second.scope.size && !it->second.group ){
            for( unsigned int i = 0; i < 4; ++i ){
                if( it->first.getSubScope(i) == takeScope ){
                    it->second.children[i] = takeMsg->getPlayer();
                }
            }
        } else if( it->second.scope == takeScope && takeMsg->getPlayer() != thisNode ){
            // Uh-oh. Somebody replaced me. Yield ownership
            ntreeNodes.erase( it );
            return;
        }
    }

}

void NTree::handleNodeGracefulLeaveNotification()
{
    for(std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.begin(); it != ntreeNodes.end(); ++it ){ 
        // Replace me on all nodes
        NTreeReplaceNodeMessage* replaceMsg = new NTreeReplaceNodeMessage("Replace me");
        replaceMsg->setOrigin( it->second.scope.origin );
        replaceMsg->setSize( it->second.scope.size );
        replaceMsg->setParent( it->second.parent );
        if( it->second.group ){
            replaceMsg->setIsLeaf( true );
            replaceMsg->setChildrenArraySize( it->second.group->members.size() );
            unsigned int i = 0;
            for( std::set<NodeHandle>::iterator mit = it->second.group->members.begin(); mit != it->second.group->members.end(); ++mit ){
                replaceMsg->setChildren(i, *mit );
                ++i;
            }
        } else {
            replaceMsg->setIsLeaf( false );
            replaceMsg->setChildrenArraySize( 4 );
            for( unsigned int i = 0; i < 4; ++i ){
                replaceMsg->setChildren(i, it->second.children[i] );
            }
        }
        replaceMsg->setBitLength( NTREEREPLACE_L(replaceMsg) );

        // Search random node to replace me
        std::set<NodeHandle> invalidNodes;
        invalidNodes.insert( thisNode );
        if( !it->second.parent.isUnspecified() ){
            invalidNodes.insert( it->second.parent );
        }
        if( !it->second.group ) {
            for( unsigned int i = 0; i < 4; ++i ){
                invalidNodes.insert( it->second.children[i] );
            }
        }
        const NodeHandle& dest = getRandomNode( invalidNodes );

        // Inform node of his new responsabilities
        RECORD_STATS(
                treeMaintenanceMessages++;
                treeMaintenanceBytes += replaceMsg->getByteLength();
        );
        sendMessage( dest, replaceMsg );
    }
    while( groups.size() ){
        // Leave all groups
        leaveGroup( groups.begin()->scope.origin, true );
    }
    // clear ntree
    ntreeNodes.clear();

    currentGroup = NULL;
    changeState( SHUTDOWN );
}

void NTree::pingNodes()
{
    for(std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.begin(); it != ntreeNodes.end(); ++it ){

        unsigned int i = 0;
        if( it->second.group ){
            // Leaf
            NTreePingCall* pingCall = new NTreePingCall("PING");
            pingCall->setOrigin( it->second.scope.origin );
            pingCall->setSize( it->second.scope.size );
            pingCall->setParent( it->second.parent );
            pingCall->setBitLength( NTREEPINGCALL_L(pingCall) );
            sendToGroup( *it->second.group, pingCall );
        } else {
            NTreeNodePingCall* pingCall = new NTreeNodePingCall("PING");
            pingCall->setOrigin( it->second.scope.origin );
            pingCall->setSize( it->second.scope.size );
            pingCall->setParent( it->second.parent );

            for( i = 0; i < 4; ++i ){
                pingCall->setSiblings(i, it->second.children[i] );
            }
            pingCall->setBitLength( NTREENODEPINGCALL_L(pingCall) );
            
            for( i = 0; i < 4; ++i ){
                pingCall->setQuadrant( i );
                sendUdpRpcCall( it->second.children[i], pingCall->dup(), new NTreePingContext(it->second.scope, i) );
            }
            delete pingCall;
        }

    }
}

void NTree::checkParentTimeout()
{
    // Go throup all nodes. iterator is incremented in-loop to allow deletion
    for(std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.begin(); it != ntreeNodes.end(); ){
        if( it->second.parent.isUnspecified() || (it->second.lastPing == 0) ) {
            ++it;
            continue;
        }
        if( it->second.lastPing + pingInterval + pingInterval < simTime() ){
            // Parent timed out
            EV << "[NTree::checkParentTimeout() @ " << thisNode.getIp()
                << " (" << thisNode.getKey().toString(16) << ")]\n"
                << "    Parent for node" << it->second << " timed out\n";
            // Replace parent if parent is root. Other parents will be replaced by their parents
            // Also, only sibling[0] does the replacing to avoid conflicts.
            if( it->second.parentIsRoot ) {
                if( it->second.siblings[0] == thisNode ){
                    EV << "    Parent was root. Replacing him\n";
                    NTreeReplaceNodeMessage* replaceMsg = new NTreeReplaceNodeMessage("Replace failed ROOT.");
                    replaceMsg->setOrigin( Vector2D(areaDimension/2.0, areaDimension/2.0) );
                    replaceMsg->setSize( areaDimension );
                    replaceMsg->setIsLeaf( false );
                    replaceMsg->setChildrenArraySize( 4 );
                    replaceMsg->setOldNode( it->second.parent );

                    std::set<NodeHandle> invalidNodes;
                    for( unsigned int i = 0; i < 4; ++i ){
                        replaceMsg->setChildren( i, it->second.siblings[i] );
                        invalidNodes.insert( replaceMsg->getChildren(i) );
                    }
                    replaceMsg->setBitLength( NTREEREPLACE_L(replaceMsg) );

                    const NodeHandle& dest = getRandomNode( invalidNodes );
                    RECORD_STATS(
                            treeMaintenanceMessages++;
                            treeMaintenanceBytes += replaceMsg->getByteLength();
                    );
                    sendMessage(dest, replaceMsg );
                }
                ++it;
            } else {
                // else drop node
                if( it->second.group ){
                    if( currentGroup && *currentGroup == *it->second.group ){
                        currentGroup = NULL;
                    }
                    groups.remove( *it->second.group );
                }
                ntreeNodes.erase( it++ );
            }
        } else {
            ++it;
        }
    }
}

void NTree::divideNode( NTreeGroupDivideContext* context )
{
    EV << "[NTree::divideNode() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Trying to divide node " << context->nodeScope << "\n";
    std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.find( context->nodeScope );
    if( it == ntreeNodes.end() ){
        EV << "    However, node is not in the list of known nodes\n";
        return;
    }
    if( !it->second.group ){
        EV << "    However, node has no known group attached\n";
        return;
    }
    // Inform group members
    NTreeGroupDeleteMessage* delMsg = new NTreeGroupDeleteMessage("Delete Group due to divide");
    delMsg->setOrigin( context->nodeScope.origin );
    delMsg->setSize( context->nodeScope.size );
    delMsg->setBitLength( NTREEDELETE_L(delMsg) );
    for( unsigned int i = 0; i < 4; ++i ){
        delMsg->setNewChild(i, context->newChild[i] );
    }
    RECORD_STATS(
            divideCount ++;
            treeMaintenanceMessages+= it->second.group->members.size();
            treeMaintenanceBytes += it->second.group->members.size()*delMsg->getByteLength();
    );
    sendToGroup( *it->second.group, delMsg );

    // delete group
    if( currentGroup && *currentGroup == *it->second.group ){
        currentGroup = NULL;
    }
    groups.remove( *it->second.group );
    it->second.group= NULL;

    // add children to ntreeNode
    for( unsigned int i = 0; i < 4; ++i ){
        it->second.children[i] = context->newChild[i];
    }
}

void NTree::collapseTree( std::map<NTreeScope,NTreeNode>::iterator node )
{
    EV << "[NTree::CollapseTree() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Collapsing tree" << node->second << "\n";
    // Inform children
    NTreeCollapseMessage* collapseMsg = new NTreeCollapseMessage("Collapse Tree");
    collapseMsg->setPlayer( thisNode );
    collapseMsg->setBitLength( NTREECOLLAPSE_L(collapseMsg) );

    for( unsigned int i = 0; i < 4; ++i ){
        collapseMsg->setOrigin( node->second.scope.getSubScope(i).origin );
        collapseMsg->setSize( node->second.scope.getSubScope(i).size );
        RECORD_STATS(
                treeMaintenanceMessages++;
                treeMaintenanceBytes += collapseMsg->getByteLength();
                collapseCount ++;
        );
        sendMessage( node->second.children[i], collapseMsg->dup() );
        // and update node
        node->second.children[i] = NodeHandle::UNSPECIFIED_NODE;
        node->second.childChildren[i].clear();
        node->second.aggChildCount[i] = 0;
    }
    delete collapseMsg;

    // create group for node
    NTreeGroup newGroup( node->second.scope );
    newGroup.leader = thisNode;
    newGroup.members.insert( thisNode );
    groups.push_front( newGroup );

    node->second.group = &(groups.front());
}

void NTree::joinGroup( Vector2D position )
{
    std::list<NTreeGroup>::iterator grpIt = findGroup( position );
    if( grpIt == groups.end() ) {
        NTreeJoinCall* joinCall = new NTreeJoinCall("JOIN GROUP");
        joinCall->setPosition( position );
        joinCall->setBitLength( NTREEJOINCALL_L(joinCall) );
        RECORD_STATS(
                joinsSend++;
                joinBytes += joinCall->getByteLength();
        );
        routeViaNTree( position, joinCall );
    }
}

void NTree::leaveGroup( Vector2D position, bool force )
{
    std::list<NTreeGroup>::iterator grpIt = findGroup( position );
    // Don't leave group if I'm leader of that group (except if forced)
    if( grpIt != groups.end() && (grpIt->leader != thisNode || force) ) {
        NTreeLeaveMessage* leaveMsg = new NTreeLeaveMessage("Leave Group");
        leaveMsg->setPlayer( thisNode );
        leaveMsg->setPosition( position );
        leaveMsg->setBitLength( NTREELEAVE_L(leaveMsg) );
        sendToGroup( *grpIt, leaveMsg );

        if( currentGroup == &(*grpIt) ){
            EV << "[NTree::leaveGroup() @ " << thisNode.getIp()
                << " (" << thisNode.getKey().toString(16) << ")]\n"
                << "    WARNING: Leaving group " << position << "\n"
                << "    This group is marked as current group!\n";
            currentGroup = NULL;
        }
        groups.erase( grpIt );
    }
}

void NTree::routeViaNTree( const Vector2D& pos, cPacket* msg, bool forward )
{
    NodeHandle dest;
    if( ntreeNodes.size() ) {
        // Check if pos is in scope of one ntree node
        std::map<NTreeScope,NTreeNode>::iterator it = findNTreeNode( pos );
        if( it != ntreeNodes.end() ){
            // Forward message to appropriate child
            dest = it->second.getChildForPos( pos );
        } else {
            // else send to parent of highest ntreeNode
            dest = ntreeNodes.begin()->second.parent;
        }
    } else {
        // No Ntree known. Send to group leader
        if( currentGroup ){
            dest = currentGroup->leader;
        } else if( groups.size() ){
            dest = groups.begin()->leader;
        } else { // No group known. We have to re-join. (Even though a JOIN might still be in progress)
            EV << "[NTree::routeViaNTree() @ " << thisNode.getIp()
                << " (" << thisNode.getKey().toString(16) << ")]\n"
                << "    No ntree node and no group known. Dropping message & re-join!\n";
            changeState( INIT );
            delete msg;
            return;
        }
    }
    if( forward && dest == thisNode ) {
        EV << "[NTree::routeViaNTree() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    Trying to route message " << msg << " to myself\n"
            << "    This will result in a loop. Dropping message\n";
        delete msg;
    } else {
        sendMessage( dest, msg, forward );
    }

}

void NTree::sendToGroup( const NTreeGroup& grp, cPacket* msg, bool keepMsg )
{
    EV << "[NTree::sendToGroup() @ " << thisNode.getIp()
        << " (" << thisNode.getKey().toString(16) << ")]\n"
        << "    Send message " << msg << "\n"
        << "    to group scope=" << grp.scope.origin << "\n";
    NTree::sendToGroup( grp.members, msg, keepMsg );
}

void NTree::sendToGroup( const std::set<NodeHandle>& grp, cPacket* msg, bool keepMsg )
{
    for( std::set<NodeHandle>::iterator it = grp.begin(); it != grp.end(); ++it ){
        sendMessage( *it, msg->dup(), false );
    }
    if (!keepMsg) delete msg;
}


void NTree::sendMessage(const TransportAddress& dest, cPacket* msg, bool forward)
{
    if( dest.isUnspecified() ) {
        EV << "[NTree::sendMessage() @ " << thisNode.getIp()
            << " (" << thisNode.getKey().toString(16) << ")]\n"
            << "    WARNING: Trying to send message " << msg
            << "    to unspecified destination. Dropping packet!\n";
        delete msg;
        return;
    }

    // TODO: statistics?
    BaseCallMessage* rpc = dynamic_cast<BaseCallMessage*>(msg);
    if( rpc && !forward ){
        // Slightly evil: If an OverlayKey is set, the RPC response
        // may get dropped if it was forwarded (because OverSim expects
        // frowarding to be done with BaseRoutMessages. Which we can't do,
        // because we are not routing to overlay keys but to positions.
        // Thus, we cast the OverlayKey away.
        sendUdpRpcCall( TransportAddress(dest), rpc );
    } else {
        sendMessageToUDP( dest, msg );
    }
}

void NTree::changeState( int newState ) {
    switch( newState ){
        case INIT:
            state = INIT;
            if( !joinTimer->isScheduled() ) {
                scheduleAt( ceil(simTime() + (simtime_t) par("joinDelay")), joinTimer );
            }
            break;
        
        case READY:
            state = READY;
            setBootstrapedIcon();
            if( !currentGroup && groups.size() ) currentGroup = &groups.front();
            setOverlayReady( true );
            break;
        
        case JOINING:
            state = JOINING;
            setOverlayReady( false );
            break;
        
        case SHUTDOWN:
            state = SHUTDOWN;
            cancelAndDelete( pingTimer );
            pingTimer = NULL;
            setOverlayReady( false );
            break;
    }
    setBootstrapedIcon();
}

void NTree::setBootstrapedIcon()
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

std::list<NTreeGroup>::iterator NTree::findGroup(const Vector2D& pos)
{
    for( std::list<NTreeGroup>::iterator it = groups.begin(); it != groups.end(); ++it ){
        if( it->isInScope( pos ) ) return it;
    }
    return groups.end();
}

std::list<NTreeGroup>::iterator NTree::findGroup(const Vector2D& pos, double size)
{
    for( std::list<NTreeGroup>::iterator it = groups.begin(); it != groups.end(); ++it ){
        if( it->scope.origin == pos && it->scope.size == size ) return it;
    }
    return groups.end();
}

std::map<NTreeScope,NTreeNode>::iterator NTree::findNTreeNode(const Vector2D& pos)
{
    double size = areaDimension + 1;
    std::map<NTreeScope,NTreeNode>::iterator bestNode = ntreeNodes.end();
    for( std::map<NTreeScope,NTreeNode>::iterator it = ntreeNodes.begin(); it != ntreeNodes.end(); ++it ){
        if( it->second.isInScope( pos ) && it->second.scope.size < size ){
            bestNode = it;
            size = bestNode->second.scope.size;
        }
    }
    return bestNode;
}

std::map<NTreeScope,NTreeNode>::iterator NTree::findNTreeNode(const Vector2D& pos, double size)
{
    return ntreeNodes.find( NTreeScope( pos, size ) );
}

NodeHandle NTree::getRandomNode( std::set<NodeHandle> invalidNodes )
{
    // FIXME: This is cheating...
    bool found = false;
    NodeHandle dest;
    while( !found ){
        dest = globalNodeList->getRandomNode();
        found = true;
        for( std::set<NodeHandle>::iterator it = invalidNodes.begin(); it != invalidNodes.end(); ++it ){
            if( dest == *it ){
                found = false;
                break;
            }
        }
    }
    return dest;
}

void NTree::finishOverlay()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;
    globalStatistics->addStdDev("NTree: Joins send per second", joinsSend / time );
    globalStatistics->addStdDev("NTree: Join bytes send per second", joinBytes / time );
    globalStatistics->addStdDev("NTree: Join timeouts per second", joinTimeout / time );
    globalStatistics->addStdDev("NTree: Tree maintenance messages send per second", treeMaintenanceMessages / time );
    globalStatistics->addStdDev("NTree: Tree maintenance bytes send per second", treeMaintenanceBytes / time );
    globalStatistics->addStdDev("NTree: Move messages send per second", movesSend / time );
    globalStatistics->addStdDev("NTree: Move bytes send per second", moveBytes / time );
    globalStatistics->addStdDev("NTree: Tree collapsed per second", collapseCount / time );
    globalStatistics->addStdDev("NTree: Tree divides per second", divideCount / time );
}

NTree::~NTree()
{
    cancelAndDelete(joinTimer);
    cancelAndDelete(pingTimer);
}

