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

#include "MacUtils.h"
#include "IMacParameters.h"
#include "IRateSelection.h"
#include "inet/linklayer/ieee80211/oldmac/Ieee80211Consts.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211MsduAContainer.h"

namespace inet {
namespace ieee80211 {

MacUtils::MacUtils(IMacParameters *params, IRateSelection *rateSelection) : params(params), rateSelection(rateSelection)
{
}

simtime_t MacUtils::getAckDuration() const
{
    return rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK);
}

simtime_t MacUtils::getCtsDuration() const
{
    return rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_CTS);
}

simtime_t MacUtils::getAckEarlyTimeout() const
{
    // Note: This excludes ACK duration. If there's no RXStart indication within this interval, retransmission should begin immediately
    return params->getSifsTime() + params->getSlotTime() + params->getPhyRxStartDelay();
}

simtime_t MacUtils::getAckFullTimeout() const
{
    return params->getSifsTime() + params->getSlotTime() + getAckDuration();
}

simtime_t MacUtils::getCtsEarlyTimeout() const
{
    return params->getSifsTime() + params->getSlotTime() + params->getPhyRxStartDelay(); // see getAckEarlyTimeout()
}

simtime_t MacUtils::getCtsFullTimeout() const
{
    return params->getSifsTime() + params->getSlotTime() + getCtsDuration();
}

Ieee80211RTSFrame *MacUtils::buildRtsFrame(Ieee80211DataOrMgmtFrame *dataFrame) const
{
    return buildRtsFrame(dataFrame, getFrameMode(dataFrame));
}

Ieee80211RTSFrame *MacUtils::buildRtsFrame(Ieee80211DataOrMgmtFrame *dataFrame, const IIeee80211Mode *dataFrameMode) const
{
    // protect CTS + Data + ACK
    simtime_t duration =
            3 * params->getSifsTime()
            + rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_CTS)
            + dataFrameMode->getDuration(dataFrame->getBitLength())
            + rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK);
    return buildRtsFrame(dataFrame->getReceiverAddress(), duration);
}

Ieee80211RTSFrame *MacUtils::buildRtsFrame(const MACAddress& receiverAddress, simtime_t duration) const
{
    Ieee80211RTSFrame *rtsFrame = new Ieee80211RTSFrame("RTS");
    rtsFrame->setTransmitterAddress(params->getAddress());
    rtsFrame->setReceiverAddress(receiverAddress);
    rtsFrame->setDuration(duration);
    setFrameMode(rtsFrame, rateSelection->getModeForControlFrame(rtsFrame));
    return rtsFrame;
}

Ieee80211CTSFrame *MacUtils::buildCtsFrame(Ieee80211RTSFrame *rtsFrame) const
{
    Ieee80211CTSFrame *frame = new Ieee80211CTSFrame("CTS");
    frame->setReceiverAddress(rtsFrame->getTransmitterAddress());
    frame->setDuration(rtsFrame->getDuration() - params->getSifsTime() - rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_CTS));
    setFrameMode(rtsFrame, rateSelection->getModeForControlFrame(rtsFrame));
    return frame;
}

Ieee80211ACKFrame *MacUtils::buildAckFrame(Ieee80211DataOrMgmtFrame *dataFrame) const
{
    Ieee80211ACKFrame *ackFrame = new Ieee80211ACKFrame("ACK");
    ackFrame->setReceiverAddress(dataFrame->getTransmitterAddress());

    if (!dataFrame->getMoreFragments())
        ackFrame->setDuration(0);
    else
        ackFrame->setDuration(dataFrame->getDuration() - params->getSifsTime() - rateSelection->getResponseControlFrameMode()->getDuration(LENGTH_ACK));
    setFrameMode(ackFrame, rateSelection->getModeForControlFrame(ackFrame));
    return ackFrame;
}

Ieee80211Frame *MacUtils::setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const
{
    ASSERT(frame->getControlInfo() == nullptr);
    Ieee80211TransmissionRequest *ctrl = new Ieee80211TransmissionRequest();
    ctrl->setMode(mode);
    frame->setControlInfo(ctrl);
    return frame;
}

const IIeee80211Mode *MacUtils::getFrameMode(Ieee80211Frame *frame) const
{
    Ieee80211TransmissionRequest *ctrl = check_and_cast<Ieee80211TransmissionRequest*>(frame->getControlInfo());
    return ctrl->getMode();
}

bool MacUtils::isForUs(Ieee80211Frame *frame) const
{
    return frame->getReceiverAddress() == params->getAddress() || (frame->getReceiverAddress().isMulticast() && !isSentByUs(frame));
}

bool MacUtils::isSentByUs(Ieee80211Frame *frame) const
{
    if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame))
        return dataOrMgmtFrame->getAddress3() == params->getAddress();
    else
        return false;
}


