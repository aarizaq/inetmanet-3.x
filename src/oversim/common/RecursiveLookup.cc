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
 * @file RecursiveLookup.cc
 * @author Bernhard Heep
 */

#include <CommonMessages_m.h>

#include <BaseOverlay.h>
#include <LookupListener.h>

#include <RecursiveLookup.h>

RecursiveLookup::RecursiveLookup(BaseOverlay* overlay,
                                 RoutingType routingType,
                                 const RecursiveLookupConfiguration& config,
                                 bool appLookup) :
    overlay(overlay),
    routingType(routingType),
    redundantNodes(config.redundantNodes),
    numRetries(config.numRetries),
    appLookup(appLookup)
{
    valid = false;
}

RecursiveLookup::~RecursiveLookup()
{
    if (listener != NULL) {
        delete listener;
        listener = NULL;
    }

    overlay->removeLookup(this);
}

void RecursiveLookup::lookup(const OverlayKey& key, int numSiblings,
                             int hopCountMax, int retries,
                             LookupListener* listener)
{
    this->listener = listener;

    FindNodeCall* call = new FindNodeCall("FindNodeCall");
    if (appLookup) call->setStatType(APP_LOOKUP_STAT);
    else call->setStatType(MAINTENANCE_STAT);
    call->setLookupKey(key);
    call->setNumRedundantNodes(redundantNodes);
    call->setNumSiblings(numSiblings);
    call->setBitLength(FINDNODECALL_L(call));

    nonce = overlay->sendRouteRpcCall(OVERLAY_COMP, key, call, NULL,
                                      routingType, -1, retries, -1, this);
}

const NodeVector& RecursiveLookup::getResult() const
{
    return siblings;
}

bool RecursiveLookup::isValid() const
{
    return valid;
}

void RecursiveLookup::abortLookup()
{
    overlay->cancelRpcMessage(nonce);

    delete this;
}

uint32_t RecursiveLookup::getAccumulatedHops() const
{
    //throw new cRuntimeError("RecursiveLookup is asked for # accumulated hops!");
    return 0; //TODO hopCount in findNodeCall/Response ?
}

void RecursiveLookup::handleRpcTimeout(BaseCallMessage* msg,
                                       const TransportAddress& dest,
                                       cPolymorphic* context, int rpcId,
                                       const OverlayKey& destKey)
{
    //TODO retry
    valid = false;

    // inform listener
    if (listener != NULL) {
        listener->lookupFinished(this);
        listener = NULL;
    }

    delete this;
}

void RecursiveLookup::handleRpcResponse(BaseResponseMessage* msg,
                                        cPolymorphic* context, int rpcId,
                                        simtime_t rtt)
{
    FindNodeResponse* findNodeResponse = check_and_cast<FindNodeResponse*>(msg);

    if (findNodeResponse->getSiblings() &&
        findNodeResponse->getClosestNodesArraySize() > 0) {
        valid = true;
        for (uint32_t i = 0; i < findNodeResponse->getClosestNodesArraySize();
             i++) {
            siblings.push_back(findNodeResponse->getClosestNodes(i));
        }
    }

//    std::cout << "RecursiveLookup::handleRpcResponse() "
//              << findNodeResponse->getClosestNodesArraySize() << std::endl;

    // inform listener
    if (listener != NULL) {
        listener->lookupFinished(this);
        listener = NULL;
    }
    delete this;
}
