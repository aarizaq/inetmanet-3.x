/*
 *  Copyright (C) 2012 Nikolaos Vastardis
 *  Copyright (C) 2012 University of Essex
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <omnetpp.h>
#include <stdlib.h>
#include <string.h>
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "SaorsManager.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"
#include  "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"

namespace inet {

namespace inetmanet {

Define_Module(SaorsManager);
//Define_Module(SaorsManagerStatic);


/*****************************************************************************************
 * The Initialization function of the class.
 *****************************************************************************************/
void SaorsManager::initialize(int stage)
{
    bool manetPurgeRoutingTables=false;
    if (stage==4)
    {
        DTActive = (bool) par("manetActive");
        routingProtocol = par("routingProtocol").stringValue ();
        if (DTActive)
        {

            manetPurgeRoutingTables = (bool) par("manetPurgeRoutingTables");
            if (manetPurgeRoutingTables)
            {
                IIPv4RoutingTable *rt = findModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
                IPv4Route *entry;
                //Clean the route table wlan interface entry
                for (int i=rt->getNumRoutes()-1; i>=0; i--)
                {
                    entry= rt->getRoute(i);
                    const InterfaceEntry *ie = entry->getInterface();
                    if (strstr (ie->getName(),"wlan")!=NULL)
                    {
                        rt->deleteRoute(entry);
                    }
                }
            }
            if (par("AUTOASSIGN_ADDRESS"))
            {
                IPv4Address AUTOASSIGN_ADDRESS_BASE(par("AUTOASSIGN_ADDRESS_BASE").stringValue());
                if (AUTOASSIGN_ADDRESS_BASE.getInt() == 0)
                    opp_error("SaorsManager needs AUTOASSIGN_ADDRESS_BASE to be set to 0.0.0.0");
                IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
                IPv4Address myAddr (AUTOASSIGN_ADDRESS_BASE.getInt() + uint32(getParentModule()->getId()));
                for (int k=0; k<ift->getNumInterfaces(); k++)
                {
                    InterfaceEntry *ie = ift->getInterface(k);
                    if (strstr (ie->getName(),"wlan")!=NULL)
                    {
                        ie->ipv4Data()->setIPAddress(myAddr);
                        ie->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS); // full address must match for local delivery
                    }
                }
            }
            // for use dynamic modules in the future //

            if (strcmp("DT-DYMO", routingProtocol)==0)
			{
				if (!gate("to_dtdymo")->isConnected())
				{
					dynamicLoad = true;
					cModuleType *moduleType = cModuleType::find("saors.DT_DYMO.DTDYMO");
					routingModule = moduleType->create("dtroutingprotocol", this);
					// Connet to ip
					routingModule->gate("to_ip")->connectTo(gate("from_dtdymo"));
				}
			}
			else if (strcmp("SAMPhO", routingProtocol)==0)
			{
				if (!gate("to_sampho")->isConnected())
				{
					dynamicLoad = true;
					cModuleType *moduleType = cModuleType::find("saors.SAMPhO.SAMPhO");
					routingModule = moduleType->create("dtroutingprotocol", this);
					//Connet to ip
					routingModule->gate("to_ip")->connectTo(gate("from_sampho"));
				}
			}
			else if (strcmp("EP-DYMO", routingProtocol)==0)
            {
                if (!gate("to_rdymo")->isConnected())
                {
                    dynamicLoad = true;
                    cModuleType *moduleType = cModuleType::find("saors.EP_DYMO.EPDYMO");
                    routingModule = moduleType->create("dtroutingprotocol", this);
                    //Connet to ip
                    routingModule->gate("to_ip")->connectTo(gate("from_epdymo"));
                }
            }
			else if (strcmp("R-DYMO", routingProtocol)==0)
            {
                if (!gate("to_rdymo")->isConnected())
                {
                    dynamicLoad = true;
                    cModuleType *moduleType = cModuleType::find("saors.R_DYMO.RDYMO");
                    routingModule = moduleType->create("dtroutingprotocol", this);
                    //Connet to ip
                    routingModule->gate("to_ip")->connectTo(gate("from_rdymo"));
                }
            }
			else if (strcmp("SimBetTS", routingProtocol)==0)
            {
                if (!gate("to_simbet")->isConnected())
                {
                    dynamicLoad = true;
                    cModuleType *moduleType = cModuleType::find("saors.SimBetTS.SimBetTS");
                    routingModule = moduleType->create("dtroutingprotocol", this);
                    //Connet to ip
                    routingModule->gate("to_ip")->connectTo(gate("from_simbet"));
                }
            }
			
            if (dynamicLoad)
            {
                //Create internals, and schedule it
                routingModule->buildInside();
                routingModule->scheduleStart(simTime());
            }
        }

        //Choose the routing protocol assigned
        if (strcmp("DT-DYMO", routingProtocol)==0)
            routing_protocol = DTDYMO;
        else if (strcmp("SAMPhO", routingProtocol)==0)
            routing_protocol = SAMPhO;
        else if (strcmp("EP-DYMO", routingProtocol)==0)
            routing_protocol = EPDYMO;
        else if (strcmp("R-DYMO", routingProtocol)==0)
            routing_protocol = RDYMO;
        else if (strcmp("SimBetTS", routingProtocol)==0)
            routing_protocol = SIMBETTS;

        ev << "active Ad-hoc routing protocol : " << routingProtocol << "  dynamic : " << dynamicLoad << " \n";
    }
}


/*****************************************************************************************
 * Handles received messages - forwards them to the routing module
 *****************************************************************************************/
void SaorsManager::handleMessage(cMessage *msg)
{
    /* for use dynamic modules in the future */
    /*
        sendDirect(msg, 0, routingModule, "ipIn");
    */
    if (!DTActive)
    {
        delete msg;
        return;
    }

    if (msg->arrivedOn("from_ip"))
    {
        switch (routing_protocol)
        {
            case DTDYMO:
                if (dynamicLoad)
                    sendDirect(msg,routingModule, "from_ip");
                else
                    send( msg, "to_dtdymo");
                break;
            case SAMPhO:
                if (dynamicLoad)
                    sendDirect(msg,routingModule, "from_ip");
                else
                    send( msg, "to_sampho");
                break;
            case EPDYMO:
                if (dynamicLoad)
                    sendDirect(msg,routingModule, "from_ip");
                else
                    send( msg, "to_epdymo");
                break;
            case RDYMO:
                if (dynamicLoad)
                    sendDirect(msg,routingModule, "from_ip");
                else
                    send( msg, "to_rdymo");
                break;
            case SIMBETTS:
                if (dynamicLoad)
                    sendDirect(msg,routingModule, "from_ip");
                else
                    send( msg, "to_simbet");
                break;
        }
    }
    else
    {
        switch (routing_protocol)
        {
            case DTDYMO:
                if (!msg->arrivedOn("from_dtdymo"))
                {
                    delete msg;
                    return;
                }
                break;
            case SAMPhO:
                if (!msg->arrivedOn("from_sampho"))
                {
                    delete msg;
                    return;
                }
                break;
            case EPDYMO:
                if (!msg->arrivedOn("from_epdymo"))
                {
                    delete msg;
                    return;
                }
                break;
            case RDYMO:
                if (!msg->arrivedOn("from_rdymo"))
                {
                    delete msg;
                    return;
                }
                break;
            case SIMBETTS:
                if (!msg->arrivedOn("from_simbet"))
                {
                    delete msg;
                    return;
                }
                break;
        }
        send(msg,"to_ip");
    }
}

} // namespace inetmanet

} // namespace inet