bool MacUtils::isBroadcastOrMulticast(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress().isMulticast();  // also true for broadcast frames
}

bool MacUtils::isBroadcast(Ieee80211Frame *frame) const
{
    return frame && frame->getReceiverAddress().isBroadcast();
}

bool MacUtils::isFragment(Ieee80211DataOrMgmtFrame *frame) const
{
    return frame->getFragmentNumber() != 0 || frame->getMoreFragments() == true;
}

bool MacUtils::isCts(Ieee80211Frame *frame) const
{
    return dynamic_cast<Ieee80211CTSFrame *>(frame);
}

bool MacUtils::isAck(Ieee80211Frame *frame) const
{
    return dynamic_cast<Ieee80211ACKFrame *>(frame);
}

simtime_t MacUtils::getTxopLimit(AccessCategory ac, const IIeee80211Mode *mode)
{
    switch (ac)
    {
        case AC_BK: return 0;
        case AC_BE: return 0;
        case AC_VI:
            if (dynamic_cast<const Ieee80211DsssMode*>(mode) || dynamic_cast<const Ieee80211HrDsssMode*>(mode)) return ms(6.016).get();
            else if (dynamic_cast<const Ieee80211HTMode*>(mode) || dynamic_cast<const Ieee80211OFDMMode*>(mode)) return ms(3.008).get();
            else return 0;
        case AC_VO:
            if (dynamic_cast<const Ieee80211DsssMode*>(mode) || dynamic_cast<const Ieee80211HrDsssMode*>(mode)) return ms(3.264).get();
            else if (dynamic_cast<const Ieee80211HTMode*>(mode) || dynamic_cast<const Ieee80211OFDMMode*>(mode)) return ms(1.504).get();
            else return 0;
        case AC_LEGACY: return 0;
        case AC_NUMCATEGORIES: break;
    }
    throw cRuntimeError("Unknown access category = %d", ac);
    return 0;
}

int MacUtils::getAifsNumber(AccessCategory ac)
{
    switch (ac)
    {
        case AC_BK: return 7;
        case AC_BE: return 3;
        case AC_VI: return 2;
        case AC_VO: return 2;
        case AC_LEGACY: return 2;
        case AC_NUMCATEGORIES: break;
    }
    throw cRuntimeError("Unknown access category = %d", ac);
    return -1;
}

int MacUtils::getCwMax(AccessCategory ac, int aCwMax, int aCwMin)
{
    switch (ac)
    {
        case AC_BK: return aCwMax;
        case AC_BE: return aCwMax;
        case AC_VI: return aCwMin;
        case AC_VO: return (aCwMin + 1) / 2 - 1;
        case AC_LEGACY: return aCwMax;
        case AC_NUMCATEGORIES: break;
    }
    throw cRuntimeError("Unknown access category = %d", ac);
    return -1;
}

int MacUtils::getCwMin(AccessCategory ac, int aCwMin)
{
    switch (ac)
    {
        case AC_BK: return aCwMin;
        case AC_BE: return aCwMin;
        case AC_VI: return (aCwMin + 1) / 2 - 1;
        case AC_VO: return (aCwMin + 1) / 4 - 1;
        case AC_LEGACY: return aCwMin;
        case AC_NUMCATEGORIES: break;
    }
    throw cRuntimeError("Unknown access category = %d", ac);
    return -1;
}


int MacUtils::cmpMgmtOverData(Ieee80211DataOrMgmtFrame *a, Ieee80211DataOrMgmtFrame *b)
{
    int aPri = dynamic_cast<Ieee80211ManagementFrame*>(a) ? 1 : 0;  //TODO there should really exist a high-performance isMgmtFrame() function!
    int bPri = dynamic_cast<Ieee80211ManagementFrame*>(b) ? 1 : 0;
    return bPri - aPri;
}

int MacUtils::cmpMgmtOverMulticastOverUnicast(Ieee80211DataOrMgmtFrame *a, Ieee80211DataOrMgmtFrame *b)
{
    int aPri = dynamic_cast<Ieee80211ManagementFrame*>(a) ? 2 : a->getReceiverAddress().isMulticast() ? 1 : 0;
    int bPri = dynamic_cast<Ieee80211ManagementFrame*>(a) ? 2 : b->getReceiverAddress().isMulticast() ? 1 : 0;
    return bPri - aPri;
}

void MacUtils::setDirection(const Ieee80211Frame *frame)
{
    if (frame->hasBitError())
        return;
    Ieee80211TwoAddressFrame *frameAux = dynamic_cast<Ieee80211TwoAddressFrame *> (const_cast<Ieee80211Frame *>(frame));
    if (frameAux == nullptr)
        return;
    Ieee80211ReceptionIndication *indication = check_and_cast<Ieee80211ReceptionIndication*>(frame->getControlInfo());
    DirectionalInfo dir;
    dir.direction = indication->getDirection();
    dir.time = simTime();
    directionalDataBase[frameAux->getTransmitterAddress()] = dir;
}

