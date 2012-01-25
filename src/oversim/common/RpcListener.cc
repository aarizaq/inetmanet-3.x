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
 * @file RpcListener.cc
 * @author Sebastian Mies
 */

#include <RpcListener.h>
#include <RpcState.h>


RpcListener::~RpcListener()
{}

void RpcListener::handleRpcResponse(BaseResponseMessage* msg,
                                    cPolymorphic* context,
                                    int rpcId, simtime_t rtt)
{
    //std::cout << "Default RpcListener Response: from="
    //          << msg->getSrcNode().getIp() << " msg=" << *msg << std::endl;
}

void RpcListener::handleRpcResponse(BaseResponseMessage* msg,
                                    const RpcState& state, simtime_t rtt)
{
    handleRpcResponse(msg, state.getContext(), state.getId(), rtt);
}

void RpcListener::handleRpcTimeout(BaseCallMessage* msg,
                                   const TransportAddress& dest,
                                   cPolymorphic* context, int rpcId,
                                   const OverlayKey& destKey)
{
    //std::cout << "Default RpcListener Timeout: " << msg->getName()
    //          << std::endl;
}

void RpcListener::handleRpcTimeout(const RpcState& state) {
    handleRpcTimeout(const_cast<BaseCallMessage*>(state.getCallMsg()),
                     state.getDest(), state.getContext(),
                     state.getId(), state.getDestKey());
}
