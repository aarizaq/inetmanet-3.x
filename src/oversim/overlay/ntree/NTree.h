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
 * @file NTree.h
 * @author Stephan Krause
 */

#ifndef __NTREE_H_
#define __NTREE_H_

#include <omnetpp.h>
#include <NodeHandle.h>
#include <BaseOverlay.h>
#include "NTreeHelper.h"
#include "NTree_m.h"

/**
 * A p2p protocol for MMOGs and virtual worlds.
 * Provides the GameAPI interface
 * Based upon:
 * GauthierDickey, C., Lo, V., and Zappala, D. 2005. "Using n-trees for scalable
 * event ordering in peer-to-peer games". In Proceedings of the international
 * Workshop on Network and Operating Systems Support For Digital Audio and
 * Video  (Stevenson, Washington, USA, June 13 - 14, 2005). NOSSDAV '05. ACM,
 * New York, NY, 87-92. DOI= http://doi.acm.org/10.1145/1065983.1066005 
 *
 * @author STephan Krause
 */
class NTree : public BaseOverlay
{
    public:
        // OMNeT++
        virtual ~NTree();
        virtual void initializeOverlay(int stage);
        virtual void finishOverlay();
        virtual void handleUDPMessage(BaseOverlayMessage* msg);
        virtual void handleTimerEvent(cMessage* msg);
        virtual void handleAppMessage(cMessage* msg);
        virtual bool handleRpcCall(BaseCallMessage* msg);
        virtual void handleRpcResponse(BaseResponseMessage *msg,
                                       cPolymorphic* context,
                                       int rpcId, simtime_t rtt);
        virtual void handleRpcTimeout(BaseCallMessage *msg,
                                      const TransportAddress & dest,
                                      cPolymorphic* context,
                                      int rpcId, const OverlayKey &destKey);

    protected:

        void setBootstrapedIcon();
        /**
         * Handles a move message from the application
         *
         * @param posMsg the move message
         */
        void handleMove( GameAPIPositionMessage* posMsg );
        /**
         * Handles a move message from the network
         *
         * @param moveMsg the move message
         */
        void handleMoveMessage( NTreeMoveMessage* moveMsg );
        /**
         * Handles request to join a group
         *
         * @param joinCall the request
         */
        void handleJoinCall( NTreeJoinCall* joinCall );
        /**
         * Handles the acknowledgement to join a group
         *
         * @param joinResp the acknowledgement (also holds information about the group)
         */
        void handleJoinResponse( NTreeJoinResponse* joinResp );
        /**
         * Gets called if a join call times out
         * Well try to re-join if needed
         *
         * @param joinCall the original call
         * @param oldNode the host the original call was send to
         */
        void handleJoinCallTimeout( NTreeJoinCall* joinCall, const TransportAddress& oldNode );
        /**
         * Handles a ping
         * If the ping was send to a node in the n-tree, it is answered with information about
         * the satte of that n-tree node (children, aggregated size)
         *
         * @param pingCall the ping request
         */
        void handlePingCall( NTreePingCall* pingCall );
        /**
         * Handles a ping response
         *
         * @param pingResp the ping response
         * @param context a context which allows to correlate the response to a gertain node in the n-tree
         */
        void handlePingResponse( NTreePingResponse* pingResp, NTreePingContext* context );
        /**
         * Handles a ping timeout
         * Inform others about the failed node, replace it if needed
         *
         * @param pingCall the original ping call
         * @param oldNode the (failed) host the ping was send to
         * @param context a context which allows to correlate the response to a gertain node in the n-tree
         */
        void handlePingCallTimeout( NTreePingCall* pingCall, const TransportAddress& oldNode, NTreePingContext* context );
        /**
         * Handles a request to divide a group
         *
         * @param divideCall the request
         */
        void handleDivideCall( NTreeDivideCall* divideCall );
        /**
         * Handles an acknowledgement that a group was divided as requested
         *
         * @param divideResp the acknowledgement
         * @param context a context pointer to correlate answers to the group that is to be divided
         */
        void handleDivideResponse( NTreeDivideResponse* divideResp, NTreeGroupDivideContext* context );
        /**
         * Gets called if a divide call times out
         * Re-send the request to a different node
         *
         * @param divideCall the original request
         * @param oldNode the  host the original request was send to
         * @param context a context pointer to correlate answers to the group that is to be divided
         */
        void handleDivideCallTimeout( NTreeDivideCall* divideCall, const TransportAddress& oldNode, NTreeGroupDivideContext* context );
        /**
         * Handles an information message that somebody joined a group we are a member of
         *
         * @param addMsg the information message
         */
        void handleAddMessage( NTreeGroupAddMessage* addMsg );
        /**
         * Handles an information message that  a group we are a member of is going to be deleted
         *
         * @param deleteMsg the information message
         */
        void handleDeleteMessage( NTreeGroupDeleteMessage* deleteMsg );
        /**
         * Handles an information message that somebody left a group we are a member of
         *
         * @param leaveMsg the information message
         */
        void handleLeaveMessage( NTreeLeaveMessage* leaveMsg );
        /**
         * Handles an information message that a subtree we are part of is collapsed
         * 
         * @param collapseMsg the information message
         */
        void handleCollapseMessage( NTreeCollapseMessage* collapseMsg );
        /**
         * Handles a request to replace a failed or leaving node
         *
         * @param replaceMsg the replacement request
         */
        void handleReplaceMessage( NTreeReplaceNodeMessage* replaceMsg );
        /**
         * Handles an information message that a node took over the responsibilities of a failed or leaving node
         *
         * @param takeMsg the information message
         */
        void handleTakeOverMessage( NTreeTakeOverMessage* takeMsg );