const bool MacUtils::getDirection(const MACAddress &addr, EulerAngles &dir,simtime_t &t) const
{
    auto it = directionalDataBase.find(addr);
    if (it == directionalDataBase.end())
        return false;
    dir = it->second.direction;
    t = it->second.time;
    return true;
}


void MacUtils::createMsduA(Ieee80211DataOrMgmtFrame * fr, cQueue &transmissionQueue)
{
    Ieee80211DataFrame *frame = dynamic_cast<Ieee80211DataFrame *>(fr);
    if (frame == nullptr) {
        transmissionQueue.insert(fr);
        return;
    }

    if (isBroadcastOrMulticast(frame) || frame->getAddress3().isMulticast() || frame->getAddress4().isMulticast()) {
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
            aux->setName(pkt->getName());
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
            aux->setName(pkt->getName());
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
        aux->setName(pkt->getName());
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
        aux->setName(pkt->getName());
        frame = aux;
    }


    for (auto it = elements.begin() ;it != elements.end();++it)
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
            aux->setName(pkt->getName());
            frameAux = aux;
        }
        else
        {
            Ieee80211MsduASubframe * header = new Ieee80211MsduASubframe();
            if (dynamic_cast<Ieee80211DataFrameWithSNAP *>(frameAux)) {
                *((Ieee80211DataFrameWithSNAP *) header) = *((Ieee80211DataFrameWithSNAP*)frameAux);
                header->setWithSnap(true);
            }
            else
                *((Ieee80211DataFrame *) header) = *((Ieee80211DataFrame*)frameAux);
            Ieee80211DataFrame *aux = header;
            aux->setByteLength(14);
            delete frameAux;
            aux->encapsulate(pkt);
            aux->setName(pkt->getName());
            frameAux = aux;
        }
        // change size
        msduaContainer->pushBack(frameAux);
        unsigned int padding = (msduaContainer->getBack()->getByteLength()%4);
        msduaContainer->getBack()->setByteLength(msduaContainer->getBack()->getByteLength()+padding);
    }

    msduaContainer->pushBack(frame);
    return;
}

bool MacUtils::isMsduA(Ieee80211DataOrMgmtFrame * frame)
{
    return dynamic_cast<Ieee80211MsduAContainer*>(frame) != nullptr;
}

bool MacUtils::isMsduAMesh(Ieee80211DataOrMgmtFrame * frame)
{
    Ieee80211MsduAContainer *fr =  dynamic_cast<Ieee80211MsduAContainer*>(frame);
    if (fr == nullptr) return false;
    return dynamic_cast<Ieee80211MsduAMeshSubframe * >(fr->getPacket(0));
}

void MacUtils::getMsduAFrames(Ieee80211DataOrMgmtFrame * frame, std::vector<Ieee80211DataOrMgmtFrame *>&frames)
{

    frames.clear();
    if (!isMsduA(frame))
        return;

    Ieee80211MsduAContainer *frameMsda = dynamic_cast<Ieee80211MsduAContainer*>(frame);
    while (frameMsda->hasBlock())
    {
        Ieee80211MsduASubframe * header = check_and_cast<Ieee80211MsduASubframe *>(frameMsda->popFrom());
        cPacket *pkt = header->decapsulate();
        Ieee80211MsduAMeshSubframe * headerMesh = dynamic_cast<Ieee80211MsduAMeshSubframe *>(header);
        if (headerMesh) {

            Ieee80211MeshFrame * frameAux = new Ieee80211MeshFrame();
            uint64_t size = frameAux->getByteLength();
            *frameAux = *((Ieee80211MeshFrame *) header);
            frameAux->setByteLength(size);
            frameAux->encapsulate(pkt);
            frames.push_back(frameAux);
        }
        else
        {
            if (header->getWithSnap()) {
                Ieee80211DataFrameWithSNAP * frameAux = new Ieee80211DataFrameWithSNAP();
                uint64_t size = frameAux->getByteLength();
                *frameAux = *((Ieee80211DataFrameWithSNAP*) header);
                frameAux->setByteLength(size);
                frameAux->encapsulate(pkt);
                frames.push_back(frameAux);

            }
            else {
                Ieee80211DataFrame * frameAux = new Ieee80211DataFrame();
                uint64_t size = frameAux->getByteLength();
                *frameAux = *((Ieee80211DataFrame*) header);
                frameAux->setByteLength(size);
                frameAux->encapsulate(pkt);
                frames.push_back(frameAux);
            }
        }
        delete header;
    }
    delete frame;
}


} // namespace ieee80211
} // namespace inet
