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
 * @file PubSubLobby.cc
 * @author Stephan Krause
 */

#include "PubSubLobby.h"

#include <GlobalStatistics.h>
#include <GlobalNodeListAccess.h>

using namespace std;

std::ostream& operator<< (std::ostream& o, const PubSubLobby::ChildEntry& entry)
{
    o << "Node: " << entry.handle << " ressources: " << entry.ressources;
    return o;
}

Define_Module(PubSubLobby);

void PubSubLobby::initializeOverlay(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if(stage != MIN_STAGE_OVERLAY) return;

    numSubspaces = par("numSubspaces");
    subspaceSize = (int) ( (unsigned int) par("areaDimension") / numSubspaces);

    // FIXME: Inefficient, make subspace a single dimensioned array
    subspaces.resize( numSubspaces );
    for ( int i = 0; i < numSubspaces; ++i ) {
        for( int ii = 0; ii < numSubspaces; ++ii ) {
            PubSubSubspaceId region( i, ii, numSubspaces );
            subspaces[i].push_back( PubSubSubspaceLobby( region ) );
        }
        WATCH_VECTOR( subspaces[i] );
    }
    thisNode.setKey( OverlayKey::random() );
    GlobalNodeListAccess().get()->registerPeer( thisNode );

    numPubSubSignalingMessages = 0;
    pubSubSignalingMessagesSize = 0;
    WATCH( numPubSubSignalingMessages );
    WATCH( pubSubSignalingMessagesSize );
    WATCH_MAP( playerMap );
}

void PubSubLobby::handleTimerEvent(cMessage* msg)
{
    if( PubSubTimer* timer = dynamic_cast<PubSubTimer*>(msg) ) {
        if( timer->getType() == PUBSUB_TAKEOVER_GRACE_TIME ){
            // Grace period for subspace takeover timed out.
            // If noone claimed the subspace yet, the next respNode query will
            // trigger the selection of a new responsible node
            PubSubSubspaceId subspaceId(timer->getSubspaceId(), numSubspaces );
            subspaces[subspaceId.getX()][subspaceId.getY()].waitingForRespNode = false;
            delete timer;
        }
    }
}

void PubSubLobby::handleUDPMessage(BaseOverlayMessage* msg)
{
    if( PubSubFailedNodeMessage* failMsg = dynamic_cast<PubSubFailedNodeMessage*>(msg) ){
        failedNode( failMsg->getFailedNode() );
        delete msg;

    } else if( PubSubReplacementMessage* repMsg = dynamic_cast<PubSubReplacementMessage*>(msg) ){
        replaceResponsibleNode( repMsg->getSubspaceId(), repMsg->getNewResponsibleNode() );
        delete msg;
    } else if( PubSubHelpReleaseMessage* helpRMsg = dynamic_cast<PubSubHelpReleaseMessage*>(msg) ){
        handleHelpReleaseMessage( helpRMsg );
        delete msg;
    }
}

bool PubSubLobby::handleRpcCall(BaseCallMessage* msg)
{
    // delegate messages
    RPC_SWITCH_START( msg )
    RPC_DELEGATE( PubSubJoin, handleJoin );
    RPC_DELEGATE( PubSubHelp, handleHelpCall );
    RPC_DELEGATE( PubSubResponsibleNode, handleRespCall );
    RPC_SWITCH_END( )

    return RPC_HANDLED;
}

void PubSubLobby::handleRpcResponse(BaseResponseMessage *msg,
                                    cPolymorphic* context,
                                    int rpcId, simtime_t rtt)
{
    RPC_SWITCH_START(msg);
    RPC_ON_RESPONSE( PubSubTakeOverSubspace ) {
        handleTakeOverResponse( _PubSubTakeOverSubspaceResponse );
        break;
    }
    RPC_SWITCH_END( );
}

