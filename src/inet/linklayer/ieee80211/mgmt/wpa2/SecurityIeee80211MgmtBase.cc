//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "SecurityIeee80211MgmtBase.h"

#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace inet {

namespace ieee80211 {



void SecurityIeee80211MgmtBase::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);

    if (stage == 1)
    {
        // add by sbeiti
//        if(plotTimer > 0){
//                   plotTime = new cMessage("itsPlotTime");
//                   scheduleAt(simTime() + plotTimer, plotTime);
//               }
        //end add
    }
}

void SecurityIeee80211MgmtBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
    }
    else if (msg->arrivedOn("macIn"))
    {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";

        // if verschlüsselt
        if(strstr(msg->getName() ,"CCMPFrame")!=nullptr && hasSecurity)
        {
            EV << "CCMPFrame Frame arrived from MAC, send it to SecurityModule" << msg << "\n";
            send(msg, "securityOut");
        }
        else
        {
            Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
            processFrame(frame);
        }
    }

    /*--------From upperLayer----------*/
    else if (msg->arrivedOn("agentIn"))
    {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cObject *ctrl = msg->removeControlInfo();
        delete msg;

        handleCommand(msgkind, ctrl);
    }

    else if(strstr(msg->getName() ,"Beacon")!=nullptr ||
                strstr(msg->getName() ,"Open Authentication Request")!=nullptr || strstr(msg->getName() ,"Open Authentication Response")!=nullptr ||
                strstr(msg->getName() ,"Auth")!=nullptr || strstr(msg->getName() ,"Auth-OK")!=nullptr || strstr(msg->getName() ,"Auth-ERROR")!=nullptr ||
                strstr(msg->getName() ,"Auth msg 1/4")!=nullptr || strstr(msg->getName() ,"Auth msg 2/4")!=nullptr ||strstr(msg->getName() ,"Auth msg 3/4")!=nullptr ||
                strstr(msg->getName() ,"Auth msg 4/4")!=nullptr || strstr(msg->getName() , "Group msg 1/2")!=nullptr || strstr(msg->getName() ,"Group msg 2/2")!=nullptr )
        {
            sendDown(PK(msg));
        }

    // else if (strstr(gateName,"securityIn")!=nullptr && hasSecurity)
    else if (msg->arrivedOn("securityIn")&&hasSecurity)
    {
        if(strstr(msg->getName() ,"CCMPFrame")!=nullptr)
        {
            EV << "CCMPFrame Frame arrived from Security, send it to Mac_" <<endl;
            sendDown(PK(msg));
        }
        else if(strstr(msg->getName() ,"DecCCMP")!=nullptr)
        {
            EV << "Frame arrived from Security, send it to upper layers: " << msg << "\n";
            Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
            processFrame(frame);
        }
        else
        {
            sendDown(PK(msg));
        }
    }

    else
    {
        // packet from upper layers, to be sent out
        cPacket *pk = PK(msg);
        EV << "SecuriryIeeeBase: Packet arrived from upper layers: " << pk << "\n";
        if (pk->getByteLength() > 2312)
            error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                  pk->getClassName(), pk->getName(), (int)(pk->getByteLength()));

        handleUpperMessage(pk);
    }
}

void SecurityIeee80211MgmtBase::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    EV << "Authentication from MAC, send it to SecurityModule" << frame << "\n";
    send(frame, "securityOut");
}
void SecurityIeee80211MgmtBase::handleCCMPFrame(CCMPFrame *frame)
{
    EV << "CCMP Frame from MAC, send it to SecurityModule" << frame << "\n";
    send(frame, "securityOut");
}

void SecurityIeee80211MgmtBase::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    EV << "Beacon from MAC, send it to SecurityModule" << frame << "\n";
    send(frame, "securityOut");
}


void SecurityIeee80211MgmtBase::sendOut(cMessage *msg)
{
    EV << "SecurityIeee80211MgmtBase:: sendOut" <<endl;
    msg->setKind(0);
    //mhn
    if(hasSecurity)
    {
        if (msg->arrivedOn("securityIn"))
        {
            send(msg, "macOut",msg->getKind());
        }
        else  if (dynamic_cast<CCMPFrame *>(msg))
        {
            EV << "CCMPFrame Frame arrived from Security, send it to Mac" <<endl;
            error("mhn");
            send(msg, "macOut",msg->getKind());
        }
        else
        {
            send(msg, "securityOut");
        }
    }
    else
    {
        send(msg, "macOut");
    }
}

}

}
