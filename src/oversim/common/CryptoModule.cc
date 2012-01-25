//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file CryptoModule.cc
 * @author Ingmar Baumgart
 */


#include <CommonMessages_m.h>
#include <OverlayAccess.h>
#include <GlobalStatisticsAccess.h>
#include <CryptoModule.h>

using namespace std;

Define_Module(CryptoModule);

CryptoModule::CryptoModule()
{
    globalStatistics = NULL;
    overlay = NULL;
}

CryptoModule::~CryptoModule()
{
}

void CryptoModule::initialize()
{
    globalStatistics = GlobalStatisticsAccess().get();
    overlay = OverlayAccess().get(this);

    numSign = 0;

//    EV << "[CryptoModule::initialize() @ " << overlay->getThisNode().getIp()
//       << " (" << overlay->getThisNode().getKey().toString(16) << ")]\n"
//       << "    Reading key from file " << par("keyFile").stdstringValue()
//       << endl;
}

void CryptoModule::signMessage(BaseRpcMessage *msg)
{
    // need to remove controlInfo before serializing
    BaseRpcMessage *msgStripped = static_cast<BaseRpcMessage*>(msg->dup());

    if (msgStripped->getControlInfo() != NULL) {
            delete msgStripped->removeControlInfo();
    }

    // serialize message (needed to calculate message hash)
    commBuffer.reset();
    commBuffer.packObject(msgStripped);
    delete msgStripped;

    // calculate hash and signature
    // commBuffer.getBuffer(), commBuffer.getBufferLength()

    // ...

    // append public key and signature
    msg->setAuthBlockArraySize(1);
    msg->getAuthBlock(0).setPubKey(BinaryValue("123"));
    msg->getAuthBlock(0).setSignature(BinaryValue("456"));
    msg->getAuthBlock(0).setCert(BinaryValue("789"));

    // record statistics
    RECORD_STATS(numSign++);
}

bool CryptoModule::verifyMessage(BaseRpcMessage *msg)
{
    if (msg->getAuthBlockArraySize() == 0) {
        // message contains no signature
        return false;
    }

    // need to remove controlInfo before serializing
    BaseRpcMessage *msgStripped = static_cast<BaseRpcMessage*>(msg->dup());

    if (msgStripped->getControlInfo() != NULL) {
            delete msgStripped->removeControlInfo();
    }

    // serialize message (needed to calculate message hash)
    commBuffer.reset();
    commBuffer.packObject(msgStripped);
    delete msgStripped;

    // calculate hash and signature
    commBuffer.getBuffer();

    //const BinaryValue& pubKey = msg->getAuthBlock(0).getPubKey();
    //const BinaryValue& signature = msg->getAuthBlock(0).getSignature();
    //const BinaryValue& cert = msg->getAuthBlock(0).getCert();

    //...

    return true;
}

void CryptoModule::handleMessage(cMessage *msg)
{
    delete msg; // just discard everything we receive
}

void CryptoModule::finish()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(
                             overlay->getCreationTime());

    if (time >= GlobalStatistics::MIN_MEASURED) {
        if (numSign > 0) {
            globalStatistics->addStdDev("CryptoModule: Sign Operations/s",
                                        numSign / time);
        }
    }
}

