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
 * @file GlobalDhtTestMap.cc
 * @author Ingmar Baumgart
 */

#include <omnetpp.h>

#include <GlobalStatisticsAccess.h>

#include <DHTTestAppMessages_m.h>

#include "GlobalDhtTestMap.h"

using namespace std;

Define_Module(GlobalDhtTestMap);

std::ostream& operator<<(std::ostream& stream, const DHTEntry entry)
{
    return stream << "Value: " << entry.value
                  << " Endtime: " << entry.endtime;
}

GlobalDhtTestMap::GlobalDhtTestMap()
{
    periodicTimer = NULL;
}

GlobalDhtTestMap::~GlobalDhtTestMap()
{
    cancelAndDelete(periodicTimer);
    dataMap.clear();
}

void GlobalDhtTestMap::initialize()
{
    p2pnsNameCount = 0;
    globalStatistics = GlobalStatisticsAccess().get();
    WATCH_MAP(dataMap);

    periodicTimer = new cMessage("dhtTestMapTimer");

    scheduleAt(simTime(), periodicTimer);
}

void GlobalDhtTestMap::finish()
{
}

void GlobalDhtTestMap::handleMessage(cMessage* msg)
{
    //cleanupDataMap();
    DhtTestEntryTimer *entryTimer = NULL;

    if (msg == periodicTimer) {
        RECORD_STATS(globalStatistics->recordOutVector(
           "GlobalDhtTestMap: Number of stored DHT entries", dataMap.size()));
        scheduleAt(simTime() + TEST_MAP_INTERVAL, msg);
    } else if ((entryTimer = dynamic_cast<DhtTestEntryTimer*>(msg)) != NULL) {
        dataMap.erase(entryTimer->getKey());
        delete msg;
    } else {
        throw cRuntimeError("GlobalDhtTestMap::handleMessage(): "
                                "Unknown message type!");
    }
}

void GlobalDhtTestMap::insertEntry(const OverlayKey& key, const DHTEntry& entry)
{
    Enter_Method_Silent();

    dataMap.erase(key);
    dataMap.insert(make_pair(key, entry));

    DhtTestEntryTimer* msg = new DhtTestEntryTimer("dhtEntryTimer");
    msg->setKey(key);

    scheduleAt(entry.endtime, msg);
}

void GlobalDhtTestMap::eraseEntry(const OverlayKey& key)
{
    dataMap.erase(key);
}

const DHTEntry* GlobalDhtTestMap::findEntry(const OverlayKey& key)
{
    std::map<OverlayKey, DHTEntry>::iterator it = dataMap.find(key);

    if (it == dataMap.end()) {
        return NULL;
    } else {
        return &(it->second);
    }
}

const OverlayKey& GlobalDhtTestMap::getRandomKey()
{
    if (dataMap.size() == 0) {
        return OverlayKey::UNSPECIFIED_KEY;
    }

    // return random OverlayKey in O(log n)
    std::map<OverlayKey, DHTEntry>::iterator it = dataMap.end();
    DHTEntry tempEntry = {BinaryValue::UNSPECIFIED_VALUE, 0};

    OverlayKey randomKey = OverlayKey::random();
    it = dataMap.find(randomKey);

    if (it == dataMap.end()) {
        it = dataMap.insert(make_pair(randomKey, tempEntry)).first;
        dataMap.erase(it++);
    }

    if (it == dataMap.end()) {
        it = dataMap.begin();
    }

    return it->first;
}


