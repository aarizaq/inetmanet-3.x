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
 * @file Bamboo.h
 * @author Gerhard Petruschat
 */

#ifndef __BAMBOO_H_
#define __BAMBOO_H_

#include <vector>
#include <map>
#include <queue>
#include <algorithm>

#include <omnetpp.h>
#include <IPvXAddress.h>

#include <OverlayKey.h>
#include <NodeHandle.h>
#include <BaseOverlay.h>
#include <BasePastry.h>

#include "PastryTypes.h"
#include "PastryMessage_m.h"
#include "PastryRoutingTable.h"
#include "PastryLeafSet.h"
#include "PastryNeighborhoodSet.h"


/**
 * Bamboo overlay module
 *
 * @author Gerhard Petruschat
 * @see BaseOverlay
 */
class Bamboo : public BasePastry, public LookupListener
{
    friend class BambooLookupListener;

  public:

    virtual ~Bamboo();

    // see BaseOverlay.h
    virtual void initializeOverlay(int stage);

    // see BaseOverlay.h
    virtual void handleTimerEvent(cMessage* msg);

    // see BaseOverlay.h
    virtual void handleUDPMessage(BaseOverlayMessage* msg);

    void handleStateMessage(PastryStateMessage* msg);

  protected:

    void lookupFinished(AbstractLookup *lookup);
    /**
     * changes node state
     *
     * @param toState state to change to
     */
    virtual void changeState(int toState);

  private:

    // local state tables
    simtime_t leafsetMaintenanceInterval;
    simtime_t localTuningInterval;
    simtime_t globalTuningInterval;

    cMessage* leafsetMaintenanceTimer;
    cMessage* globalTuningTimer;
    cMessage* localTuningTimer;

    /**
     * periodically repairs the leafset by pushing it to and pulling it from
     * from a random live leafset node
     */
    void doLeafsetMaintenance(void);

    int getNextRowToMaintain();

    void doLocalTuning();

    /**
     * periodically repairs the routing table by performing random lookups
     */
    void doGlobalTuning(void);

    /**
     * notifies leafset and routingtable of a failed node and sends out
     * a repair request if possible
     *
     * @param failed the failed node
     * @return true as long as local state is READY (signals lookup to try
     * again)
     */
    bool handleFailedNode(const TransportAddress& failed);

     /**
     * checks whether proxCache is complete, takes appropriate actions
     * depending on the protocol state
     */
    void checkProxCache(void);

    // see BaseOverlay.h
    virtual void joinOverlay();

};

class BambooLookupListener : public LookupListener
{
  private:
    Bamboo* overlay;

  public:
    BambooLookupListener(Bamboo* overlay)
    {
        this->overlay = overlay;
    }

    virtual void lookupFinished(AbstractLookup *lookup)
    {
        overlay->lookupFinished(lookup);
        delete this;
    }
};


#endif
