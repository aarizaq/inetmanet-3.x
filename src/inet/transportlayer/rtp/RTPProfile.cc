/***************************************************************************
                          RTPProfile.cc  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <string.h>

#include "inet/transportlayer/rtp/RTPProfile.h"

#include "inet/transportlayer/rtp/RTPInnerPacket.h"
#include "inet/transportlayer/rtp/RTPPayloadReceiver.h"
#include "inet/transportlayer/rtp/RTPPayloadSender.h"

namespace inet {

namespace rtp {

Define_Module(RTPProfile);

RTPProfile::RTPProfile()
{
}

void RTPProfile::initialize()
{
    EV_TRACE << "initialize() Enter" << endl;
    _profileName = "Profile";
    _rtcpPercentage = 5;
    _preferredPort = PORT_UNDEF;

    // how many gates to payload receivers do we have
    _maxReceivers = gateSize("payloadReceiverOut");
    _ssrcGates.clear();
    _autoOutputFileNames = par("autoOutputFileNames").boolValue();
    EV_TRACE << "initialize() Exit" << endl;
}

RTPProfile::~RTPProfile()
{
    for (auto & elem : _ssrcGates)
        delete elem.second;
}

void RTPProfile::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGateId() == findGate("rtpIn")) {
        handleMessageFromRTP(msg);
    }
    else if (msg->getArrivalGateId() == findGate("payloadSenderIn")) {
        handleMessageFromPayloadSender(msg);
    }
    else if (msg->getArrivalGateId() >= findGate("payloadReceiverIn")
             && msg->getArrivalGateId() < findGate("payloadReceiverIn") + _maxReceivers)
    {
        handleMessageFromPayloadReceiver(msg);
    }
    else {
        throw cRuntimeError("message coming from unknown gate");
    }
}

void RTPProfile::handleMessageFromRTP(cMessage *msg)
{
    EV_TRACE << "handleMessageFromRTP Enter " << endl;

    RTPInnerPacket *rinpIn = check_and_cast<RTPInnerPacket *>(msg);

    switch (rinpIn->getType()) {
        case RTP_INP_INITIALIZE_PROFILE:
            initializeProfile(rinpIn);
            break;

        case RTP_INP_CREATE_SENDER_MODULE:
            createSenderModule(rinpIn);
            break;

        case RTP_INP_DELETE_SENDER_MODULE:
            deleteSenderModule(rinpIn);
            break;

        case RTP_INP_SENDER_MODULE_CONTROL:
            senderModuleControl(rinpIn);
            break;

        case RTP_INP_DATA_IN:
            dataIn(rinpIn);
            break;

        default:
            throw cRuntimeError("RTPInnerPacket from RTPModule has wrong type: %d", rinpIn->getType());
            break;
    }

    EV_TRACE << "handleMessageFromRTP Exit " << endl;
}

void RTPProfile::handleMessageFromPayloadSender(cMessage *msg)
{
    RTPInnerPacket *rinpIn = check_and_cast<RTPInnerPacket *>(msg);

    switch (rinpIn->getType()) {
        case RTP_INP_DATA_OUT:
            dataOut(rinpIn);
            break;

        case RTP_INP_SENDER_MODULE_INITIALIZED:
            senderModuleInitialized(rinpIn);
            break;

        case RTP_INP_SENDER_MODULE_STATUS:
            senderModuleStatus(rinpIn);
            break;

        default:
            throw cRuntimeError("Profile received RTPInnerPacket from sender module with wrong type: %d", rinpIn->getType());
            break;
    }
}

void RTPProfile::handleMessageFromPayloadReceiver(cMessage *msg)
{
    // currently payload receiver modules don't send messages
    delete msg;
}

void RTPProfile::initializeProfile(RTPInnerPacket *rinp)
{
    EV_TRACE << "initializeProfile Enter" << endl;
    _mtu = rinp->getMTU();
    delete rinp;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("profileInitialized()");
    rinpOut->setProfileInitializedPkt(_rtcpPercentage, _preferredPort);
    send(rinpOut, "rtpOut");
    EV_TRACE << "initializeProfile Exit" << endl;
}

void RTPProfile::createSenderModule(RTPInnerPacket *rinp)
{
    EV_TRACE << "createSenderModule Enter" << endl;
    int ssrc = rinp->getSsrc();
    int payloadType = rinp->getPayloadType();

    EV_INFO << "ProfileName: " << _profileName << " payloadType: " << payloadType << endl;
    std::string moduleTypeName(std::string("inet.transportlayer.rtp.RTP") + _profileName + "Payload" + std::to_string(payloadType) + "Sender");
    std::string moduleName(std::string("rtp") + _profileName + "Payload" + std::to_string(payloadType) + "Sender");

    cModuleType *moduleType = cModuleType::find(moduleTypeName.c_str());
    if (moduleType == nullptr)
        throw cRuntimeError("RTPProfile: payload sender module '%s' not found", moduleTypeName.c_str());

    RTPPayloadSender *rtpPayloadSender = check_and_cast<RTPPayloadSender *>(moduleType->create(moduleName.c_str(), this));
    rtpPayloadSender->finalizeParameters();

    gate("payloadSenderOut")->connectTo(rtpPayloadSender->gate("profileIn"));
    rtpPayloadSender->gate("profileOut")->connectTo(gate("payloadSenderIn"));

    rtpPayloadSender->callInitialize();
    rtpPayloadSender->scheduleStart(simTime());

    RTPInnerPacket *rinpOut1 = new RTPInnerPacket("senderModuleCreated()");
    rinpOut1->setSenderModuleCreatedPkt(ssrc);
    send(rinpOut1, "rtpOut");

    RTPInnerPacket *rinpOut2 = new RTPInnerPacket("initializeSenderModule()");
    rinpOut2->setInitializeSenderModulePkt(ssrc, rinp->getFileName(), _mtu);
    send(rinpOut2, "payloadSenderOut");

    delete rinp;
    EV_TRACE << "createSenderModule Exit" << endl;
}

void RTPProfile::deleteSenderModule(RTPInnerPacket *rinpIn)
{
    cModule *senderModule = gate("payloadSenderOut")->getNextGate()->getOwnerModule();
    senderModule->deleteModule();

    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleDeleted()");
    rinpOut->setSenderModuleDeletedPkt(rinpIn->getSsrc());
    delete rinpIn;

    send(rinpOut, "rtpOut");
}

void RTPProfile::senderModuleControl(RTPInnerPacket *rinp)
{
    send(rinp, "payloadSenderOut");
}

void RTPProfile::dataIn(RTPInnerPacket *rinp)
{
    EV_TRACE << "dataIn(RTPInnerPacket *rinp) Enter" << endl;
    processIncomingPacket(rinp);

    RTPPacket *packet = check_and_cast<RTPPacket *>(rinp->getEncapsulatedPacket());

    uint32 ssrc = packet->getSsrc();

    SSRCGate *ssrcGate = findSSRCGate(ssrc);

    if (!ssrcGate) {
        ssrcGate = newSSRCGate(ssrc);
        std::string payloadReceiverTypeName(std::string("inet.transportlayer.rtp.RTP") + _profileName + "Payload" + std::to_string(packet->getPayloadType()) + "Receiver");
        std::string payloadReceiverName(std::string("rtp") + _profileName + "Payload" + std::to_string(packet->getPayloadType()) + "Receiver" + std::to_string(ssrc));
        cModuleType *moduleType = cModuleType::find(payloadReceiverTypeName.c_str());
        if (moduleType == nullptr)
            throw cRuntimeError("Receiver module type %s not found", payloadReceiverName.c_str());
        else {
            RTPPayloadReceiver *receiverModule = check_and_cast<RTPPayloadReceiver *>(moduleType->create(payloadReceiverName.c_str(), this));
            if (_autoOutputFileNames) {
                char outputFileName[100];
                sprintf(outputFileName, "id%i.sim", receiverModule->getId());
                receiverModule->par("outputFileName") = outputFileName;
            }
            receiverModule->finalizeParameters();

            this->gate(ssrcGate->getGateId())->connectTo(receiverModule->gate("profileIn"));
            receiverModule->gate("profileOut")->connectTo(this->gate(ssrcGate->getGateId()
                            - findGate("payloadReceiverOut", 0) + findGate("payloadReceiverIn", 0)));

            for (int i = 0; receiverModule->callInitialize(i); i++)
                ;

            receiverModule->scheduleStart(simTime());
        }
    }

    send(rinp, ssrcGate->getGateId());
    EV_TRACE << "dataIn(RTPInnerPacket *rinp) Exit" << endl;
}

void RTPProfile::dataOut(RTPInnerPacket *rinp)
{
    processOutgoingPacket(rinp);
    send(rinp, "rtpOut");
}

void RTPProfile::senderModuleInitialized(RTPInnerPacket *rinp)
{
    EV_TRACE << "senderModuleInitialized" << endl;
    send(rinp, "rtpOut");
}

void RTPProfile::senderModuleStatus(RTPInnerPacket *rinp)
{
    EV_TRACE << "senderModuleStatus" << endl;
    send(rinp, "rtpOut");
}

void RTPProfile::processIncomingPacket(RTPInnerPacket *rinp)
{
    // do nothing with the packet
}

void RTPProfile::processOutgoingPacket(RTPInnerPacket *rinp)
{
    // do nothing with the packet
}

RTPProfile::SSRCGate *RTPProfile::findSSRCGate(uint32 ssrc)
{
    auto objectIndex = _ssrcGates.find(ssrc);
    if (objectIndex == _ssrcGates.end())
        return nullptr;
    return objectIndex->second;
}

RTPProfile::SSRCGate *RTPProfile::newSSRCGate(uint32 ssrc)
{
    SSRCGate *ssrcGate = new SSRCGate(ssrc);
    bool assigned = false;
    int receiverGateId = findGate("payloadReceiverOut", 0);
    for (int i = receiverGateId; i < receiverGateId + _maxReceivers && !assigned; i++) {
        if (!gate(i)->isConnected()) {
            ssrcGate->setGateId(i);
            assigned = true;
        }
    }

    if (!assigned)
        throw cRuntimeError("Can't manage more senders");

    _ssrcGates[ssrc] = ssrcGate;
    return ssrcGate;
}

} // namespace rtp

} // namespace inet

