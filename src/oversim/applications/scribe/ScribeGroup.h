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
 * @file ScribeGroup.h
 * @author Stephan Krause
 */


#ifndef __SCRIBEGROUP_H_
#define __SCRIBEGROUP_H_

#include <set>
#include "OverlayKey.h"
#include "NodeHandle.h"

class ScribeGroup;

#include "ScribeMessage_m.h"

/**
 * Capsulates the informations of a scribe multicast group
 */

class ScribeGroup
{
    private:
        OverlayKey groupId;
        NodeHandle rendezvousPoint;
        NodeHandle parent;
        std::set<NodeHandle> children;
        bool subscription;
        bool amISource;

        ScribeTimer* parentTimer;
        ScribeTimer* heartbeatTimer;

    public:
        /**
         * Creates a new ScribeGroup
         *
         * @param id The group ID of the new group
         */
        ScribeGroup( OverlayKey id );
        ~ScribeGroup( );

        /**
         * Adds a new child to the multicast tree
         *
         * @param node The nodeHandle of the child
         * @return An iterator to the inserted child and a boolean value (true = new child, false = child was already present)
         */
        std::pair<std::set<NodeHandle>::iterator, bool> addChild( const NodeHandle& node );

        /**
         * Removes a child from the multicast tree
         *
         * @param node The nodeHandle of the child
         */
        void removeChild( const NodeHandle& node );

        /**
         * Returns an iterator to the begin of the children list
         *
         * @return the iterator
         */
        std::set<NodeHandle>::iterator getChildrenBegin();

        /**
         * Returns an iterator to the end of the children list
         *
         * @return the iterator
         */
        std::set<NodeHandle>::iterator getChildrenEnd();

        /**
         * Get the number of children
         *
         * @return The number of children
         */
        int numChildren() const { return children.size(); }

        /**
         * Return whether the node is forwarder for a group.
         *
         * @return True if there are any children, false else.
         */
        bool isForwarder() const;

        /**
         * Returns the amISource status
         *
         * This status indicates if the node is a source of the multicastgroup
         * FIXME: currently the flag is only be set to true if the node is not
         * also a member (i.e. subscriber)  of the group
         *
         * @return True if the node is sending in multicast messages to the group
         */
        bool getAmISource() const { return amISource; }

        /**
         * Set the amISource status
         *
         * This status indicates if the node is a source of the multicastgroup
         * FIXME: currently the flag is only be set to true if the node is not
         * also a member (i.e. subscriber)  of the group
         *
         * @param source True if the node is sending in multicast messages to the group
         */
        void setAmISource( bool source ) { amISource = source; }

        /**
         * Returns whether the local node is subscriber of the group
         *
         * @return True if the local node is interested in multicast messages for the group, false else
         */
        bool getSubscription() const { return subscription; }

        /**
         * Set the subscription status
         *
         * @param subscribe True if the node is interested in multicast messages for the group, false else
         */
        void setSubscription( bool subscribe ) { subscription = subscribe; }

        /**
         * Return the parent in the multicast tree
         *
         * @return The parent. thisNode if the node is root of the tree
         */
        NodeHandle getParent() const { return parent; }

        /**
         * Sets a new parent for the multicast tree
         *
         * @param _parent The new Parent. Set to thisNode if node should be root of the tree
         */
        void setParent( NodeHandle& _parent ) { parent = _parent; }

        /**
         * Returns the rendevouzPoint (root) of the multicast tree for the group
         *
         * @return The root of the multicast tree if known. UNSPECIFIED_NODE else
         */
        NodeHandle getRendezvousPoint() const { return rendezvousPoint; }

        /**
         * Sets the rendevouzPoint (root) of the multicast tree for the group
         *
         * @param _rendezvousPoint The root of the tree
         */
        void setRendezvousPoint( const NodeHandle& _rendezvousPoint ) { rendezvousPoint = _rendezvousPoint; }

        /**
         * Returns the groupId of the group
         *
         * @return The group ID
         */
        OverlayKey getGroupId() const { return groupId; }

        /**
         * Returns the parent timer.
         *
         * The parent timer is supposed to expire if the parent
         * fails to send heartbeat messages.
         *
         * @return The parentTimer
         */
        ScribeTimer* getParentTimer() { return parentTimer; }

        /**
         * Sets the parent timer.
         *
         * The parent timer is supposed to expire if the parent
         * fails to send heartbeat messages.
         *
         * @param t The parentTimer
         */
        void setParentTimer(ScribeTimer* t ) { parentTimer = t; }

        /**
         * Returns the heartbeat timer.
         *
         * If the timer expires, the node is supposed to send
         * heartbeat messages to all children.
         *
         * @return The heratbetTimer
         */
        ScribeTimer* getHeartbeatTimer() { return heartbeatTimer; }

        /**
         * Sets the heartbeat timer.
         *
         * If the timer expires, the node is supposed to send
         * heartbeat messages to all children.
         *
         * @param t The heartbeatTimer
         */
        void setHeartbeatTimer(ScribeTimer* t ) { heartbeatTimer = t; }

        /**
         * Checks whether the group has a certain groupId
         *
         * @param id The groupId to check
         * @return True if id == groupId, false else
         */
        bool operator== (const OverlayKey& id) const { return id == groupId; };

        /**
         * Checks whether two groups have the same ID
         *
         * @param a The group to compare
         * @return True if the groups have the same ID, false else.
         */
        bool operator== (const ScribeGroup& a) const { return a.getGroupId() == groupId; };

        /**
         * Checks whether the group has a smaller ID than the given key
         *
         * @param id The key to compare
         * @return True if the group id is smaller than the key, false else.
         */
        bool operator< (const OverlayKey& id) const { return id < groupId; };

        /**
         * Checks whether the group has a smaller ID than another group
         *
         * @param a the group to compare
         * @return True if the (local) group id is smaller than the a's groupId, false else.
         */
        bool operator< (const ScribeGroup& a) const { return groupId < a.getGroupId(); };
};

#endif
