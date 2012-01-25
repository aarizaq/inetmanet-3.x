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
 * @file GiaMessageBookkeeping.cc
 * @author Robert Palmer
 */

#include <assert.h>

#include <omnetpp.h>
#include "GiaMessageBookkeeping.h"


GiaMessageBookkeeping::GiaMessageBookkeeping( GiaNeighbors* neighbors, uint32_t timeout )
{
    this->neighbors = neighbors;
    this->timeout = timeout;
}

GiaMessageBookkeeping::~GiaMessageBookkeeping()
{}

uint32_t GiaMessageBookkeeping::getSize()
{
    return messages.size();
}

void GiaMessageBookkeeping::addMessage( GiaIDMessage* msg )
{
    assert(!(msg->getID().isUnspecified()));

    std::vector<GiaNode> remainNodes;
    // push all neighbors except the node where message was comming from
    // to remainNodes
    for ( uint32_t i=0; i<neighbors->getSize(); i++ ) {
        if ( neighbors->get(i).getKey() != msg->getSrcNode().getKey())
            remainNodes.push_back(neighbors->get(i));
    }
    MessageItem messageItem;
    messageItem.remainNodes = remainNodes;
    messageItem.timestamp = simTime();
    messages[msg->getID()] = messageItem;
}

void GiaMessageBookkeeping::removeMessage( GiaIDMessage* msg )
{
    std::map<OverlayKey, MessageItem>::iterator it = messages.find(msg->getID());
    // delete message if key is equal
    if ( it->first == msg->getID() )
        messages.erase(messages.find(msg->getID()));
}

bool GiaMessageBookkeeping::contains( GiaIDMessage* msg )
{
    std::map<OverlayKey, MessageItem>::iterator it = messages.find(msg->getID());

    if(it != messages.end())
        return true;
    return false;
}

NodeHandle GiaMessageBookkeeping::getNextHop( GiaIDMessage* msg )
{
    if ( neighbors->getSize() > 0 ) {
        std::map<OverlayKey, MessageItem>::iterator it = messages.find(msg->getID());
        std::priority_queue<FullGiaNodeInfo, std::vector<FullGiaNodeInfo>, GiaNodeQueueCompare> nodeQueue;

        if ( it != messages.end() && it->first == msg->getID() ) {
            MessageItem messageItem = it->second;
            std::vector<GiaNode> remNodes = messageItem.remainNodes;
            if ( remNodes.size() == 0) {
                for ( uint32_t i=0; i<neighbors->getSize(); i++ ) {
                    remNodes.push_back(neighbors->get(i));
                }
            }

            for ( uint32_t i=0; i<remNodes.size(); i++ ) {
                if(!(remNodes[i].isUnspecified())) {
                    FullGiaNodeInfo temp;
                    temp.node = remNodes[i];
                    temp.info = neighbors->get(temp.node);
                    if (temp.info) nodeQueue.push(temp);
                }
            }

            if (!nodeQueue.empty()) {
                NodeHandle nextHop = nodeQueue.top().node;
                GiaNeighborInfo* nextHopInfo = neighbors->get(nextHop);
                nodeQueue.pop();

                if (nextHopInfo != NULL &&  nextHopInfo->receivedTokens > 0 ) {
                    remNodes.clear();
                    while ( !nodeQueue.empty() ) {
                        remNodes.push_back(nodeQueue.top().node);
                        nodeQueue.pop();
                    }
                    messageItem.remainNodes = remNodes;
                    messageItem.timestamp = simTime();
                    messages[msg->getID()] = messageItem;
                    return nextHop;
                }
            }
	}
    }
    return NodeHandle::UNSPECIFIED_NODE;
}

void GiaMessageBookkeeping::removeTimedoutMessages()
{
    std::map<OverlayKey, MessageItem>::iterator it = messages.begin();
    std::map<OverlayKey, MessageItem>::iterator it2 = messages.begin();
    for ( uint32_t i=0; i<messages.size(); i++) {
        OverlayKey key = it->first;
        MessageItem messageItem = it->second;
        it2 = it++;
        if (simTime() > (messageItem.timestamp + timeout))
            messages.erase(it2);
    }
}

bool GiaMessageBookkeeping::GiaNodeQueueCompare::operator()(const FullGiaNodeInfo& x,
                                                            const FullGiaNodeInfo& y)
{
    if (x.info->receivedTokens > y.info->receivedTokens) {
        if (y.info->receivedTokens == 0)
            return false;
        else {
            if (x.node.getCapacity() >= y.node.getCapacity())
                return false;
            else
                return true;
        }
    }
    else if (x.info->receivedTokens < y.info->receivedTokens) {
        if (x.info->receivedTokens == 0)
            return true;
        else {
            if (x.node.getCapacity() > y.node.getCapacity())
                return false;
            else
                return true;
        }
    }
    else {
        if (x.info->receivedTokens == 0)
            return true;
        else {
            if (x.node.getCapacity() > y.node.getCapacity())
                return false;
            else
                return true;
        }
    }
}
