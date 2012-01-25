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
 * @file TraceChurn.cc
 * @author Stephan Krause
 */

#include <PeerInfo.h>
#include <GlobalNodeListAccess.h>
#include <UnderlayConfigurator.h>
#include "TraceChurn.h"

Define_Module(TraceChurn);

using namespace std;

void TraceChurn::initializeChurn()
{
    Enter_Method_Silent();

    // get uppermost tier
    // Quick hack. Works fine unless numTiers is > 9 (which should never happen)
    maxTier = new char[6];
    strcpy(maxTier, "tier0");
    maxTier[4] += par("numTiers").longValue();

    // FIXME: There should be a tracefile command to decide when init phase has finished
    underlayConfigurator->initFinished();
}

void TraceChurn::handleMessage(cMessage* msg)
{
    delete msg;
    return;
}

void TraceChurn::createNode(int nodeId)
{
    Enter_Method_Silent();

    TransportAddress* ta = underlayConfigurator->createNode(type);
    PeerInfo* peer = GlobalNodeListAccess().get()->getPeerInfo(*ta);
    cGate* inGate = simulation.getModule(peer->getModuleID())->getSubmodule(maxTier)->gate("trace_in");
    if (!inGate) {
        throw cRuntimeError("Application has no trace_in gate. Most probably "
                             "that means it is not able to handle trace data.");
    }
    nodeMapEntry* e = new nodeMapEntry(ta, inGate);
    nodeMap[nodeId] = e;
}

void TraceChurn::deleteNode(int nodeId)
{
    Enter_Method_Silent();

    nodeMapEntry* e;
    UNORDERED_MAP<int, nodeMapEntry*>::iterator it = nodeMap.find(nodeId);

    if (it == nodeMap.end()) {
        throw cRuntimeError("Trying to delete non-existing node");
    }

    e = it->second;
    underlayConfigurator->preKillNode(NodeType(), e->first);
    nodeMap.erase(it);
    delete e->first;
    delete e;
}

TransportAddress* TraceChurn::getTransportAddressById(int nodeId) {
    UNORDERED_MAP<int, nodeMapEntry*>::iterator it = nodeMap.find(nodeId);

    if (it == nodeMap.end()) {
    	throw cRuntimeError("Trying to get TransportAddress of nonexisting node");
    }

    return it->second->first;
}

cGate* TraceChurn::getAppGateById(int nodeId) {
    UNORDERED_MAP<int, nodeMapEntry*>::iterator it = nodeMap.find(nodeId);

    if (it == nodeMap.end()) {
    	throw cRuntimeError("Trying to get appGate of nonexisting node");
    }

    return it->second->second;
}

void TraceChurn::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "trace churn");
    getDisplayString().setTagArg("t", 0, buf);
}
