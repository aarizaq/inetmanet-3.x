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
 * @file MessageObserver.cc
 * @author Dimitar Toshev
 */

#include <assert.h>
#include <sstream>
#include <omnetpp.h>
#include <GlobalStatisticsAccess.h>
#include "MessageObserver.h"
#include "ALMTestTracedMessage_m.h"

Define_Module(MessageObserver);

MessageObserver::MessageObserver() {
    gcTimer = new cMessage("garbage_collection");
    gcInterval = 1.0;
    cacheMaxAge = 10.0;
    numLooped = 0;
}

MessageObserver::~MessageObserver() {
    cancelAndDelete(gcTimer);
}

void MessageObserver::initialize() {
    WATCH_MAP(groups);
    WATCH_MAP(joinedAt);
    WATCH_MAP(receivedAt);
    WATCH(numLooped);

    numLooped = 0;
    gcInterval = par("gcInterval");
    cacheMaxAge = par("cacheMaxAge");

    if (gcInterval > 0.0)
        scheduleAt(OPP::simTime() + gcInterval, gcTimer);

    creationTime = OPP::simTime();
    globalStatistics = GlobalStatisticsAccess().get();
}

void MessageObserver::finish() {
    uint64_t totalSent = 0;
    uint64_t totalReceived = 0;
    for (std::map<OverlayKey, MulticastGroup>::iterator i = groups.begin(); i != groups.end(); ++i) {
        std::stringstream message;
        message << "MessageObserver: Group " << i->first;
        std::string name;

        name = message.str() + " Sent Messages";
        recordScalar(name.c_str(), (double)i->second.sent);

        name = message.str() + " Received Messages";
        recordScalar(name.c_str(), (double)i->second.received);

        name = message.str() + " Delivered Percentage";
        recordScalar(name.c_str(), ((double)i->second.received * 100.0) / (double)i->second.sent);

        totalSent += i->second.sent;
        totalReceived += i->second.received;
    }

    recordScalar("MessageObserver: Total Sent Messages", (double)totalSent);
    recordScalar("MessageObserver: Total Received Messages", (double)totalReceived);
    recordScalar("MessageObserver: Total Delivered Percentage", ((double)totalReceived * 100.0) / (double)totalSent);
    
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);
    if ( time >= GlobalStatistics::MIN_MEASURED ) {
        globalStatistics->addStdDev("MessageObserver: Looped messages/s", (double)numLooped / time);
    }
}

void MessageObserver::handleMessage(cMessage* msg) {
    if (msg == gcTimer) {
        simtime_t now = OPP::simTime();
        std::map<NodeMessagePair, simtime_t>::iterator i, iPrev;
        i = receivedAt.begin();
        while (i != receivedAt.end()) {
            if (now - i->second >= cacheMaxAge) {
                iPrev = i;
                ++i;
                receivedAt.erase(iPrev);
            }
            else {
                ++i;
            }
        }
        scheduleAt(OPP::simTime() + gcInterval, gcTimer);
    }
}

/**
 * Adds one to node count for group.
 */
void MessageObserver::joinedGroup(int moduleId, OverlayKey groupId) {
    groups[groupId].size += 1;
    joinedAt[NodeGroupPair(moduleId, groupId)] = OPP::simTime();
}

/**
 * Subtracts one from node count for group.
 */
void MessageObserver::leftGroup(int moduleId, OverlayKey groupId) {
    std::map<OverlayKey, MulticastGroup>::iterator iter = groups.find(groupId);
    if (iter == groups.end()) {
        EV << "Warning: MessageObserver asked to remove node from nonexistent group " << groupId.toString();
    }
    else if (iter->second.size == 0) {
        EV << "Warning: MessageObserver asked to remove node from empty group " << groupId.toString();
    }
    else {
        iter->second.size -= 1;
    }
    joinedAt.erase(NodeGroupPair(moduleId, groupId));
}

/**
 * Counts n - 1 messages pending reception, where n is the
 * size of the group.
 */
void MessageObserver::sentMessage(ALMTestTracedMessage* msg) {
    if (msg == NULL) {
        error("%s called with null message.", __PRETTY_FUNCTION__);
    }

    std::map<OverlayKey, MulticastGroup>::iterator iter;
    iter = groups.find(msg->getGroupId());
    if (iter == groups.end()) {
        EV << "Warning: MessageObserver notified of sent message for nonexistent group " << msg->getGroupId().toString();
    }
    else if (iter->second.size == 0) {
        EV << "Warning: MessageObserver notified of sent message for empty group " << msg->getGroupId().toString();
    }
    else {
        iter->second.sent += iter->second.size - 1;
    }
}

/**
 * Counts one received message for group
 */
void MessageObserver::receivedMessage(ALMTestTracedMessage* msg) {
    if (msg == NULL) {
        error("%s called with null message.", __PRETTY_FUNCTION__);
    }

    std::map<OverlayKey, MulticastGroup>::iterator iGroup;
    iGroup = groups.find(msg->getGroupId());
    if (iGroup == groups.end()) {
        EV << "Warning: MessageObserver notified of received message for nonexistent group " << msg->getGroupId().toString();
    }
    else if (iGroup->second.size == 0) {
        EV << "Warning: MessageObserver notified of received message for empty group " << msg->getGroupId().toString();
    }
    else if (msg->getSenderId() != msg->getReceiverId()) {

        // Only count if message was received after joining
        std::map<NodeGroupPair, simtime_t>::iterator iJoinInfo;
        iJoinInfo = joinedAt.find(NodeGroupPair(msg->getReceiverId(), msg->getGroupId()));

        if (iJoinInfo != joinedAt.end() && iJoinInfo->second < msg->getTimestamp()) {
            
            // Check if this message has not already been received
            NodeMessagePair nmp = NodeMessagePair(msg->getReceiverId(), msg->getMcastId());
            if (receivedAt.find(nmp) == receivedAt.end()) {
                iGroup->second.received += 1;
                receivedAt[nmp] = msg->getTimestamp();
            }
        }
    }
    else {
        RECORD_STATS(++numLooped);
    }
}

/**
 * Notifies the observer that the node doesn't exist anymore.
 */
void MessageObserver::nodeDead(int moduleId) {
    // For each group, if node has joined group, decrease group member count by one
    // and clear joined info.
    for (std::map<OverlayKey, MulticastGroup>::iterator ig = groups.begin(); ig != groups.end(); ++ig) {
        NodeGroupPair ngp = NodeGroupPair(moduleId, ig->first);
        if (joinedAt.find(ngp) != joinedAt.end()) {
            ig->second.size--;
            joinedAt.erase(ngp);
        }
    }
}

std::ostream& operator<< (std::ostream& os, MessageObserver::MulticastGroup const & mg) {
    return os << "Nodes: " << mg.size << "; Messages Sent: " << mg.sent
        << ", Received: " << mg.received << ", Dropped: " << (mg.sent - mg.received);
}

std::ostream& operator<< (std::ostream& os, MessageObserver::NodeGroupPair const & ngp) {
    cModule* module = OPP::cSimulation::getActiveSimulation()->getModule(ngp.first);
    return os << "(" << (module != NULL ? module->getFullPath() : "Deleted node")
        << ", " << ngp.second << ")";
}

