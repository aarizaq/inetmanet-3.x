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
 * @file RpcState.h
 * @author Sebastian Mies
 * @author Ingmar Baumgart
 */

#ifndef __RPC_STATE_H
#define __RPC_STATE_H

class RpcListener;

#include "CommonMessages_m.h"

class RpcState
{
    friend class BaseRpc;

public:
    int getId() const { return id; }
    const TransportAddress& getDest() const { return *dest; }
    const OverlayKey& getDestKey() const { return destKey; }
    BaseCallMessage *getCallMsg() const { return callMsg; }
    cPolymorphic *getContext() const { return context; }

private:
    int id;
    int retries;
    TransportType transportType;
    RoutingType routingType;
    CompType destComp;
    CompType srcComp;
    RpcListener* listener;
    const TransportAddress* dest;
    OverlayKey destKey;
    BaseCallMessage *callMsg;
    RpcTimeoutMessage *timeoutMsg;
    simtime_t timeSent;
    simtime_t rto;
    cPolymorphic *context;
};

#endif
