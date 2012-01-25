//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file P2pnsCache.cc
 * @author Ingmar Baumgart
 */

#include <omnetpp.h>

#include "P2pnsCache.h"

Define_Module(P2pnsCache);

using namespace std;

std::ostream& operator<<(std::ostream& os, const P2pnsIdCacheEntry entry)
{
    os << "key: " << entry.key << " addr: " << entry.addr
       << " state: " << entry.state << " lastUsage: " << entry.lastUsage
       << " queueSize: " << entry.payloadQueue.size();

    return os;
}

std::ostream& operator<<(std::ostream& Stream, const P2pnsCacheEntry entry)
{
    Stream << "Value: " << entry.value;

    if (entry.ttlMessage != NULL) {
        Stream << "Endtime: " << entry.ttlMessage->getArrivalTime();
    }

    return Stream;
}


void P2pnsCache::initialize(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    WATCH_MAP(cache);
    WATCH_MAP(idCache);
}

void P2pnsCache::handleMessage(cMessage* msg)
{
    error("This module doesn't handle messages!");
}

void P2pnsCache::clear()
{
  map<BinaryValue, P2pnsCacheEntry>::iterator iter;
  for( iter = cache.begin(); iter != cache.end(); iter++ ) {
    cancelAndDelete(iter->second.ttlMessage);
  }
  cache.clear();
}


uint32_t P2pnsCache::getSize()
{
    return cache.size();
}

bool P2pnsCache::isEmpty()
{
    if (cache.size() == 0)
        return true;
    else
        return false;
}

P2pnsIdCacheEntry* P2pnsCache::getIdCacheEntry(const OverlayKey& key)
{
    P2pnsIdCache::iterator it = idCache.find(key);

    if (it != idCache.end()) {
        return &it->second;
    } else {
        return NULL;
    }
}

P2pnsIdCacheEntry* P2pnsCache::addIdCacheEntry(const OverlayKey& key,
                                               const BinaryValue* payload)
{
    P2pnsIdCache::iterator it = idCache.find(key);

    if (it == idCache.end()) {
        it = idCache.insert(make_pair(key, P2pnsIdCacheEntry(key))).first;
    }

    if (payload != NULL) {
        it->second.payloadQueue.push_back(*payload);
    }

    it->second.lastUsage = simTime();

    return &it->second;
}

void P2pnsCache::removeIdCacheEntry(const OverlayKey& key)
{
    idCache.erase(key);
}

const BinaryValue& P2pnsCache::getData(const BinaryValue& name) {

    std::map<BinaryValue, P2pnsCacheEntry>::iterator it =  cache.find(name);

    if (it == cache.end())
        return BinaryValue::UNSPECIFIED_VALUE;
    else
        return it->second.value;
}



cMessage* P2pnsCache::getTtlMessage(const BinaryValue& name){
    std::map<BinaryValue, P2pnsCacheEntry>::iterator it = cache.find(name);

    if (it == cache.end())
        return NULL;
    else
        return it->second.ttlMessage;
}


const BinaryValue& P2pnsCache::getDataAtPos(uint32_t pos)
{
    if (pos >= cache.size()) {
        error("Index out of bound (P2pnsCache, getDataAtPos())");
    }

    std::map<BinaryValue, P2pnsCacheEntry>::iterator it = cache.begin();
    for (uint32_t i= 0; i < pos; i++) {
        it++;
        if (i == (pos-1))
            return it->second.value;
    }
    return it->second.value;
}


void P2pnsCache::addData(BinaryValue name, BinaryValue value, cMessage* ttlMessage)
{
    P2pnsCacheEntry entry;
    entry.value = value;
    entry.ttlMessage = ttlMessage;
    // replace with new value
    cache.erase(name);
    cache.insert(make_pair(name, entry));
}

void P2pnsCache::removeData(const BinaryValue& name)
{
    cache.erase(name);
}

void P2pnsCache::updateDisplayString()
{
// FIXME: doesn't work without tcl/tk
    //if (ev.isGUI()) {
    if (1) {
        char buf[80];

        if (cache.size() == 1) {
            sprintf(buf, "1 data item");
        } else {
            sprintf(buf, "%zi data items", cache.size());
        }

        getDisplayString().setTagArg("t", 0, buf);
        getDisplayString().setTagArg("t", 2, "blue");
    }

}

void P2pnsCache::updateTooltip()
{
    if (ev.isGUI()) {
        std::stringstream str;
        for (uint32_t i = 0; i < cache.size(); i++)	{
            str << getDataAtPos(i);

            if ( i != cache.size() - 1 )
                str << endl;
        }


        char buf[1024];
        sprintf(buf, "%s", str.str().c_str());
        getDisplayString().setTagArg("tt", 0, buf);
    }
}

void P2pnsCache::display()
{
    cout << "Content of P2pnsCache:" << endl;
    for (std::map<BinaryValue, P2pnsCacheEntry>::iterator it = cache.begin();
         it != cache.end(); it++) {
        cout << "name: " << it->first << " Value: " << it->second.value << "End-time: " << it->second.ttlMessage->getArrivalTime() << endl;
    }
}
