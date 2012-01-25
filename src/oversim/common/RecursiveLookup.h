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
 * @file RecursiveLookup.h
 * @author Bernhard Heep
 */

#ifndef __RECURSIVE_LOOKUP_H
#define __RECURSIVE_LOOKUP_H

#include <NodeVector.h>
#include <AbstractLookup.h>
#include <RpcListener.h>
#include <CommonMessages_m.h>

class LookupListener;
class BaseOverlay;

/**
 * This class holds the lookup configuration.
 *
 * @author Bernhard Heep
 */
class RecursiveLookupConfiguration
{
public:
    int redundantNodes;   /**< number of next hops in each step */
    int numRetries;
    bool failedNodeRpcs; //!< communicate failed nodes
};

class RecursiveLookup : public RpcListener,
                        public AbstractLookup
{
public:
    RecursiveLookup(BaseOverlay* overlay, RoutingType routingType,
                    const RecursiveLookupConfiguration& config,
                    bool appLookup);
    /**
     * Virtual destructor
     */
    virtual ~RecursiveLookup();

    // see IterativeLookup.h
    virtual void lookup(const OverlayKey& key, int numSiblings = 1,
                        int hopCountMax = 0, int retries = 0,
                        LookupListener* listener = NULL);

    /**
     * Returns the result of the lookup
     *
     * @return The result node vector.
     */
    virtual const NodeVector& getResult() const;

    /**
     * Returns true, if the lookup was successful.
     *
     * @return true, if the lookup was successful.
     */
    virtual bool isValid() const;

    /**
     * Aborts a running lookup.
     *
     * This method aborts a running lookup without calling the
     * listener and delete the lookup object.
     */
    virtual void abortLookup();

    /**
     * Returns the total number of hops for all lookup paths.
     *
     * @return The accumulated number of hops.
     */
    virtual uint32_t getAccumulatedHops() const;

    void handleRpcTimeout(BaseCallMessage* msg,
                          const TransportAddress& dest,
                          cPolymorphic* context, int rpcId,
                          const OverlayKey& destKey);

    void handleRpcResponse(BaseResponseMessage* msg, cPolymorphic* context,
                           int rpcId, simtime_t rtt);
 private:
    BaseOverlay* overlay;
    LookupListener* listener;
    uint32_t nonce;
    bool valid;
    NodeVector siblings;
    RoutingType routingType;
    int redundantNodes;
    int numRetries;
    bool appLookup;
};

#endif //__RECURSIVE_LOOKUP_H
