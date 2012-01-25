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
 * @file DHTDataStorage.h
 * @author Ingmar Baumgart
 */

#ifndef __DHTDATASTORAGE_H_
#define __DHTDATASTORAGE_H_

#include <set>
#include <vector>
#include <map>
#include <sstream>

#include <omnetpp.h>

#include <NodeHandle.h>
#include <InitStages.h>
#include <BinaryValue.h>
#include <NodeVector.h>
#include <CommonMessages_m.h>

/**
 * DHT data storage module
 *
 * This modul contains the data storage of the DHT implementation.
 *
 * @author Ingmar Baumgart
 * @see DHT
 */

typedef std::map<BinaryValue, NodeVector> SiblingVoteMap;

struct DhtDataEntry
{
    BinaryValue value;
    uint32_t kind;
    uint32_t id;
    cMessage* ttlMessage;
    bool is_modifiable;
    NodeHandle sourceNode;
    bool responsible; //is this node responsible for this key ?
    friend std::ostream& operator<<(std::ostream& Stream, const DhtDataEntry entry);
    SiblingVoteMap siblingVote;
};

typedef std::vector<std::pair<OverlayKey, DhtDataEntry> > DhtDataVector;
typedef std::vector<DhtDumpEntry> DhtDumpVector;
typedef std::multimap<OverlayKey, DhtDataEntry> DhtDataMap;

class DHTDataStorage : public cSimpleModule
{
  public:

    virtual int numInitStages() const
    {
        return MAX_STAGE_APP + 1;
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
     * Returns a pointer to the requested stored data item
     *
     * @param key The key of the data item
     * @param kind The kind of the data item
     * @param id A random integer to identify multiple items with same key and kind
     *
     * @return pointer to the data item or NULL if data item is not available
     */
    DhtDataEntry* getDataEntry(const OverlayKey& key,
                               uint32_t kind, uint32_t id);


    /**
     * Returns the stored data items with a given key, kind and id
     *
     * @param key The key of the data item
     * @param kind The kind of the data item
     * @param id A random integer to identify multiple items with same key and kind
     *
     * @return The value of the data item with the given key
     */
    virtual DhtDataVector* getDataVector(const OverlayKey& key,
                                         uint32_t kind = 0,
                                         uint32_t id = 0);

    /**
     * Returns the source node of a stored data item with a given key
     *
     * @param key The key of the data item
     * @param kind The kind of the data item
     * @param id A random integer to identify multiple items with same key and kind
     *
     * @return The source node of the data item with the given key
     */
    virtual const NodeHandle& getSourceNode(const OverlayKey& key,
                                            uint32_t kind, uint32_t id);

    /**
     * Returns a boolean telling if this data if modifiable
     *
     * @param key The key of the data item
     * @param kind The kind of the data item
     * @param id A random integer to identify multiple items with same key and kind
     * @return The value of the is_modifiable value
     */
    virtual const bool isModifiable(const OverlayKey& key,
                                    uint32_t kind, uint32_t id);

    /**
     * Returns an iterator to the beginning of the map
     *
     * @return An iterator
     */
    virtual const DhtDataMap::iterator begin();

    /**
     * Returns an iterator to the end of the map
     *
     * @return An iterator
     */
    virtual const DhtDataMap::iterator end();


    /**
     * Store a new data item in the map
     *
     * @param key The key of the data item to be stored
     * @param kind The kind of the data item
     * @param id A random integer to identify multiple items with same key and kind
     * @param value The value of the data item to be stored
     * @param ttlMessage The self-message sent for the ttl expiration
     * @param is_modifiable Flag that tell if the data can be change by anyone, or just by the sourceNode
     * @param sourceNode Node which asked to store the value
     * @param responsible
     */
    virtual DhtDataEntry* addData(const OverlayKey& key, uint32_t kind,
                                  uint32_t id,
                                  BinaryValue value, cMessage* ttlMessage,
                                  bool is_modifiable=true,
                                  NodeHandle sourceNode=NodeHandle::UNSPECIFIED_NODE,
                                  bool responsible=true);

    /**
     * Removes a certain data item from the map
     *
     * @param key The key of the data item to be removed
     * @param kind The kind of the data item
     * @param id A random integer to identify multiple items with same key and kind
     *
     */
    virtual void removeData(const OverlayKey& key, uint32_t kind, uint32_t id);

    void display();

    /**
     * Dump filtered local data records into a vector
     *
     * @param key The key of the data items to dump
     * @param kind The kind of the data items to dump
     * @param id The id of the data items to dump
     *
     * @return the vector containing all matching data items
     */
    DhtDumpVector* dumpDht(const OverlayKey& key = OverlayKey::UNSPECIFIED_KEY,
                           uint32_t kind = 0, uint32_t id = 0);

  protected:
    DhtDataMap dataMap; /**< internal representation of the data storage */

    /**
     * Displays the current number of successors in the list
     */
    void updateDisplayString();

    /**
     * Displays the first 4 successor nodes as tooltip.
     */
    void updateTooltip();
};

#endif
