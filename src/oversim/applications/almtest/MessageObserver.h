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
 * @file MessageObserver.h
 * @author Dimitar Toshev
 */

#ifndef __MESSAGEOBSERVER_H__
#define __MESSAGEOBSERVER_H__

#include <stdint.h>
#include <time.h>
#include <ostream>
#include <omnetpp.h>
#include "OverlayKey.h"

class ALMTestTracedMessage;

class MessageObserver : public cSimpleModule {

    public:

        MessageObserver();
        ~MessageObserver();

        void initialize();

        void finish();

        void handleMessage(cMessage* msg);

        /**
         * Adds one to node count for group.
         */
        void joinedGroup(int moduleId, OverlayKey groupId);

        /**
         * Subtracts one from node count for group.
         */
        void leftGroup(int moduleId, OverlayKey groupId);

        /**
         * Counts n - 1 messages pending reception, where n is the
         * size of the group to which the message is sent.
         */
        void sentMessage(ALMTestTracedMessage* msg);

        /**
         * Counts one received message for group.
         */
        void receivedMessage(ALMTestTracedMessage* msg);

        /**
         * Notifies the observer that the node doesn't exist anymore.
         */
        void nodeDead(int moduleId);

    private:

        /*
         * Tracks data related to a single group
         */
        struct MulticastGroup {
            MulticastGroup() : size(0), sent(0), received(0) {}

            // Number of nodes in the group
            uint32_t size;

            // Number of messages that should have been received
            uint64_t sent;

            // Number of messages recieved total by all nodes
            uint64_t received;

        };

        simtime_t creationTime;

        typedef std::pair<int, OverlayKey> NodeGroupPair;

        typedef std::pair<int, long> NodeMessagePair;

        // Info about a specific group
        std::map<OverlayKey, MulticastGroup> groups;

        // When a node joined a given group
        std::map<NodeGroupPair, simtime_t> joinedAt;

        // When a node received a given message
        std::map<NodeMessagePair, simtime_t> receivedAt;

        // Periodically clean up the above map. Set to 0 to disable.
        cMessage* gcTimer;

        // How often to clean up
        double gcInterval;

        // How long data will be kept in the received cache
        double cacheMaxAge;

        // How many messages have been received by their sender (have looped)
        int numLooped;

        GlobalStatistics* globalStatistics;

        friend std::ostream& operator<< (std::ostream& os, MessageObserver::MulticastGroup const & mg);
        friend std::ostream& operator<< (std::ostream& os, MessageObserver::NodeGroupPair const & ngp);
};

std::ostream& operator<< (std::ostream& os, MessageObserver::MulticastGroup const & mg);
std::ostream& operator<< (std::ostream& os, MessageObserver::NodeGroupPair const & ngp);

#endif
