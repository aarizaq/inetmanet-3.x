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
 * @file GiaMessageBookkeeping.h
 * @author Robert Palmer
 */

#ifndef __GIAMESSAGEBOOKKEEPING_H_
#define __GIAMESSAGEBOOKKEEPING_H_

#include <vector>
#include <queue>
#include <map>

#include "GiaNeighbors.h"
#include "GiaMessage_m.h"
#include "GiaNode.h"


/**
 *
 * This class contains all send messages and their timestamp.
 * It is used for timing out old messages and for biased random walk.
 *
 */
class GiaMessageBookkeeping
{
  public:

    /**
     * Constructor
     * @param neighbors Pointer to neighbors-list
     * @param timeout Value for timing out old messages
     */
    GiaMessageBookkeeping( GiaNeighbors* neighbors, uint32_t timeout );

    /**
     * Destructor
     */
    ~GiaMessageBookkeeping();

    /**
     * 
     * @return Size of MessageBookkeeping-List
     */
    uint32_t getSize();

    /**
     * Add GiaMessage to MessageBookkeeping
     * @param msg This is a GiaIDMessage
     */
    void addMessage( GiaIDMessage* msg );

    /**
     * Removes GiaMessage from MessageBookkeeping
     * @param msg This is a GiaIDMessage
     */
    void removeMessage( GiaIDMessage* msg );

    /**
     * @param msg This is a GiaIDMessage
     * @return true, if MessageBookkeeping contains GiaMessage
     */
    bool contains( GiaIDMessage* msg );

    /**
     *
     * @param msg This is a GiaIDMessage
     * @return Next hop to route message to, NULL, if no token available
     */
    NodeHandle getNextHop( GiaIDMessage* msg );

    /**
     * Removes timedout messages from list
     */
    void removeTimedoutMessages();

  protected:

class GiaNodeQueueCompare : public std::binary_function<FullGiaNodeInfo, FullGiaNodeInfo, FullGiaNodeInfo>
    {
    public:
        bool operator()(const FullGiaNodeInfo& x, const FullGiaNodeInfo& y);
    };

    struct MessageItem
    {
        std::vector<GiaNode> remainNodes; /**< contains all nodes, to which this message has NOT ALREADY been forwarded */
        simtime_t timestamp; /** last time this message arrived at this node */
    };
    std::map<OverlayKey, MessageItem> messages; /**< contains all sent messages */
    GiaNeighbors* neighbors; /**< pointer to our neighbor list */
    uint32_t timeout; /** timeout for messages in ms */
};

#endif
