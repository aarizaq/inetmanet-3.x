/* -*- mode:c++ -*- ********************************************************
 * file:        CSMA.cc
 *
 * author:      Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *              Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 *
 * copyright:   (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *				(C) 2004,2005,2006
 *              Telecommunication Networks Group (TKN) at Technische
 *              Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/
#include "inet/linklayer/csma/CSMA802154.h"

#include <cassert>

#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/common/FindModule.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/linklayer/csma/CSMAFrame_m.h"

namespace inet {

Define_Module(CSMA802154);

void CSMA802154::initialize(int stage)
{
    MACProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        useMACAcks = par("useMACAcks").boolValue();
        queueLength = par("queueLength");
        sifs = par("sifs");
        transmissionAttemptInterruptedByRx = false;
        nbTxFrames = 0;
        nbRxFrames = 0;
        nbMissedAcks = 0;
        nbTxAcks = 0;
        nbRecvdAcks = 0;
        nbDroppedFrames = 0;
        nbDuplicates = 0;
        nbBackoffs = 0;
        backoffValues = 0;
        macMaxCSMABackoffs = par("macMaxCSMABackoffs");
        macMaxFrameRetries = par("macMaxFrameRetries");
        macAckWaitDuration = par("macAckWaitDuration").doubleValue();
        aUnitBackoffPeriod = par("aUnitBackoffPeriod").doubleValue();
        ccaDetectionTime = par("ccaDetectionTime").doubleValue();
        rxSetupTime = par("rxSetupTime").doubleValue();
        aTurnaroundTime = par("aTurnaroundTime").doubleValue();
        ackLength = par("ackLength");
        ackMessage = NULL;

        //init parameters for backoff method
        std::string backoffMethodStr = par("backoffMethod").stdstringValue();
        if (backoffMethodStr == "exponential") {
            backoffMethod = EXPONENTIAL;
            macMinBE = par("macMinBE");
            macMaxBE = par("macMaxBE");
        }
        else {
            if (backoffMethodStr == "linear") {
                backoffMethod = LINEAR;
            }
            else if (backoffMethodStr == "constant") {
                backoffMethod = CONSTANT;
            }
            else {
                error("Unknown backoff method \"%s\".\
					   Use \"constant\", \"linear\" or \"\
					   \"exponential\".", backoffMethodStr.c_str());
            }
            initialCW = par("contentionWindow");
        }
        NB = 0;

        // initialize the timers
        backoffTimer = new cMessage("timer-backoff");
        ccaTimer = new cMessage("timer-cca");
        sifsTimer = new cMessage("timer-sifs");
        rxAckTimer = new cMessage("timer-rxAck");
        macState = IDLE_1;
        txAttempts = 0;

        ITransmitter *tr = const_cast<ITransmitter *>(radio->getTransmitter());
        transmitter = dynamic_cast<Ieee802154UWBIRTransmitter *>(tr);

        cModule *radioModule = getParentModule()->getSubmodule("radio");
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

        //check parameters for consistency
        //aTurnaroundTime should match (be equal or bigger) the RX to TX
        //switching time of the radio
        if (radioModule->hasPar("timeRXToTX")) {
            simtime_t rxToTx = radioModule->par("timeRXToTX").doubleValue();
            if (rxToTx > aTurnaroundTime) {
                opp_warning("Parameter \"aTurnaroundTime\" (%f) does not match"
                            " the radios RX to TX switching time (%f)! It"
                            " should be equal or bigger",
                        SIMTIME_DBL(aTurnaroundTime), SIMTIME_DBL(rxToTx));
            }
        }
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        bitrate = transmitter->getRadioConfiguration().bitrate.get();

        initializeMACAddress();
        registerInterface();


        radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        EV_DETAIL << "queueLength = " << queueLength
                  << " bitrate = " << bitrate
                  << " backoff method = " << par("backoffMethod").stringValue() << endl;

        EV_DETAIL << "finished csma init stage 1." << endl;
    }
}


void CSMA802154::initializeMACAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr, "auto")) {
        // assign automatic address
        address = MACAddress::generateAutoAddress();
        address.convert64();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(address.str().c_str());
    }
    else {
        address.setAddress(addrstr);
    }
}

InterfaceEntry *CSMA802154::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // interface name: NIC module's name without special characters ([])
    e->setName(utils::stripnonalnum(getParentModule()->getFullName()).c_str());

    // data rate
    e->setDatarate(bitrate);

    // generate a link-layer address to be used as interface token for IPv6
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    // capabilities

    e->setMtu(transmitter->getRadioConfiguration().MaxPSDULength);
    e->setMulticast(false);
    e->setBroadcast(true);

    return e;
}


} // namespace inet

