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
 * @file GiaTokenFactory.cc
 * @author Robert Palmer
 */

#include <assert.h>

#include "GiaTokenFactory.h"
#include "Gia.h"


Define_Module(GiaTokenFactory);


void GiaTokenFactory::initialize( int stage )
{
    // wait until IPAddressResolver finished his initialization
    if(stage != MIN_STAGE_OVERLAY)
        return;

    gia = check_and_cast<Gia*>(getParentModule()->getSubmodule("gia"));

    stat_sentTokens = 0;

    WATCH(stat_sentTokens);
    WATCH_VECTOR(tokenQueueVector);
}

void GiaTokenFactory::handleMessages( cMessage* msg )
{
    error("this module doesn't handle messages, it runs only in initialize()");
}

void GiaTokenFactory::setNeighbors( GiaNeighbors* nghbors )
{
    neighbors = nghbors;
}

void GiaTokenFactory::setMaxHopCount( uint32_t maxHopCount)
{
    this->maxHopCount = maxHopCount;
}

void GiaTokenFactory::grantToken()
{
    if (neighbors->getSize() == 0) return;

    // create priority queue
    createPriorityQueue();

    // update sentTokenCount at node on top of priority queue
    updateSentTokens();

    // send token to top of queue
    assert( tokenQueue.size() );
    assert( !tokenQueue.top().node.isUnspecified() );
    gia->sendToken(tokenQueue.top().node);

    // increse statistic variable
    stat_sentTokens++;

    updateQueueVector();
}

void GiaTokenFactory::createPriorityQueue()
{
    clearTokenQueue();
    for (uint32_t i = 0; i < neighbors->getSize(); i++ ) {
        FullGiaNodeInfo temp;
        temp.node= neighbors->get(i);
        temp.info = neighbors->get(temp.node);
        //temp.setCapacity(tempInfo->capacity);
        //temp.setSentTokens(tempInfo->sentTokens);

        tokenQueue.push(temp);
    }
}

void GiaTokenFactory::clearTokenQueue()
{
    while( !tokenQueue.empty() )
        tokenQueue.pop();
}

void GiaTokenFactory::updateQueueVector()
{
    // fill tokenQueueVector
    tokenQueueVector.clear();
    while (!tokenQueue.empty()) {
        tokenQueueVector.push_back(tokenQueue.top().node);
        tokenQueue.pop();
    }
}

void GiaTokenFactory::updateSentTokens()
{
    if (tokenQueue.empty()) return;

    tokenQueue.top().info->sentTokens++;
}

bool GiaTokenFactory::tokenCompareGiaNode::operator()(const FullGiaNodeInfo& x,
                                                      const FullGiaNodeInfo& y)
{
    if (x.info->sentTokens > y.info->sentTokens)
        return true;
    if (x.info->sentTokens == y.info->sentTokens)
        if (x.node.getCapacity() < y.node.getCapacity()) //???
            return true;
    return false;
}
