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
 * @file Scribe.h
 * @author Stephan Krause
 */


#ifndef __SCRIBE_H_
#define __SCRIBE_H_

#include <map>
#include <string>

#include "BaseApp.h"
#include "CommonMessages_m.h"
#include "GlobalNodeList.h"
#include "NodeHandle.h"

#include "ScribeGroup.h"
#include "ScribeMessage_m.h"

// Output function for grouplist, needed for WATCH
std::ostream& operator<< (std::ostream& o, std::map<OverlayKey, ScribeGroup> m )
{
    for (std::map<OverlayKey, ScribeGroup>::iterator it = m.begin(); it != m.end(); ++it) {
        o << it->first << "\n";
        o << "  Parent: " << it->second.getParent() << "\n";
        o << "  Status: " << (it->second.getSubscription() ? "Subscriber\n" : "Forwarder\n");
        o << "  Children (" << it->second.numChildren() << "):\n";
        std::set<NodeHandle>::iterator iit = it->second.getChildrenBegin();
        for (int i = it->second.numChildren(); i > 0; --i) {
            o << "    " << *iit << "\n";
            ++iit;
        }
    }
    return o;
}


class Scribe : public BaseApp
{
    private:
        typedef std::map<OverlayKey, ScribeGroup> GroupList;
        GroupList groupList;
        typedef std::multimap<NodeHandle, ScribeTimer*> ChildTimeoutList;
        ChildTimeoutList childTimeoutList;

        int childTimeout;
        int parentTimeout;

        ScribeTimer* subscriptionTimer;

        // statistics
        int numJoins;
        int numChildTimeout;
        int numParentTimeout;
        int numForward;
        int forwardBytes;
        int numReceived;
        int receivedBytes;
        int numHeartbeat;
        int heartbeatBytes;
        int numSubscriptionRefresh;
        int subscriptionRefreshBytes;

    public:
        Scribe( );
        ~Scribe( );

        // see BaseOverlay.h
        virtual void initializeApp( int stage );

        virtual void handleUpperMessage( cMessage *msg );
        virtual void handleReadyMessage( CompReadyMessage* msg );

        virtual void handleTimerEvent( cMessage *msg );

        virtual bool handleRpcCall( BaseCallMessage* msg );
        virtual void handleRpcResponse( BaseResponseMessage* msg,
                                        cPolymorphic* context,
                                        int rpcId, simtime_t rtt );

        virtual void forward(OverlayKey* key, cPacket** msg,
                NodeHandle* nextHopNode);

        virtual void deliver(OverlayKey& key, cMessage* msg);
        virtual void update( const NodeHandle& node, bool joined );

        virtual void finishApp( );

    protected:
        /**
         * Handles a response to a join call send by this node
         */
        void handleJoinResponse( ScribeJoinResponse* joinResponse );

        /**
         * Handles a join request from another node
         *
         * This method only gets called if the local node is the root of the
         * multicast group. It only calls handlePublishCall with amIRoot
         * parameter set to "true"
        */
        void handleJoinCall( ScribeJoinCall* joinMsg);

        /**
         * Handles a publish call from another node
         *
         * Publish calls are used to send multicast messages to the root
         * of the multicast tree.
         */
        void handlePublishCall( ScribePublishCall* publishCall );

        /**
         * Handles a response to a publish call send b this node
         *
         * Publish calls are used to send multicast messages to the root
         * of the multicast tree.
         */
        void handlePublishResponse( ScribePublishResponse* publishResponse );

        /**
         * Handles join requests from other nodes
         */
        void handleJoinMessage( ScribeJoinCall* joinMsg, bool amIRoot);

        /**
         * Handles leave requests from other nodes
         */
        void handleLeaveMessage( ScribeLeaveMessage* leaveMsg );

        /**
         * Gets called if the local node wants to subscribe to a multicast group
         *
         * @param groupId the ID of the group to join
         */
        void subscribeToGroup( const OverlayKey& groupId );

        /**
         * Gets called if the local node wants to leave a multicast group
         *
         * @param group the ID of the group to leave
         */
        void leaveGroup( const OverlayKey& group );

        /**
         * Starts a local timer
         *
         * This method automaticly determines the type of the timer
         * and schedules it accordingly. If the timer is already scheduled,
         * it gets canceled before getting rescheduled.
         */
        void startTimer( ScribeTimer* timer );

        /**
         * Adds a child to a multicast group
         */
        void addChildToGroup( const NodeHandle& child, ScribeGroup& group );

        /**
         * Removes a child from a multicast group
         */
        void removeChildFromGroup( const NodeHandle& child, ScribeGroup& group );

        /**
         * Removes a child from a multicast group
         *
         * Both the child and the group are determined from the timer message
         * This method gets calld if a subscription timer of a child expires.
         */
        void removeChildFromGroup( ScribeTimer* timer );

        /**
         * Chechs wheter there are any subscibers left to a given root
         */
        void checkGroupEmpty( ScribeGroup& group );

        /**
         * Refreshes a child timer
         *
         * If a child sends a subscribtion refresh, this method gets called.
         * It finds the subscriptionTimeout timer for the group and reschedules it.
         */
        void refreshChildTimer( NodeHandle& child, OverlayKey& groupId );

        /**
         * Delivers a multicast message to all children in the multicast group
         */
        void deliverALMDataToGroup( ScribeDataMessage* dataMsg );

        /**
         * Delivers a multicast message to the tree's root
         *
         * This method gets called when the local app wants to publish some data
         * to the multiacst group.
         */
        void deliverALMDataToRoot( ALMMulticastMessage* mcastMsg );
};

#endif
