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

#include "Ieee802Ctrl_m.h"
#include <fstream>
#include <iostream>
#include <iomanip>



void SecurityIeee80211MgmtBase::initialize(int stage)
{
    if (stage==0)
    {
        PassiveQueueBase::initialize();

        dataQueue.setName("wlanDataQueue");
        mgmtQueue.setName("wlanMgmtQueue");
        dataQueueLenSignal = registerSignal("dataQueueLen");
 //       dataArrivedSignal = registerSignal("dataArrived");
        emit(dataQueueLenSignal, dataQueue.length());
      //  dropPkByQueueSignal = registerSignal("dropPkByQueue");
        //emit(dropPkByQueueSignal, NULL);

        numDataFramesReceived = 0;
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
    //    numDataFramesDropped = 0;
        WATCH(numDataFramesReceived);
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);
      //  WATCH(numDataFramesDropped);
        // configuration
        frameCapacity = par("frameCapacity");
        // add by sbeiti
//        userQueueFilePath = "results\\queue\\";
//        currentSim = cSimulation::getActiveSimulation();
//        fileNameBegin = "test";
//        plotTime = 0;
//        plotTimer = 1;
        //end add
    }
    else if (stage==1)
    {
        // obtain our address from MAC
        cModule *mac = getParentModule()->getSubmodule("mac");
        if (!mac)
            error("MAC module not found; it is expected to be next to this submodule and called 'mac'");
        myAddress.setAddress(mac->par("address").stringValue());
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
        if(strstr(msg->getName() ,"CCMPFrame")!=NULL && hasSecurity)
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

    else if(strstr(msg->getName() ,"Beacon")!=NULL ||
                strstr(msg->getName() ,"Open Authentication Request")!=NULL || strstr(msg->getName() ,"Open Authentication Response")!=NULL ||
                strstr(msg->getName() ,"Auth")!=NULL || strstr(msg->getName() ,"Auth-OK")!=NULL || strstr(msg->getName() ,"Auth-ERROR")!=NULL ||
                strstr(msg->getName() ,"Auth msg 1/4")!=NULL || strstr(msg->getName() ,"Auth msg 2/4")!=NULL ||strstr(msg->getName() ,"Auth msg 3/4")!=NULL ||
                strstr(msg->getName() ,"Auth msg 4/4")!=NULL || strstr(msg->getName() , "Group msg 1/2")!=NULL || strstr(msg->getName() ,"Group msg 2/2")!=NULL )
        {
            sendOrEnqueue(PK(msg));
        }

    // else if (strstr(gateName,"securityIn")!=NULL && hasSecurity)
    else if (msg->arrivedOn("securityIn")&&hasSecurity)
    {
        if(strstr(msg->getName() ,"CCMPFrame")!=NULL)
        {
            EV << "CCMPFrame Frame arrived from Security, send it to Mac_" <<endl;
            sendOrEnqueue(PK(msg));
        }
        else if(strstr(msg->getName() ,"DecCCMP")!=NULL)
        {
            EV << "Frame arrived from Security, send it to upper layers: " << msg << "\n";
            Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
            processFrame(frame);
        }
        else
        {
            sendOrEnqueue(PK(msg));
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
            packetRequested++;
            send(msg, "securityOut");
        }
    }
    else
    {
        packetRequested++;
        send(msg, "macOut");
    }
}
