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
 * @file Pastry.h
 * @author Felix Palmen
 */

#ifndef __PASTRY_H_
#define __PASTRY_H_

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
 * Pastry overlay module
 *
 * @author Felix Palmen
 * @see BaseOverlay
 */
class Pastry : public BasePastry
{
  public:
    virtual ~Pastry();

    // see BaseOverlay.h
    virtual void initializeOverlay(int stage);

    // see BaseOverlay.h
    virtual void handleTimerEvent(cMessage* msg);

    // see BaseOverlay.h
    virtual void handleUDPMessage(BaseOverlayMessage* msg);

    void handleStateMessage(PastryStateMessage* msg);

    virtual void pingResponse(PingResponse* pingResponse,
                              cPolymorphic* context, int rpcId,
                              simtime_t rtt);

  protected:

    virtual void purgeVectors(void);

    /**
     * changes node state
     *
     * @param toState state to change to
     */
    virtual void changeState(int toState);


    virtual bool recursiveRoutingHook(const TransportAddress& dest,
                                      BaseRouteMessage* msg);

    void iterativeJoinHook(BaseOverlayMessage* msg, bool incrHopCount);
    /**
     * State messages to process during join
     */
    std::vector<PastryStateMsgHandle> stReceived;
    std::vector<PastryStateMsgHandle>::iterator stReceivedPos;

    /**
     * List of nodes to notify after join
     */
    std::vector<TransportAddress> notifyList;

  private:

    void clearVectors();

    simtime_t secondStageInterval;
    simtime_t routingTableMaintenanceInterval;
    simtime_t discoveryTimeoutAmount;
    bool partialJoinPath;

    int depth;

    int updateCounter;

    bool minimalJoinState;
    bool useDiscovery;
    bool useSecondStage;
    bool sendStateAtLeafsetRepair;
    bool pingBeforeSecondStage;

    bool overrideOldPastry;
    bool overrideNewPastry;

    cMessage* secondStageWait;
    cMessage* ringCheck;
    cMessage* discoveryTimeout;
    cMessage* repairTaskTimeout;

    /**
     * do the second stage of initialization as described in the paper
     */
    void doSecondStage(void);


    /**
    * periodic routing table maintenance
    * requests the corresponding routing table row
    * from one node in each row
    */
    void doRoutingTableMaintenance();

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

    void processState(void);

    bool mergeState(void);

    void endProcessingState(void);

    /**
     * send updated state to all nodes when entering ready state
     */
    void doJoinUpdate(void);

    // see BaseOverlay.h
    virtual void joinOverlay();

};



#endif
