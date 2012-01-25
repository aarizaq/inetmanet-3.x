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
 * @file RpcListener.h
 * @author Sebastian Mies
 */

#ifndef __RPC_LISTENER_H
#define __RPC_LISTENER_H

class RpcState;
class BaseCallMessage;
class BaseResponseMessage;
class TransportAddress;
class OverlayKey;

#include <omnetpp.h>

/**
 * A Remote-Procedure-Call listener class
 *
 * @author Sebastian Mies
 */
class RpcListener
{
    friend class BaseRpc;

public:

    /**
     * destructor
     */
    virtual ~RpcListener();

protected:
    /**
     * This method is called if an RPC response has been received.
     *
     * @param msg The response message.
     * @param context Pointer to an optional state object. The object
     *                has to be handled/deleted by the handleRpcResponse() code
     * @param rpcId The RPC id.
     * @param rtt The Round-Trip-Time of this RPC
     */
    virtual void handleRpcResponse(BaseResponseMessage* msg,
                                   cPolymorphic* context,
                                   int rpcId, simtime_t rtt);

    /**
     * This method is called if an RPC response has been received.
     *
     * @param msg The response message.
     * @param rpcState Reference to an RpcState object containing e.g. the
     *                 original call message, the destination (TransportAddress
     *                 and/or OverlayKey), a context pointer, ...
     * @param rtt The round-trip time of this RPC
     */
    virtual void handleRpcResponse(BaseResponseMessage* msg,
                                   const RpcState& rpcState, simtime_t rtt);


    /**
     * This method is called if an RPC timeout has been reached.
     *
     * @param msg The original RPC message.
     * @param dest The destination node
     * @param context Pointer to an optional state object. The object
     *                has to be handled/deleted by the handleRpcResponse() code
     * @param rpcId The RPC id.
     * @param destKey the destination OverlayKey
     */
    virtual void handleRpcTimeout(BaseCallMessage* msg,
                                  const TransportAddress& dest,
                                  cPolymorphic* context, int rpcId,
                                  const OverlayKey& destKey);

    /**
      * This method is called if an RPC timeout has been reached.
      *
      * @param rpcState Reference to an RpcState object containing e.g. the
      *                 original call message, the destination (TransportAddress
      *                 and/or OverlayKey), a context pointer, ...
      */
     virtual void handleRpcTimeout(const RpcState& rpcState);

};

#endif
