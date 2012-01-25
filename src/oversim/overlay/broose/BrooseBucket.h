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
 * @file BrooseBucket.h
 * @author Jochen Schenk
 */

#ifndef __BROOSEBUCKET_H_
#define __BROOSEBUCKET_H_

#include <omnetpp.h>
#include "BrooseHandle.h"
#include "Broose.h"
#include <map>
#include <OverlayKey.h>

class Broose;

/**
 * Broose bucket module
 *
 * This modul contains the bucket of the Broose implementation.
 *
 * @author Jochen Schenk
 * @see Broose
 */
class BrooseBucket : public cSimpleModule
{
public:
    virtual int numInitStages() const
    {
        return MAX_STAGE_OVERLAY + 1;
    }

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage* msg);

    // bucket functions
    /**
     * adds a broose node handle to the bucket
     *
     * @param node the NodeHandle to add
     * @param isAlive true, if it is known that the node is alive
     * @param rtt measured round-trip-time to node
     * @return true, if the node was known or has been added
     */
    virtual bool add(const NodeHandle& node, bool isAlive = false,
                     simtime_t rtt = MAXTIME);

    /**
     * removes a broose node handle from the bucket
     *
     * @param node broose handle which will be removed
     */
    virtual void remove(const NodeHandle& node);

    /**
     * returns a specific broose handle
     *
     * @param pos position of the broose handle in the bucket
     */
    virtual const BrooseHandle& get(uint32_t pos = 0);

    /**
     * returns distance of a specific broose handle
     *
     * @param pos position of the broose handle in the bucket
     */
    virtual const OverlayKey& getDist(uint32_t pos = 0);

    /**
     * initializes a bucket
     *
     * @param shiftingBits specifies the kind of the bucket
     * @param prefix in case of a R bucket specifies the prefix
     * @param size maximal size of the bucket
     * @param overlay pointer to the main Broose module
     * @param isBBucket true, is this bucket is the node's BBucket
     */
    virtual void initializeBucket(int shiftingBits, uint32_t prefix,
                                  int size, Broose* overlay,
                                  bool isBBucket = false);

    /**
     * returns number of current entries
     *
     * @return number of current entries
     */
    virtual uint32_t getSize();

    /**
     * returns number of maximal entries
     *
     * @return number of maximal entries
     */
    virtual uint32_t getMaxSize();

    /**
     * Fills a NodeVector with all bucket entries
     *
     * @param result a pointer to an existing NodeVector, which gets filled with
     *               all bucket entries
     */
    virtual void fillVector(NodeVector* result);

    /**
     * checks if the bucket is empty
     *
     * @return true if the bucket is empty
     */
    virtual bool isEmpty();

    /**
     * removes all entries from the bucket
     */
    virtual void clear();

    /**
     * return the longest prefix of all entries
     *
     * @return the longest prefix
     */
    virtual int longestPrefix(void);

    /**
     * checks if the key close to the owner's id
     *
     * @param key key to check
     * @return true if node is close
     */
    virtual bool keyInRange(const OverlayKey& key);

    /**
     * returns the position of a node in this bucket
     *
     * @param node broose handle to check
     * @return the position of the node (0 = first node, -1 = node not in bucket)
     */
    virtual int getPos(const NodeHandle& node);

    /**
     * returns the number of failed responses to a specific broose handle
     *
     * @param node broose handle to check
     * @return number of failed responses
     */
    virtual int getFailedResponses (const NodeHandle& node);

    /**
     * increase the number of failed responses to a specific broose handle
     *
     * @param node broose handle which counter will be increased
     */
    virtual void increaseFailedResponses (const NodeHandle& node);

    /**
     * resets the counter of failed responses to a specific broose handle
     *
     * @param node broose handle which counter will be reset
     */
    virtual void resetFailedResponses (const NodeHandle& node);

    /**
     * sets the round trip time to a specific broose handle
     *
     * @param node broose handle to which the rtt will be stored
     * @param rpcRTT rtt to the specific node
     */
    virtual void setRTT(const NodeHandle& node, simtime_t rpcRTT);

    /**
     * returns the round trip time to a specific broose handle
     *
     * @param node broose handle to which the rtt will be retrieved
     * @return rtt to the specific node
     */
    virtual simtime_t getRTT(const NodeHandle& node);

    /**
     * updates the timestamp of a specific node
     *
     * @param node broose handle to which the timestamp will be updated
     * @param lastSeen timestamp of the last message
     */
    virtual void setLastSeen(const NodeHandle& node, simtime_t lastSeen);

    /**
     * returns the timestamp of a specific node
     *
     * @param node broose handle to which the timestamp will be retrieved
     * @return the retrieved timestamp
     */
    virtual simtime_t getLastSeen(const NodeHandle& node);

    /**
     * displays the content of the bucket on standard out
     *
     * @param maxEntries the maximal number of entries which will be printed
     */
    void output(int maxEntries = 0);


protected:
    // parameter
    std::map<OverlayKey, BrooseHandle> bucket; /**< data structure representing the bucket */
    std::map<OverlayKey, BrooseHandle>::iterator bucketIter; /**< iterator to navigate through the bucket */

    unsigned int maxSize; /**< maximal size of the bucket */
    OverlayKey key; /**< the node's key shifted to fit the bucket and used to measure distance to other keys */
    Broose* overlay; /**< pointer to the main Broose module */
    bool isBBucket; /**< true, if this bucket is the node's BBucket */
};
#endif
