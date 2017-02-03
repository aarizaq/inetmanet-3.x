//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#include "DcfUpperMac.h"
#include "Ieee80211Mac.h"
#include "IRx.h"
#include "IContention.h"
#include "ITx.h"
#include "MacUtils.h"
#include "MacParameters.h"
#include "FrameExchanges.h"
#include "DuplicateDetectors.h"
#include "IFragmentation.h"
#include "IRateSelection.h"
#include "IRateControl.h"
#include "IStatistics.h"
#include "inet/common/INETUtils.h"
#include "inet/common/queue/IPassiveQueue.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MsduAContainer.h"

namespace inet {
namespace ieee80211 {

Define_Module(DcfUpperMac);

DcfUpperMac::DcfUpperMac()
{
}

DcfUpperMac::~DcfUpperMac()
{
    delete frameExchange;
    delete duplicateDetection;
    delete fragmenter;
    delete reassembly;
    delete params;
    delete utils;
    delete [] contention;
}

void DcfUpperMac::initialize()
{
    mac = check_and_cast<Ieee80211Mac *>(getModuleByPath(par("macModule")));
    rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
    tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
    contention = nullptr;
    collectContentionModules(getModuleByPath(par("firstContentionModule")), contention);

    maxQueueSize = par("maxQueueSize");
    transmissionQueue.setName("txQueue");
    transmissionQueue.setup(par("prioritizeMulticast") ? (CompareFunc)MacUtils::cmpMgmtOverMulticastOverUnicast : (CompareFunc)MacUtils::cmpMgmtOverData);

    rateSelection = check_and_cast<IRateSelection*>(getModuleByPath(par("rateSelectionModule")));
    rateControl = dynamic_cast<IRateControl*>(getModuleByPath(par("rateControlModule"))); // optional module
    rateSelection->setRateControl(rateControl);

    params = extractParameters(rateSelection->getSlowestMandatoryMode());
    utils = new MacUtils(params, rateSelection);
    rx->setAddress(params->getAddress());

    statistics = check_and_cast<IStatistics*>(getModuleByPath(par("statisticsModule")));
    statistics->setMacUtils(utils);
    statistics->setRateControl(rateControl);

    duplicateDetection = new NonQoSDuplicateDetector(); //TODO or LegacyDuplicateDetector();
    fragmenter = check_and_cast<IFragmenter *>(inet::utils::createOne(par("fragmenterClass")));
    reassembly = check_and_cast<IReassembly *>(inet::utils::createOne(par("reassemblyClass")));

    WATCH(maxQueueSize);
    WATCH(fragmentationThreshold);
}

inline double fallback(double a, double b) {return a!=-1 ? a : b;}
inline simtime_t fallback(simtime_t a, simtime_t b) {return a!=-1 ? a : b;}

IMacParameters *DcfUpperMac::extractParameters(const IIeee80211Mode *slowestMandatoryMode)
{
    const IIeee80211Mode *referenceMode = slowestMandatoryMode;  // or any other; slotTime etc must be the same for all modes we use

    MacParameters *params = new MacParameters();
    params->setAddress(mac->getAddress());
    params->setShortRetryLimit(fallback(par("shortRetryLimit"), 7));
    params->setLongRetryLimit(fallback(par("longRetryLimit"), 4));
    params->setRtsThreshold(par("rtsThreshold"));
    params->setPhyRxStartDelay(referenceMode->getPhyRxStartDelay());
    params->setUseFullAckTimeout(par("useFullAckTimeout"));
    params->setEdcaEnabled(false);
    params->setSlotTime(fallback(par("slotTime"), referenceMode->getSlotTime()));
    params->setSifsTime(fallback(par("sifsTime"), referenceMode->getSifsTime()));
    int aCwMin = referenceMode->getLegacyCwMin();
    int aCwMax = referenceMode->getLegacyCwMax();
    params->setAifsTime(AC_LEGACY, fallback(par("difsTime"), referenceMode->getSifsTime() + MacUtils::getAifsNumber(AC_LEGACY) * params->getSlotTime()));
    params->setEifsTime(AC_LEGACY, params->getSifsTime() + params->getAifsTime(AC_LEGACY) + slowestMandatoryMode->getDuration(LENGTH_ACK));
    params->setCwMin(AC_LEGACY, fallback(par("cwMin"), MacUtils::getCwMin(AC_LEGACY, aCwMin)));
    params->setCwMax(AC_LEGACY, fallback(par("cwMax"), MacUtils::getCwMax(AC_LEGACY, aCwMax, aCwMin)));
    params->setCwMulticast(AC_LEGACY, fallback(par("cwMulticast"), MacUtils::getCwMin(AC_LEGACY, aCwMin)));
    return params;
}

void DcfUpperMac::handleMessage(cMessage *msg)
{
    if (msg->getContextPointer() != nullptr)
        ((MacPlugin *)msg->getContextPointer())->handleSelfMessage(msg);
    else
        ASSERT(false);
}

void DcfUpperMac::upperFrameReceived(Ieee80211DataOrMgmtFrame *frame)
{
    Enter_Method("upperFrameReceived(\"%s\")", frame->getName());
    take(frame);

    EV_INFO << "Frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    if (maxQueueSize > 0 && transmissionQueue.getLength() >= maxQueueSize && dynamic_cast<Ieee80211DataFrame *>(frame)) {
        EV << "Dataframe " << frame << " received from higher layer but MAC queue is full, dropping\n";
        delete frame;
        return;
    }