void PubSubLobby::handleRpcTimeout (BaseCallMessage *msg,
                                    const TransportAddress &dest,
                                    cPolymorphic* context, int rpcId,
                                    const OverlayKey &destKey)
{
    RPC_SWITCH_START(msg)
    RPC_ON_CALL( PubSubTakeOverSubspace ) {
        handleTakeOverTimeout( _PubSubTakeOverSubspaceCall, dest );
        EV << "[PubSubMMOG::handleRpcTimeout() @ " << thisNode.getIp()
           << " (" << thisNode.getKey().toString(16) << ")]\n"
           << "    TakeOverSubspace RPC Call timed out: id=" << rpcId << "\n"
           << "    msg=" << *_PubSubTakeOverSubspaceCall
           << endl;
        break;
    }
    RPC_SWITCH_END( )
}

void PubSubLobby::handleJoin( PubSubJoinCall* joinMsg )
{
    // Insert node in the queue of possible backup nodes
    ChildEntry e;
    e.handle = joinMsg->getSrcNode();
    e.ressources = joinMsg->getRessources();

    pair<PlayerMap::iterator, bool> inserter;
    inserter = playerMap.insert( make_pair( e.handle, e ));
    ChildEntry* childEntry = &(inserter.first->second);
    //pair<PlayerRessourceMap::iterator, bool> rInserter;
    //rInserter = playerRessourceMap.insert( make_pair( e.ressources, childEntry ));
    PlayerRessourceMap::iterator rInserter;
    rInserter = playerRessourceMap.insert( make_pair( e.ressources, childEntry ));
    bool insertedAtBegin = rInserter == playerRessourceMap.begin();

    // send answer with responsible node
    PubSubJoinResponse* joinResp = new PubSubJoinResponse( "Join Response");
    unsigned int x = (unsigned int) (joinMsg->getPosition().x / subspaceSize);
    unsigned int y = (unsigned int) (joinMsg->getPosition().y / subspaceSize);
    PubSubSubspaceLobby& subspace = subspaces[x][y];
    NodeHandle respNode = subspace.getResponsibleNode();
    joinResp->setResponsibleNode( respNode );
    joinResp->setBitLength( PUBSUB_JOINRESPONSE_L( joinResp ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize += joinResp->getByteLength()
            );
    sendRpcResponse( joinMsg, joinResp );

    if( respNode.isUnspecified() && !subspace.waitingForRespNode) {
        // respNode is unknown, create new...
        // TODO: refactor: make a funktion out of this...
        PubSubTakeOverSubspaceCall* toCall = new PubSubTakeOverSubspaceCall( "Take over subspace");
        toCall->setSubspacePos( Vector2D(x, y) );

        ChildEntry* child = playerRessourceMap.begin()->second;
        toCall->setBitLength( PUBSUB_TAKEOVERSUBSPACECALL_L( toCall ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize += toCall->getByteLength()
                );
        sendUdpRpcCall( child->handle, toCall );

        playerRessourceMap.erase( playerRessourceMap.begin() );
        child->dutySet.insert( subspace.getId().getId() );
        child->ressources-=2; // XXX FIXME: make it a parameter...
        if( insertedAtBegin ){
            rInserter = playerRessourceMap.insert( make_pair(child->ressources, child) );
        } else {
            playerRessourceMap.insert( make_pair(child->ressources, child) );
        }

        subspace.waitingForRespNode = true;
    }

    // New node is out of luck: he gets to help all waiting nodes as long as he has ressources left
    if( waitingForHelp.size() > 0 ) {
        std::list<PubSubHelpCall*>::iterator it = waitingForHelp.begin();
        while( it != waitingForHelp.end() ) {
            // Insert subspace into node's dutySet
            if( childEntry->dutySet.insert( (*it)->getSubspaceId() ).second ){
                // If it was not already there (due to duplicate HelpCalls because of retransmissions),
                // decrease ressources
                childEntry->ressources -= ( (*it)->getHelpType() == PUBSUB_BACKUP ) ? 2 : 1; // FIXME: make it a parameter
            }

            PubSubHelpResponse* helpResp = new PubSubHelpResponse("Ask him to help you");
            helpResp->setSubspaceId( (*it)->getSubspaceId() );
            helpResp->setType( (*it)->getType() );
            helpResp->setNode( e.handle );
            helpResp->setBitLength( PUBSUB_HELPRESPONSE_L( helpResp ));
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize += helpResp->getByteLength()
                    );
            sendRpcResponse( *it, helpResp );

            waitingForHelp.erase( it++ );

            if( childEntry->ressources <= 0 ) break; // FIXME: clean up duplicate calls!
        }
        // Fix ressource map entry
        playerRessourceMap.erase( rInserter );
        playerRessourceMap.insert( make_pair(childEntry->ressources, childEntry) );
    }
}

void PubSubLobby::handleHelpCall( PubSubHelpCall* helpMsg )
{
    // A node needs help! Give him the handle of the node with the most ressources...
    const NodeHandle& src = helpMsg->getSrcNode();
    int subspaceId = helpMsg->getSubspaceId();
    PlayerRessourceMap::iterator it;
    for( it = playerRessourceMap.begin(); it != playerRessourceMap.end(); ++it ){
        if( it->second->handle != src &&
               it->second->dutySet.find( subspaceId ) == it->second->dutySet.end() &&
               it->second->ressources > 1 ){
            break;
        }
    }

    // No suitable node found!
    if( it == playerRessourceMap.end() ){
        waitingForHelp.push_back( helpMsg );
        return;
    }

    // decrease ressources
    ChildEntry* child = it->second;
    child->ressources -= ( helpMsg->getHelpType() == PUBSUB_BACKUP ) ? 2 : 1; // FIXME: make it a parameter
    child->dutySet.insert( subspaceId );
    playerRessourceMap.erase( it );
    playerRessourceMap.insert( make_pair(child->ressources, child) );

    // Send handle to requesting node
    PubSubHelpResponse* helpResp = new PubSubHelpResponse("Ask him to help you");
    helpResp->setSubspaceId( subspaceId );
    helpResp->setHelpType( helpMsg->getHelpType() );
    helpResp->setNode( child->handle );
    helpResp->setBitLength( PUBSUB_HELPRESPONSE_L( helpResp ));
    RECORD_STATS(
            ++numPubSubSignalingMessages;
            pubSubSignalingMessagesSize += helpResp->getByteLength()
            );
    sendRpcResponse( helpMsg, helpResp );
}

void PubSubLobby::handleRespCall( PubSubResponsibleNodeCall* respCall )
{
    unsigned int x = (unsigned int) respCall->getSubspacePos().x;
    unsigned int y = (unsigned int) respCall->getSubspacePos().y;
    NodeHandle respNode = subspaces[x][y].getResponsibleNode();
    if( !respNode.isUnspecified() ) {
        PubSubSubspaceId region( x, y, numSubspaces);

        PubSubResponsibleNodeResponse* msg = new PubSubResponsibleNodeResponse( "ResponsibleNode Response");
        msg->setResponsibleNode( respNode );
        msg->setSubspaceId( region.getId() );
        msg->setBitLength( PUBSUB_RESPONSIBLENODERESPONSE_L( msg ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize += msg->getByteLength()
                );
        sendRpcResponse( respCall, msg );
    } else {
        // no responsible node for subspace known.
        // push call to list of waiting nodes ...
        PubSubSubspaceLobby& subspace = subspaces[x][y];
        subspace.waitingNodes.push_back( respCall );

        if (!subspace.waitingForRespNode) {
            // ... and ask a node to take over the subspace
            PubSubTakeOverSubspaceCall* msg = new PubSubTakeOverSubspaceCall( "Take over subspace");
            msg->setSubspacePos( Vector2D( x, y) );

            ChildEntry* child = playerRessourceMap.begin()->second;
            msg->setBitLength( PUBSUB_TAKEOVERSUBSPACECALL_L( msg ));
            RECORD_STATS(
                    ++numPubSubSignalingMessages;
                    pubSubSignalingMessagesSize += msg->getByteLength()
                    );
            sendUdpRpcCall( child->handle, msg );

            playerRessourceMap.erase( playerRessourceMap.begin() );
            child->dutySet.insert( subspace.getId().getId() );
            // Decrease ressources. Note: the ressources are decreased by the cost of an "backup" node
            // The rest will be decreased when the new responsible answeres the takeover call
            child->ressources-=1; // FIXME: make it a parameter...
            playerRessourceMap.insert( make_pair(child->ressources, child) );

            subspace.waitingForRespNode = true;
        }
    }
}

void PubSubLobby::handleTakeOverResponse( PubSubTakeOverSubspaceResponse* takeOverResp )
{
    NodeHandle respNode = takeOverResp->getSrcNode();
    unsigned int x = (unsigned int) takeOverResp->getSubspacePos().x;
    unsigned int y = (unsigned int) takeOverResp->getSubspacePos().y;
    PubSubSubspaceId region( x, y, numSubspaces);

    replaceResponsibleNode( region, takeOverResp->getSrcNode() );
}

void PubSubLobby::handleTakeOverTimeout( PubSubTakeOverSubspaceCall* takeOverCall, const TransportAddress& oldNode )
{
    Vector2D pos = takeOverCall->getSubspacePos();
    subspaces[(int) pos.x][(int) pos.y].waitingForRespNode = false;
    failedNode( oldNode );
}

void PubSubLobby::handleHelpReleaseMessage( PubSubHelpReleaseMessage* helpRMsg )
{
    PlayerMap::iterator playerIt = playerMap.find( helpRMsg->getNode() );
    if( playerIt == playerMap.end() ){
        // Player was already deleted
        return;
    }
    ChildEntry* nodeEntry = &(playerIt->second);

    // remove subspace from node's duty set
    nodeEntry->dutySet.erase( helpRMsg->getSubspaceId() );

    // Increase node's ressources
    pair<PlayerRessourceMap::iterator, PlayerRessourceMap::iterator> resRange;
    PlayerRessourceMap::iterator resIt;
    resRange = playerRessourceMap.equal_range( nodeEntry->ressources );
    for( resIt = resRange.first; resIt != resRange.second; ++resIt ){
        if( resIt->second == nodeEntry ){
            playerRessourceMap.erase( resIt );
            break;
        }
    }
    nodeEntry->ressources += 1; // FIXME: make it a parameter
    playerRessourceMap.insert( make_pair(nodeEntry->ressources, nodeEntry) );
}

void PubSubLobby::replaceResponsibleNode( int subspaceId, NodeHandle respNode )
{
    replaceResponsibleNode( PubSubSubspaceId( subspaceId, numSubspaces), respNode );
}

void PubSubLobby::replaceResponsibleNode( PubSubSubspaceId subspaceId, NodeHandle respNode )
{
    // a new responsible node was found for a subspace
    PubSubSubspaceLobby& subspace = subspaces[subspaceId.getX()][subspaceId.getY()];
//    NodeHandle oldNode = subspace.getResponsibleNode();

    // set new responsible node
    subspace.setResponsibleNode( respNode );
    subspace.waitingForRespNode = false;

    // decrease responsible node's ressources
    pair<PlayerRessourceMap::iterator, PlayerRessourceMap::iterator> resRange;
    PlayerRessourceMap::iterator resIt;
    PlayerMap::iterator plIt = playerMap.find( respNode );

    if( plIt == playerMap.end() ){
        // FIXME: How to react?
        // Best would be: reinsert node. But most probable we have two nodes that want to be
        // responsible, so how to avoid the resulting inconsostency?
        opp_error("PlayerMap inconsistent: Allegedly failed node wants to become Responsible node");
    }
//    ChildEntry* respNodeEntry = &(plIt->second);
//    resRange = playerRessourceMap.equal_range( respNodeEntry->ressources );
//    for( resIt = resRange.first; resIt != resRange.second; ++resIt ){
//        if( resIt->second == respNodeEntry ){
//            playerRessourceMap.erase( resIt );
//            break;
//        }
//    }
//    respNodeEntry->ressources -= 2; // FIXME: make it a parameter
//    playerRessourceMap.insert( make_pair(respNodeEntry->ressources, respNodeEntry) );

    // remove old node from backupList->he failed...
//    failedNode( oldNode );

    // inform all waiting nodes...
    std::list<PubSubResponsibleNodeCall*>::iterator it;
    for( it = subspace.waitingNodes.begin(); it != subspace.waitingNodes.end(); ++it ) {
        PubSubResponsibleNodeResponse* msg = new PubSubResponsibleNodeResponse( "ResponsibleNode Response");
        msg->setResponsibleNode( respNode );
        msg->setSubspaceId( subspaceId.getId() );
        msg->setBitLength( PUBSUB_RESPONSIBLENODERESPONSE_L( msg ));
        RECORD_STATS(
                ++numPubSubSignalingMessages;
                pubSubSignalingMessagesSize += msg->getByteLength()
                );
        sendRpcResponse( *it, msg );
    }
    subspace.waitingNodes.clear();
}

// void PubSubLobby::failedNode(const NodeHandle& node)
void PubSubLobby::failedNode(const TransportAddress& node)
{
    if( node.isUnspecified() ) return;

    // Find node in playerMap
    PlayerMap::iterator playerIt = playerMap.find( node );
    if( playerIt == playerMap.end() ){
        // Player was already deleted
        return;
    }
    ChildEntry* respNodeEntry = &(playerIt->second);

// FIXME: only for debugging
if( GlobalNodeListAccess().get()->getPeerInfo( node ) ){
    opp_error("Trying to delete node that's still there...");
}

    // check if node was responsible for a subspace
    set<int>::iterator dutyIt;
    for( dutyIt = respNodeEntry->dutySet.begin(); dutyIt != respNodeEntry->dutySet.end(); ++dutyIt ){
        PubSubSubspaceId subspaceId( *dutyIt, numSubspaces );
        PubSubSubspaceLobby& subspace = subspaces[subspaceId.getX()][subspaceId.getY()];
        if( !subspace.getResponsibleNode().isUnspecified() && node == subspace.getResponsibleNode() ){
            // remove old responsible node
            subspace.setResponsibleNode(NodeHandle());

            // wait for the backup node to claim subspace; if timer expires, waiting-flag will be reset
            subspace.waitingForRespNode = true;
            PubSubTimer* graceTimer = new PubSubTimer("Grace timer for claiming subspace");
            graceTimer->setType( PUBSUB_TAKEOVER_GRACE_TIME );
            graceTimer->setSubspaceId( subspace.getId().getId() );
            scheduleAt( simTime() + 5, graceTimer ); //FIXME: make it a parameter
        }
    }

   // delete node from backupList
    pair<PlayerRessourceMap::iterator, PlayerRessourceMap::iterator> resRange;
    PlayerRessourceMap::iterator resIt;

    resRange = playerRessourceMap.equal_range( respNodeEntry->ressources );
    for( resIt = resRange.first; resIt != resRange.second; ++resIt ){
        if( resIt->second == respNodeEntry ){
            playerRessourceMap.erase( resIt );
            break;
        }
    }
    playerMap.erase( playerIt );
}

void PubSubLobby::finishOverlay()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if (time < GlobalStatistics::MIN_MEASURED) return;

    globalStatistics->addStdDev("PubSubLobby: Sent Signaling Messages/s",
                                numPubSubSignalingMessages / time);
    globalStatistics->addStdDev("PubSubLobby: Sent Signaling bytes/s",
                                pubSubSignalingMessagesSize / time);
}

PubSubLobby::~PubSubLobby()
{
}