        void handleNodeGracefulLeaveNotification();

        /**
         * Divide a group that has gotten to large into four subgroups
         *
         * @param context A context pointer to coordinate the four new children
         */

        void divideNode( NTreeGroupDivideContext* context );
        /**
         * Collapse a subtree that has too few children
         *
         * @param node the new leaf of the subtree
         */

        void collapseTree( std::map<NTreeScope,NTreeNode>::iterator node );
        /**
         * Joins the group that is responsible for a given position
         *
         * @param position the position
         */
        void joinGroup( Vector2D position );

        /**
         * Leaves a group for a given position
         *
         * @param position the position
         * @param force if set to true, leave group even if I'm leader of that group
         */
        void leaveGroup( Vector2D position, bool force = false);

        /**
         * Ping all children of all groups
         */
        void pingNodes();

        /**
         * Check if a parent failed to send a ping for a longer time
         */
        void checkParentTimeout();

        /**
         * Route a message through the N-Tree to the closest know node to the given position
         *
         * @param pos the position
         * @param msg the message
         * @param forward Should be set to true if the message was not created by the local node
         */
        void routeViaNTree( const Vector2D& pos, cPacket* msg, bool forward = false);

        /**
         * Sends a message to all members of a group
         *
         * @param grp the group
         * @param msg the message
         * @param keepMsg do not free the memory occupied by msg after sending
         */
        void sendToGroup( const NTreeGroup& grp, cPacket* msg, bool keepMsg = false);

        /**
         * Sends a message to all members of a group
         *
         * @param grp the group
         * @param msg the message
         * @param keepMsg do not free the memory occupied by msg after sending
         */
        void sendToGroup( const std::set<NodeHandle>& grp, cPacket* msg, bool keepMsg = false);

        /**
         * Sends a message to a host. Silently discards the message if the destination is unspecified
         *
         * @param dest the destination address
         * @param msg the message
         * @param forward should be set to true if the message was not created by the local node
         */
        void sendMessage(const TransportAddress& dest, cPacket* msg, bool forward = false);

        /**
         * Change the internal state of the protocol
         *
         * @param state the state (see enum States in BaseOverlay.h)
         */
        void changeState( int state );

        unsigned int AOIWidth;
        unsigned int maxChildren;
        double areaDimension;
        Vector2D position;
        NTreeGroup* currentGroup;

        simtime_t pingInterval;

        cMessage* joinTimer;
        cMessage* pingTimer;

        std::list<NTreeGroup> groups;
        std::map<NTreeScope,NTreeNode> ntreeNodes;

        /**
         * Find a group that is responsible for the given position from the local list of groups
         *
         * @param pos the position
         */
        std::list<NTreeGroup>::iterator findGroup(const Vector2D& pos);
        /**
         * Find a group that is responsible for the given position from the local list of groups
         * Only return a group if it maches the exact scope given
         *
         * @param pos the origin of the group
         * @param size the size of the group
         */
        std::list<NTreeGroup>::iterator findGroup(const Vector2D& pos, double size);
        /**
         * Find the lowest ntree node that contains the given position
         *
         * @param pos the position
         */
        std::map<NTreeScope,NTreeNode>::iterator findNTreeNode(const Vector2D& pos);
        /**
         * Find the group that matches the given scope
         *
         * @param pos the origin of the node
         * @param size the size of the scope
         */
        std::map<NTreeScope,NTreeNode>::iterator findNTreeNode(const Vector2D& pos, double size);

        /**
         * Return a random node
         *
         * @param invalidNodes a list of nodes that should not be returned
         */
        NodeHandle getRandomNode( std::set<NodeHandle> invalidNodes = std::set<NodeHandle>() );

        // statistics
        unsigned int joinsSend;
        unsigned int joinBytes;
        unsigned int joinTimeout;

        unsigned int divideCount;
        unsigned int collapseCount;

        unsigned int treeMaintenanceMessages;
        unsigned int treeMaintenanceBytes;

        unsigned int movesSend;
        unsigned int moveBytes;
};

#endif