    ASSERT(!frame->getReceiverAddress().isUnspecified());
    frame->setTransmitterAddress(params->getAddress());
    duplicateDetection->assignSequenceNumber(frame);

    if (frame->getByteLength() <= fragmentationThreshold)
        enqueue(frame);
    else {
        auto fragments = fragmenter->fragment(frame, fragmentationThreshold);
        for (Ieee80211DataOrMgmtFrame *fragment : fragments)
            enqueue(fragment);
    }
}

void DcfUpperMac::enqueue(Ieee80211DataOrMgmtFrame *frame)
{
    statistics->upperFrameReceived(frame);
    if (frameExchange)
        createMsduA(frame);
        //transmissionQueue.insert(frame);
    else
        startSendDataFrameExchange(frame, 0, AC_LEGACY);
}


void DcfUpperMac::createMsduA(Ieee80211DataOrMgmtFrame * fr)
{
    if (!activeMsduA) {
        transmissionQueue.insert(fr);
        return;
    }
    Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(fr);
    if (frame == nullptr) {
        transmissionQueue.insert(fr);
        return;
    }

    if (frame->getReceiverAddress().isBroadcast() || frame->getAddress3().isBroadcast() || frame->getAddress4().isBroadcast() ||
            frame->getReceiverAddress().isMulticast() || frame->getAddress3().isMulticast() || frame->getAddress4().isMulticast()) {
        transmissionQueue.insert(frame);
        return;
    }
    Ieee80211MeshFrame *meshFrame = dynamic_cast<Ieee80211MeshFrame *>(frame);
    if (meshFrame && meshFrame->getSubType() != UPPERMESSAGE){ // mesh frame in transit
        transmissionQueue.insert(fr);
        return;
    }

    MACAddress dest = frame->getReceiverAddress();
    MACAddress addr3 = frame->getAddress3();
    MACAddress addr4 = frame->getAddress4();
    //Search same destination address
    std::vector<Ieee80211DataFrame *>elements;
    for (auto it = cQueue::Iterator(transmissionQueue);!it.end(); ++it) {

        Ieee80211DataFrame *frameAux = dynamic_cast<Ieee80211DataFrame *>(*it);
        MACAddress destAux = frameAux->getReceiverAddress();
        MACAddress addr3Aux = frameAux->getAddress3();
        MACAddress addr4Aux = frameAux->getAddress4();
        // Compare addresses
        if (destAux.compareTo(dest) == 0 && addr3Aux.compareTo(addr3) == 0 && addr4Aux.compareTo(addr4) == 0 && frameAux->getToDS() == frame->getToDS()) {
            elements.push_back(frameAux);
        }
    }

    if (elements.empty()) { // no available frames to the same destination
        transmissionQueue.insert(fr);
        return;
    }

    // if frames search for frames of the same type,
    // search frames to the same destination and characteristics
    int64_t maxSize = 7000;

    // search for msdu-a in the queues
    for (auto it = elements.begin() ;it != elements.end();)
    {
        Ieee80211MsduAContainer *frameMsda = dynamic_cast<Ieee80211MsduAContainer*>(*it); // check if msdu-a is present
        if (frameMsda == nullptr)
        {
            ++it;
            continue;
        }
        if (frameMsda->getByteLength() + frame->getByteLength() >= maxSize)
        {
            ++it;
            continue;
        }
        // check type
        Ieee80211MsduAMeshSubframe* msduMesh = dynamic_cast<Ieee80211MsduAMeshSubframe *>(frameMsda->getPacket(0));
        if ((meshFrame == nullptr) !=  (msduMesh == nullptr))
        {
            ++it;
            continue;
        }
        if (meshFrame && meshFrame->getSubType() != frameMsda->getSubType())
        {
            ++it;
            continue;
        }
        // add to the msdu-a
        // decapsulate and recapsulate
        // it is necessary to include padding bytes in the latest frame
        cPacket *pkt = frame->decapsulate();
        if (meshFrame)
        {
            Ieee80211MsduAMeshSubframe * header = new Ieee80211MsduAMeshSubframe();
            Ieee80211MeshFrame *aux = header;
            *aux = (*meshFrame);
            delete frame;
            aux->setByteLength(20);
            aux->encapsulate(pkt);
            frame = aux;
        }
        else
        {
            Ieee80211MsduASubframe * header = new Ieee80211MsduASubframe();
            if (dynamic_cast<Ieee80211DataFrameWithSNAP *>(frame))
                *((Ieee80211DataFrameWithSNAP *) header) = *((Ieee80211DataFrameWithSNAP*)frame);
            else
                *((Ieee80211DataFrame *) header) = *((Ieee80211DataFrame*)frame);
            Ieee80211DataFrame *aux = header;
            delete frame;
            aux->setByteLength(14);
            aux->encapsulate(pkt);
            frame = aux;
        }

        frameMsda->pushBack(frame);
        return;
    }


    // search for other frame and create an Msdu-a
    for (auto it = elements.begin() ;it != elements.end();)
    {
        // check if is a Msdu-A container
        if (dynamic_cast<Ieee80211MsduAContainer*>(*it))
        {
            it = elements.erase(it);
            continue;
        }
        // check if it is a MsduAframe
        if (dynamic_cast<Ieee80211MsduAMeshFrame *>(*it) || dynamic_cast<Ieee80211MsduAFrame *>(*it))
        {
            it = elements.erase(it);
            continue;
        }
        Ieee80211MeshFrame * frameAuxMesh = dynamic_cast<Ieee80211MeshFrame *>(*it);
        if (((meshFrame != nullptr) && (frameAuxMesh == nullptr)) || ((meshFrame == nullptr) && (frameAuxMesh != nullptr)))
        {
            it = elements.erase(it);
            continue;
        }
        if (meshFrame && meshFrame->getSubType() != frameAuxMesh->getSubType())
        {
            it = elements.erase(it);
            continue;
        }
        ++it;
    }

    if (elements.empty())    { // no available frames to the same destination
        transmissionQueue.insert(fr);
        return;
    }
    // create a new msdu
    Ieee80211MsduAContainer *msduaContainer = new Ieee80211MsduAContainer();
    // insert before
    transmissionQueue.insertBefore(elements.front(), msduaContainer);
    msduaContainer->setReceiverAddress(dest);
    msduaContainer->setAddress3(addr3);
    msduaContainer->setAddress4(addr4);
    msduaContainer->setTransmitterAddress(fr->getTransmitterAddress());
    cPacket *pkt = frame->decapsulate();
    if (meshFrame == nullptr)
    {
        // set byte length
        if (dynamic_cast<Ieee80211DataFrameWithSNAP *>(frame))
            *((Ieee80211DataFrameWithSNAP *) msduaContainer) = *((Ieee80211DataFrameWithSNAP*)frame);
        else
            *((Ieee80211DataFrame *) msduaContainer) = *((Ieee80211DataFrame*)frame);
        msduaContainer->setByteLength(DATAFRAME_HEADER_MINLENGTH / 8 + SNAP_HEADER_BYTES);
    }
    else
    {
        *((Ieee80211MeshFrame *) msduaContainer) = *meshFrame;
        msduaContainer->setByteLength(38);
    }

    if (meshFrame)
    {
        Ieee80211MsduAMeshSubframe * header = new Ieee80211MsduAMeshSubframe();
        Ieee80211MeshFrame *aux = header;
        *aux = (*meshFrame);
        delete frame;
        aux->setByteLength(20);
        msduaContainer->setSubType(header->getSubType());
        aux->encapsulate(pkt);
        frame = aux;
    }
    else
    {
        Ieee80211MsduASubframe * header = new Ieee80211MsduASubframe();
        if (dynamic_cast<Ieee80211DataFrameWithSNAP *>(frame))
            *((Ieee80211DataFrameWithSNAP *) header) = *((Ieee80211DataFrameWithSNAP*)frame);
        else
            *((Ieee80211DataFrame *) header) = *((Ieee80211DataFrame*)frame);
        Ieee80211DataFrame *aux = header;
        delete frame;
        aux->setByteLength(14);
        aux->encapsulate(pkt);
        frame = aux;
    }


    for (auto it = elements.begin() ;it != elements.end();)
    {
        transmissionQueue.remove(*it);
        Ieee80211DataFrame * frameAux = (Ieee80211DataFrame *)(*it);
        // check type
        Ieee80211MeshFrame * frameAuxMesh = dynamic_cast<Ieee80211MeshFrame *>(frameAux);
        cPacket *pkt = frameAux->decapsulate();
        if (frameAuxMesh)
        {
            Ieee80211MsduAMeshSubframe * header = new Ieee80211MsduAMeshSubframe();
            Ieee80211MeshFrame *aux = header;
            *aux = (*frameAuxMesh);
            aux->setByteLength(20);
            delete frameAux;
            aux->encapsulate(pkt);
            frameAux = aux;
        }
        else
        {
            Ieee80211MsduASubframe * header = new Ieee80211MsduASubframe();
            if (dynamic_cast<Ieee80211DataFrameWithSNAP *>(frameAux))
                *((Ieee80211DataFrameWithSNAP *) header) = *((Ieee80211DataFrameWithSNAP*)frameAux);
            else
                *((Ieee80211DataFrame *) header) = *((Ieee80211DataFrame*)frameAux);
            Ieee80211DataFrame *aux = header;
            aux->setByteLength(14);
            delete frameAux;
            aux->encapsulate(pkt);
            frameAux = aux;
        }
        // change size
        msduaContainer->pushBack(frameAux);
    }
    msduaContainer->pushBack(frame);
    return;
}

bool DcfUpperMac::isMsduA(Ieee80211DataOrMgmtFrame * frame)
{
    return dynamic_cast<Ieee80211MsduAContainer*>(frame) != nullptr;
}

void DcfUpperMac::getMsduAFrames(Ieee80211DataOrMgmtFrame * frame, std::vector<Ieee80211DataOrMgmtFrame *>&frames)
{

    frames.clear();
    if (!isMsduA(frame))
        return;

    Ieee80211MsduAContainer *frameMsda = dynamic_cast<Ieee80211MsduAContainer*>(frame);
    while (frameMsda->haveBlock())
    {
        Ieee80211DataFrame * subframe = frameMsda->popFrom();
        Ieee80211MsduASubframe * header

    }
}


void DcfUpperMac::lowerFrameReceived(Ieee80211Frame *frame)
{
    Enter_Method("lowerFrameReceived(\"%s\")", frame->getName());
    delete frame->removeControlInfo();          //TODO
    take(frame);

    if (!utils->isForUs(frame)) {
        EV_INFO << "This frame is not for us" << std::endl;
        delete frame;
        if (frameExchange)
            frameExchange->corruptedOrNotForUsFrameReceived();
    }
    else {
        // offer frame to ongoing frame exchange
        IFrameExchange::FrameProcessingResult result = frameExchange ? frameExchange->lowerFrameReceived(frame) : IFrameExchange::IGNORED;
        bool processed = (result != IFrameExchange::IGNORED);
        if (processed) {
            // already processed, nothing more to do
            if (result == IFrameExchange::PROCESSED_DISCARD)
                delete frame;
        }
        else if (Ieee80211RTSFrame *rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(frame)) {
            sendCts(rtsFrame);
            delete rtsFrame;
        }
        else if (Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
            if (!utils->isBroadcastOrMulticast(frame))
                sendAck(dataOrMgmtFrame);
            if (duplicateDetection->isDuplicate(dataOrMgmtFrame)) {
                EV_INFO << "Duplicate frame " << frame->getName() << ", dropping\n";
                delete dataOrMgmtFrame;
            }
            else {
                if (!utils->isFragment(dataOrMgmtFrame))
                    mac->sendUp(dataOrMgmtFrame);
                else {
                    Ieee80211DataOrMgmtFrame *completeFrame = reassembly->addFragment(dataOrMgmtFrame);
                    if (completeFrame)
                        mac->sendUp(completeFrame);
                }
            }
        }
        else {
            EV_INFO << "Unexpected frame " << frame->getName() << ", dropping\n";
            delete frame;
        }
    }
}

void DcfUpperMac::corruptedFrameReceived()
{
    if (frameExchange)
        frameExchange->corruptedOrNotForUsFrameReceived();
}

void DcfUpperMac::channelAccessGranted(IContentionCallback *callback, int txIndex)
{
    Enter_Method("channelAccessGranted()");
    callback->channelAccessGranted(txIndex);
}

void DcfUpperMac::internalCollision(IContentionCallback *callback, int txIndex)
{
    Enter_Method("internalCollision()");
    if (callback)
        callback->internalCollision(txIndex);
}

void DcfUpperMac::transmissionComplete(ITxCallback *callback)
{
    Enter_Method("transmissionComplete()");
    if (callback)
        callback->transmissionComplete();
}

void DcfUpperMac::startSendDataFrameExchange(Ieee80211DataOrMgmtFrame *frame, int txIndex, AccessCategory ac)
{
    ASSERT(!frameExchange);

    if (utils->isBroadcastOrMulticast(frame))
        utils->setFrameMode(frame, rateSelection->getModeForMulticastDataOrMgmtFrame(frame));
    else
        utils->setFrameMode(frame, rateSelection->getModeForUnicastDataOrMgmtFrame(frame));

    FrameExchangeContext context;
    context.ownerModule = this;
    context.params = params;
    context.utils = utils;
    context.contention = contention;
    context.tx = tx;
    context.rx = rx;
    context.statistics = statistics;

    bool useRtsCts = frame->getByteLength() > params->getRtsThreshold();
    if (utils->isBroadcastOrMulticast(frame))
        frameExchange = new SendMulticastDataFrameExchange(&context, this, frame, txIndex, ac);
    else if (useRtsCts)
        frameExchange = new SendDataWithRtsCtsFrameExchange(&context, this, frame, txIndex, ac);
    else
        frameExchange = new SendDataWithAckFrameExchange(&context, this, frame, txIndex, ac);

    frameExchange->start();
}

void DcfUpperMac::frameExchangeFinished(IFrameExchange *what, bool successful)
{
    EV_INFO << "Frame exchange finished" << std::endl;
    delete frameExchange;
    frameExchange = nullptr;

    if (!transmissionQueue.isEmpty()) {
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(transmissionQueue.pop());
        startSendDataFrameExchange(frame, 0, AC_LEGACY);
    }
}

void DcfUpperMac::sendAck(Ieee80211DataOrMgmtFrame *frame)
{
    Ieee80211ACKFrame *ackFrame = utils->buildAckFrame(frame);
    tx->transmitFrame(ackFrame, params->getSifsTime(), nullptr);
}

void DcfUpperMac::sendCts(Ieee80211RTSFrame *frame)
{
    Ieee80211CTSFrame *ctsFrame = utils->buildCtsFrame(frame);
    tx->transmitFrame(ctsFrame, params->getSifsTime(), nullptr);
}

} // namespace ieee80211
} // namespace inet

