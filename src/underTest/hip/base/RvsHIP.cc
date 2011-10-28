//*********************************************************************************
// File:           RvsHIP.cc
//
// Authors:        Laszlo Tamas Zeke, Levente Mihalyi, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
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
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************


#include "RvsHIP.h"
#include "IPv6ControlInfo.h"
#include "HipMessages_m.h"
#include "IPv6Datagram.h"

Define_Module(RvsHIP);

RvsHIP::RvsHIP(){}
RvsHIP::~RvsHIP(){}
void RvsHIP::specInitialize()
{
     _isRvs = true;
     ownHIT.set(this->par("OWN_HIT"));
}

// Handles incoming I1 messages
void RvsHIP::handleMsgFromNetwork(cMessage *msg)
{
     if(dynamic_cast<HIPHeaderMessage *>(msg)!= NULL)
     {
        HIPHeaderMessage *hipHeader = check_and_cast<HIPHeaderMessage *>(msg);
        //HIP msg
        if (hipHeader != NULL)
        {
           IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());

            //dest is RVS's HIT, it's a registration
			ev << "destHIT ? ownHIT " << hipHeader->getDestHIT() << " ? " << ownHIT << endl;
            if (hipHeader->getDestHIT() == ownHIT)
            {
				ev << "true" << endl;
				int fsmId = -1;
				if(hitToIpMap.find(hipHeader->getSrcHIT()) != hitToIpMap.end())
					fsmId = hitToIpMap.find(hipHeader->getSrcHIT())->second->fsmId;
               if(fsmId < 0)
				{
					//create new daemon and fw to it
					ev << "RVS received I1" << endl;
					msg->setControlInfo(networkControlInfo);
					sendDirect(msg, createStateMachine(networkControlInfo->getSrcAddr(), hipHeader->getSrcHIT()), "remoteIn");
				}
				else
				{
					//spi is known, find daemon and fw to it
					ev << "RVS received SPI (possible UPDATE, BE) " << hipHeader->getLocator(1).locatorESP_SPI << endl;
					msg->setControlInfo(networkControlInfo);
					sendDirect(msg, findStateMachine(fsmId),"remoteIn");
				}
			}
				//not registration, somebody's trying to reach destHIT, fw I1 to it
			else
			{
                 hitToIpMapIt = hitToIpMap.find(hipHeader->getDestHIT());
                 alterHipPacketAndSend(hipHeader, networkControlInfo->getDestAddr(), hitToIpMapIt->second->addr.front(), networkControlInfo->getSrcAddr());
			}

        }
        else
        {
           delete msg;
        }
     }
}

// Forwards I1 messages to the registered host's IP address
void RvsHIP::alterHipPacketAndSend(HIPHeaderMessage* hipHead, IPv6Address &rvsIP, IPv6Address &destIP, IPv6Address &fromIP)
     {
        //create new dest locator
        HipLocator *destLoc = new HipLocator();
        destLoc->locatorESP_SPI = -1;
        destLoc->locatorIPv6addr = destIP;
        hipHead->setLocator(1,*destLoc);
        //set HIPHeader's FROM field
        hipHead->setFrom_i(fromIP);
        hipHead->setRvs_mac(1);
		IPv6ControlInfo *networkControlInfo = new IPv6ControlInfo();
		networkControlInfo->setDestAddr(destIP);
		networkControlInfo->setSrcAddr(rvsIP);
		networkControlInfo->setProtocol(6);
		hipHead->setControlInfo(networkControlInfo);
        send(hipHead, "tcp6Out");
     }

// Overridden, RVS doesn't functions as a host
void RvsHIP::handleMsgFromTransport(cMessage *msg) { delete msg; }

// And hopefully doesn't changes IP addresses either
void RvsHIP::handleAddressChange() { return; }
