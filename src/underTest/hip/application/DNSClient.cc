//*********************************************************************************
// File:           DNSClient.cc
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


#include "DNSClient.h"
#include "IPvXAddressResolver.h"
#include "DNSBaseMsg_m.h"


Define_Module(DNSClient);


void DNSClient::initialize()
{
     socket.setOutputGate(gate("udpOut"));
     socket.bind(par("localPort").longValue());
}

void DNSClient::finish()
{
}

// Handles incoming DNS requests
void DNSClient::handleMessage(cPacket* msg)
{
    if (msg->isSelfMessage())
    {
        delete msg;
    }
    else if (dynamic_cast<DNSBaseMsg *>(msg)!= NULL)
    {
      if (msg->isName("DNS Request"))
    	  // Sending response
          socket.sendTo(msg, IPvXAddressResolver().resolve(par("dnsAddress")), 23);
      else
      {
         sendDirect(msg,this->getParentModule()->getSubmodule(par("tcpAppName"),par("tcpAppIndex")),"dns");
      }
    }
    else
    {
    	ev << "Packet not a DNSRequest, dropping...";
        delete msg;
    }
}


