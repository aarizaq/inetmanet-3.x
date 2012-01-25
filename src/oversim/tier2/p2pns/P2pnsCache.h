//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file P2pnsCache.h
 * @author Ingmar Baumgart
 */

#ifndef __P2PNSCACHE_H_
#define __P2PNSCACHE_H_

#include <set>
#include <vector>
#include <deque>
#include <sstream>

#include <omnetpp.h>

#include <NodeHandle.h>
#include <InitStages.h>
#include <BinaryValue.h>

struct P2pnsCacheEntry
{
    BinaryValue value;
    cMessage* ttlMessage;
    friend std::ostream& operator<<(std::ostream& Stream,
                                    const P2pnsCacheEntry entry);
};

enum P2pnsConnectionStates {
    CONNECTION_PENDING = 0,
    CONNECTION_ACTIVE = 1
};

class P2pnsIdCacheEntry
{
public:
    P2pnsIdCacheEntry(const OverlayKey& key) :
        key(key), state(CONNECTION_PENDING) {};

public:
    OverlayKey key;
    TransportAddress addr;
    P2pnsConnectionStates state;
    simtime_t lastUsage;
    std::deque<BinaryValue> payloadQueue;
};

typedef std::map<OverlayKey, P2pnsIdCacheEntry> P2pnsIdCache;

/**
 * P2PNS name cache module
 *
 * This modul contains the name cache of the P2PNS implementation.
 *
 * @author Ingmar Baumgart
 * @see P2pns.cc
 */
class P2pnsCache : public cSimpleModule
{
  public:
    virtual int numInitStages() const
    {
        return MAX_STAGE_APP;
    }

    virtual void initialize(int stage);
    virtual void handleMessage(cMessage* msg);

    /**
     * Returns number of stored data items in the map
     *
     * @return number of stored data items
     */
    virtual uint32_t getSize();

    /**
     * Clears all stored data items
     */
    virtual void clear();

    /**
     * Checks if the data storage map is empty
     *
     * @return returns false if there are stored data items, true otherwise.
     */
    virtual bool isEmpty();


    virtual P2pnsIdCacheEntry* getIdCacheEntry(const OverlayKey& key);

    virtual P2pnsIdCacheEntry* addIdCacheEntry(const OverlayKey& key,
                                               const BinaryValue* payload = NULL);

    virtual void removeIdCacheEntry(const OverlayKey& key);

    /**
     * Returns the value of a stored data item with a given name
     *
     * @param name The name of the data item
     * @return The value of the data item with the given name
     */
    virtual const BinaryValue& getData(const BinaryValue& name);

    /**
     * Returns the ttlMessage of a stored data item with a given name
     *
     * @param name The name of the data item
     * @return The ttlMessage of the data item with the given name
     */
    virtual cMessage* getTtlMessage(const BinaryValue& name);

    /**
     * Returns the value of the data item stored at position pos
     *
     * @param pos position in data storage map
     * @return The value of the data item at position pos
     */
    virtual const BinaryValue& getDataAtPos(uint32_t pos = 0);

    /**
     * Store a new data item in the map
     *
     * @param name The name of the data item to be stored
     * @param value The value of the data item to be stored
     * @param ttlMessage The self-message sent for the ttl expiration
     */
    virtual void addData(BinaryValue name, BinaryValue value, cMessage* ttlMessage = NULL);

    /**
     * Removes a certain data item from the map
     *
     * @param name The name of the data item to be removed
     */
    virtual void removeData(const BinaryValue& name);
    void display ();

  protected:
    std::map<BinaryValue, P2pnsCacheEntry> cache; /**< internal representation of the cache */
    P2pnsIdCache idCache; /**< internal representation of the KBR nodeId cache */

    /**
     * Updates the display string
     */
    void updateDisplayString();

    /**
     * Updates the tooltip
     */
    void updateTooltip();
};

#endif
