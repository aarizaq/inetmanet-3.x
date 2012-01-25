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
 * @file SearchMsgBookkeeping.cc
 * @author Robert Palmer
 */

#include "SearchMsgBookkeeping.h"


SearchMsgBookkeeping::~SearchMsgBookkeeping()
{
    // virtual dectructor
}

uint32_t SearchMsgBookkeeping::getSize() const
{
    return messages.size();
}

void SearchMsgBookkeeping::addMessage(const OverlayKey& searchKey)
{
    SearchMessageItem item;
    item.searchKey = searchKey;
    item.creationTime = simTime();
    item.minDelay = 0;
    item.maxDelay = 0;
    item.minHopCount = 0;
    item.maxHopCount = 0;
    item.responseCount = 0;
    messages[searchKey] = item;
}

void SearchMsgBookkeeping::removeMessage(const OverlayKey& searchKey)
{
    SearchBookkeepingListIterator it = messages.find(searchKey);

    if(it->first == searchKey)
        messages.erase(it);
}

bool SearchMsgBookkeeping::contains(const OverlayKey& searchKey) const
{
    SearchBookkeepingListConstIterator it = messages.find(searchKey);
    return (it != messages.end());
}

void SearchMsgBookkeeping::updateItem(const OverlayKey& searchKey,
				      uint32_t hopCount)
{
    SearchBookkeepingListIterator it = messages.find(searchKey);
    SearchMessageItem currentItem;

    if(it->first == searchKey) {
        currentItem = it->second;
        simtime_t currentTime = simTime();

        simtime_t delay = currentTime - currentItem.creationTime;

        // initialize first minDelay
        if (currentItem.minDelay == 0)
            currentItem.minDelay = delay;
        // initialize first minHopCount
        if (currentItem.minHopCount == 0)
            currentItem.minHopCount = hopCount;

        if (delay < currentItem.minDelay)
            currentItem.minDelay = delay;
        if (delay > currentItem.maxDelay)
            currentItem.maxDelay = delay;

        if (hopCount < currentItem.minHopCount)
            currentItem.minHopCount = hopCount;
        if (hopCount > currentItem.maxHopCount)
            currentItem.maxHopCount = hopCount;

        currentItem.responseCount++;

        it->second = currentItem;
    }
}

GiaSearchStats SearchMsgBookkeeping::getStatisticalData() const
{
    SearchMessageItem currentItem;
    GiaSearchStats temp = {0, 0, 0, 0, 0};

    uint32_t size = messages.size();

    if (size == 0) return temp;

    for(SearchBookkeepingListConstIterator it = messages.begin();
                                           it != messages.end(); it++) {
        currentItem = it->second;
        temp.minDelay += (float)SIMTIME_DBL(currentItem.minDelay);
        temp.maxDelay += (float)SIMTIME_DBL(currentItem.maxDelay);
        temp.minHopCount += currentItem.minHopCount;
        temp.maxHopCount += currentItem.maxHopCount;
        temp.responseCount += currentItem.responseCount;
    }

    temp.minDelay /= size;
    temp.maxDelay /= size;
    temp.minHopCount /= size;
    temp.maxHopCount /= size;
    temp.responseCount /= size;

    return temp;
}
