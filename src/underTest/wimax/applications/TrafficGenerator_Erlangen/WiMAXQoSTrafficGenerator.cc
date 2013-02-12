
//
// Copyright (C) 2006 Autonomic Networking Group,
// Department of Computer Science 7, University of Erlangen, Germany
//
// Author: Isabel Dietrich
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

#include "WiMAXQoSTrafficGenerator.h"
#include "IPv4Datagram.h"

Define_Module(WiMAXQoSTrafficGenerator);

/* 802.11 spezifisch für die sendTraffic methode
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "Ieee802Ctrl_m.h"
*/

void WiMAXQoSTrafficGenerator::initialize(int aStage)
{
    TrafGen::initialize(aStage);

    if (0 == aStage)
    {
        mLowerLayerIn = findGate("lowerLayerIn");
        mLowerLayerOut = findGate("lowerLayerOut");

        // if needed, change the current traffic pattern
        //mCurrentTrafficPattern = 0;
        // set parameters for the traffic generator
        //setParams(mCurrentTrafficPattern);

        mNumTrafficMsgs = 0;

        ev << this->getName();
        if (strcmp(this->getName(), "trafGen_ftp") == 0)
        {
            ipTrafficType = BE;
        }
        else if (strcmp(this->getName(), "trafGen_voice_no_supr") == 0)
        {
            ipTrafficType = UGS;
        }
        else if (strcmp(this->getName(), "trafGen_voice_supr") == 0)
        {
            ipTrafficType = ERTPS;
        }
        else if (strcmp(this->getName(), "trafGen_guaranteed_minbw_web_access") == 0)
        {
            ipTrafficType = NRTPS;
        }
        else if (strcmp(this->getName(), "trafGen_video_stream") == 0)
        {
            ipTrafficType = RTPS;
        }

        interDepartureTime = InterDepartureTime();
        packetSize = PacketSize();

//        string ugs_station = par("ugs_station");
//        if ( ugs_station.compare("") != 0 ) {
//         if ( ipTrafficType == ERTPS || ipTrafficType == NRTPS || ipTrafficType == RTPS )
//          cancelAndDelete(mpSendMessage);
//
//         if ( !(ugs_station.compare(parentModule()->name()) == 0) && ipTrafficType == UGS) {
//          cancelAndDelete(mpSendMessage);
//         }
//        }
    }

// srand( (unsigned)time(0) );
}

void WiMAXQoSTrafficGenerator::finish()
{
    recordScalar("trafficSent", mNumTrafficMsgs);
    recordScalar("Bitrate of generated traffic", (1 / interDepartureTime * packetSize) + 0.5);
}

void WiMAXQoSTrafficGenerator::handleLowerMsg(cPacket * apMsg)
{
    delete apMsg;
}

void WiMAXQoSTrafficGenerator::handleSelfMsg(cPacket * apMsg)
{
    TrafGen::handleSelfMsg(apMsg);
}

/** this function has to be redefined in every application derived from the
    TrafGen class.
    Its purpose is to translate the destination (given, for example, as "host[5]")
    to a valid address (MAC, IP, ...) that can be understood by the next lower
    layer.
    It also constructs an appropriate control info block that might be needed by
    the lower layer to process the message.
    In the example, the messages are sent directly to a mac 802.11 layer, address
    and control info are selected accordingly.
*/
void WiMAXQoSTrafficGenerator::SendTraf(cPacket * apMsg, const char *apDest)
{
    //apMsg->setLength(PacketSize()*8);
    apMsg->setByteLength(PacketSize()); // packet size in bits is more convenient

    IPv4Datagram *ip_d = new IPv4Datagram("ip_datagram");
    ip_d->encapsulate(apMsg);

    Ieee80216TGControlInformation *ci = new Ieee80216TGControlInformation();
    ci->setTraffic_type(ipTrafficType);
    ci->setBitrate((1 / interDepartureTime * packetSize) + 0.5);
    ci->setPacketInterval(interDepartureTime);

    ip_d->setControlInfo(ci);
    mNumTrafficMsgs++;
    send(ip_d, mLowerLayerOut);
}
