//
// Copyright (C) 2013, NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//
// This file is part of NETA.
//
//    NETA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NETA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NETA.  If not, see <http://www.gnu.org/licenses/>.
//


#include "NA_UDPBasicBurst.h"

#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"



using std::cout;

namespace inet {

namespace neta {

Define_Module(NA_UDPBasicBurst);

simsignal_t NA_UDPBasicBurst::hopCountSignal = registerSignal("hopCount");

void NA_UDPBasicBurst::initialize(int stage)
{
    // because of IPvXAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != 3)
        return;

    if (stage == INITSTAGE_LOCAL) {
        numHopsTotal = 0;
        avHopCount = 0;
    }

    UDPBasicBurst::initialize(stage);
}


void NA_UDPBasicBurst::processPacket(cPacket *pk)
{

    if (pk->getKind() == UDP_I_ERROR) {
        EV_WARN << "UDP error received\n";
        delete pk;
        return;
    }

    if (pk->hasPar("sourceId") && pk->hasPar("msgId")) {
        // duplicate control
        int moduleId = (int)pk->par("sourceId");
        int msgId = (int)pk->par("msgId");
        auto it = sourceSequence.find(moduleId);
        if (it != sourceSequence.end()) {
            if (it->second >= msgId) {
                EV_DEBUG << "Out of order packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
                emit(outOfOrderPkSignal, pk);
                delete pk;
                numDuplicated++;
                return;
            }
            else
                it->second = msgId;
        }
        else
            sourceSequence[moduleId] = msgId;
    }

    if (delayLimit > 0) {
        if (simTime() - pk->getTimestamp() > delayLimit) {
            EV_DEBUG << "Old packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
            emit(dropPkSignal, pk);
            delete pk;
            numDeleted++;
            return;
        }
    }

    EV_INFO << "Received packet: " << UDPSocket::getReceivedPacketInfo(pk) << endl;
    emit(rcvdPkSignal, pk);
    numReceived++;
    totalPkRec++;

    cObject *ctrl = pk->getControlInfo();
    if (dynamic_cast<UDPDataIndication *>(ctrl)!=NULL) {
        UDPDataIndication *udpCtrl = (UDPDataIndication *)ctrl;
        short hopCnt = 32 - udpCtrl->getTtl(); // travelled hops of IP packet

        numHopsTotal = numHopsTotal + hopCnt;
        if ( numReceived > 0 )
            avHopCount = (double)numHopsTotal/(double)numReceived;
        else
            avHopCount = 0;

        //cout << "Hop Count = " << hopCnt << endl;
        //cout << "Num Hop total = " << numHopsTotal << endl;
        //cout << "Average Hop Count = " << numHopsTotal << " / " << numReceived << " = "<< avHopCount << endl << endl;

        emit(hopCountSignal, avHopCount);

    }

    delete pk;

}

}
}
