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
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/antenna/PhasedArray.h"

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
    for (unsigned int i = 0; i < sectorsQueue.size();i++) {
        while (!sectorsQueue[i].isEmpty()) {
            delete sectorsQueue[i].pop();
        }
    }
    sectorsQueue.clear();
    mobilityList.clear();
    cancelAndDelete(trigger);
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
    mobilityList.clear();
    trigger = new cMessage("phase array trigger");
    interval = 2;
    mintime.setRaw(1); // minimum time
    //
    scheduleAt(interval+mintime,trigger);


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
    if (trigger == msg) {
        // TODO: change in the configuration, extract information from the queue and send,
        // search section active
        cModule *par = this->getParentModule()->getParentModule();
        IRadio *radio= dynamic_cast< IRadio*>(par->getSubmodule("radio"));
        IAntenna *pa = const_cast<IAntenna*>(reinterpret_cast<const IAntenna *>(radio->getAntenna()));
        PhasedArray *phas = dynamic_cast<PhasedArray*>(pa);
        unsigned int activeSector = phas->getCurrentActiveSector();
        unsigned int numSectors = phas->getNumSectors();
        if (sectorsQueue.empty()) sectorsQueue.resize(numSectors);
        if (mobilityList.empty()) getListaMac();
        if (!transmissionQueue.isEmpty(s)) {
#ifdef DELAYEDFRAME
            while (!transmissionQueue.isEmpty()){
                Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *> transmissionQueue.pop();
                unsigned int frameSector = getFrameSector(frame);
                sectorsQueue[frameSector-1].insertBefore(sectorsQueue[frameSector-1].front(),frame);
            }
#elif defined(DELETEFRAME)
            while (!transmissionQueue.isEmpty()){
                delete transmissionQueue.pop();
            }
#endif
        }
        // TODO: Limit the number of frames that it is possible to transmit in funtion of the sector time
        while (!sectorsQueue[activeSector-1].isEmpty()) {
            enqueue(check_and_cast<Ieee80211DataOrMgmtFrame *>(sectorsQueue[activeSector-1].pop()));
        }
        simtime_t next = interval * ceil(simTime().dbl()/interval.dbl());
        scheduleAt(next+mintime,trigger); // this guarantee that this trigger arrive always after the phase array trigger.
        return;
    }
    /*
     auto frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
     if ((msg->isSelfMessage()) &&  frame !=nullptr) { // delayed frame, reenque or delay other time
           enqueue(frame);
           return;
     }
     simtime_t next = interval * ceil(simTime().dbl()/Interval.dbl());
     scheduleAt(next+mintime,trigger); // this guarantee that this trigger arrive always after the phase array trigger.
     */
     if (msg->getContextPointer() != nullptr)
        ((MacPlugin *)msg->getContextPointer())->handleSelfMessage(msg);
    else
        ASSERT(false);
}

void DcfUpperMac::upperFrameReceived(Ieee80211DataOrMgmtFrame *frame)
{
    Enter_Method("upperFrameReceived(\"%s\")", frame->getName());
    take(frame);

    MACAddress rxaddr=frame->getReceiverAddress();
    MACAddress txaddr;
                if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
                     txaddr = dataOrMgmtFrame->getTransmitterAddress();
                }

    if (!rxaddr.isBroadcast() ){

    auto itRc = mobilityList.find(rxaddr);
    if (itRc == mobilityList.end())
    throw cRuntimeError("Address not found %s",rxaddr.str().c_str());
    PhasedArray *phas = itRc->second.phaseAr;
    unsigned int activeSector = phas->getCurrentActiveSector();
    unsigned int frameSector = getFrameSector(frame);
    if (activeSector != frameSector) {
        sectorsQueue[frameSector-1].insert(frame);
        return;
    }
    }
    EV_INFO << "Frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    /*bool decision= decideQueue(frame);
                if (!decision){
                    delete frame;
                                    }*/
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
    if (frameExchange)
        transmissionQueue.insert(frame);
    else{
        startSendDataFrameExchange(frame, 0, AC_LEGACY);
    }
}

void DcfUpperMac::lowerFrameReceived(Ieee80211Frame *frame)
{
    Enter_Method("lowerFrameReceived(\"%s\")", frame->getName());
    utils->setDirection(frame);
    /*Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame);
    bool decision= decideQueue(dataOrMgmtFrame);
            if (!decision){
                delete frame;
                                }*/
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

bool DcfUpperMac::decideQueue(Ieee80211DataOrMgmtFrame *frame){
           bool var =false;
           std::vector<MACAddress>list;
           list=getListaMac();
           MACAddress rxaddr=frame->getReceiverAddress();
           MACAddress txaddr;
            if (auto dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
                 txaddr = dataOrMgmtFrame->getTransmitterAddress();
            }

            if (myAddress.isUnspecified())  throw cRuntimeError("myAdress is undefined");
             if (!rxaddr.isMulticast()){

                  txaddr = myAddress;
               int rxpos;
               for (int ind=0;ind < list.size();ind++){
                            if(rxaddr.equals(list[ind]))rxpos=ind;
                         }
               cTopology topology ("topology");
               topology.extractByProperty("networkNode");
               cTopology::Node *NodoRx = topology.getNode(rxpos);
               cModule *hostRx = NodoRx->getModule();
             //  std::cout << hostRx-><<endl;
             //  IRadio *rm= dynamic_cast< IRadio*> (hostRx->getSubmodule("radio"));

               auto itRc = mobilityList.find(rxaddr);
                       if (itRc == mobilityList.end())
                          throw cRuntimeError("Address not found %s",rxaddr.str().c_str());
               auto itTx = mobilityList.find(txaddr);
                       if (itTx == mobilityList.end())
                          throw cRuntimeError("Address not found %s",txaddr.str().c_str());
               Coord txhost= itTx->second.mob->getCurrentPosition();
               Coord rxhost= itRc->second.mob->getCurrentPosition();
               Coord transmissionStartDirection = rxhost - txhost;
               double z = transmissionStartDirection.z;
               transmissionStartDirection.z = 0;
               double heading = atan2(transmissionStartDirection.y, transmissionStartDirection.x);
               double elevation = atan2(z, transmissionStartDirection.length());
               EulerAngles direction = EulerAngles(heading, elevation, 0);
               PhasedArray *phas = itRc->second.phaseAr;
               // double test = phas->getPhizero();


               if(phas){
                   bool temp=phas->isWithinSector(direction);
               if (temp){ // test the position of the receiver
              var=true; //ok (misses instruction)
               }
               else {

                   var=false;//remove frame from queue (misses instruction)
               }
           }

           }
           return var;
          // std::cout<<"prova"<<endl;
}

int DcfUpperMac::getFrameSector (Ieee80211DataOrMgmtFrame *frame) {

    //EulerAngles direction = utils->getFrameDirection(frame);


    if (mobilityList.empty()) getListaMac();
    if (mobilityList.empty()) throw cRuntimeError("List is empty");
    auto itRc = mobilityList.find(frame->getReceiverAddress());
    if (itRc == mobilityList.end())
        throw cRuntimeError("Address not found %s",frame->getReceiverAddress().str().c_str());
    auto itTx = mobilityList.find(myAddress);
    if (itTx == mobilityList.end())
        throw cRuntimeError("Address not found %s",myAddress.str().c_str());

    Coord txhost= itTx->second.mob->getCurrentPosition();
    Coord rxhost= itRc->second.mob->getCurrentPosition();

    Coord transmissionStartDirection = rxhost - txhost;

    double z = transmissionStartDirection.z;
    transmissionStartDirection.z = 0;
    double heading = atan2(transmissionStartDirection.y, transmissionStartDirection.x);
    double elevation = atan2(z, transmissionStartDirection.length());
    EulerAngles direction = EulerAngles(heading, elevation, 0);

    int currentFrameSector =0;

    PhasedArray *phas = itRc->second.phaseAr;
    switch (phas->getNumSectors()){
    case 8 :

         if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 45)){currentFrameSector =1;return currentFrameSector; break;} // first sector width 45°
         if ((direction.alpha*(180/3.14) > 45)&& (direction.alpha*(180/3.14) <= 90)){currentFrameSector =2;return currentFrameSector;break;} //second sector width 45°
         if ((direction.alpha*(180/3.14) > 90)&& (direction.alpha*(180/3.14) <= 135)){currentFrameSector =3;return currentFrameSector; break;} // third sector width 45°..
         if ((direction.alpha*(180/3.14) > 135)&& (direction.alpha*(180/3.14) <= 180)){currentFrameSector =4;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= -135)){currentFrameSector =5;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -135)&& (direction.alpha*(180/3.14) <= -90)){currentFrameSector =6;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -90)&& (direction.alpha*(180/3.14) <= -45)){currentFrameSector =7;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -45)&& (direction.alpha*(180/3.14) < 0)){currentFrameSector =8;return currentFrameSector; break;}


    case  6:

         if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 60)){currentFrameSector =1;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > 60)&& (direction.alpha*(180/3.14) <= 120)){currentFrameSector =2;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > 120)&& (direction.alpha*(180/3.14) <= 180)){currentFrameSector =3;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= 120)){currentFrameSector =4;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -120)&& (direction.alpha*(180/3.14) <= -60)){currentFrameSector =5;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -60)&& (direction.alpha*(180/3.14) <= 0)){currentFrameSector =6;return currentFrameSector; break;}


    case  4:

         if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 90)){currentFrameSector =1;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > 90)&& (direction.alpha*(180/3.14) <= 180)){currentFrameSector =2;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= -90)){currentFrameSector =3;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -90)&& (direction.alpha*(180/3.14) <= 0)){currentFrameSector =4;return currentFrameSector; break;}


    case  3:

         if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 120)){currentFrameSector =1;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > 120)&& (direction.alpha*(180/3.14) <= -120)){currentFrameSector =2;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -120)&& (direction.alpha*(180/3.14) <= 0)){currentFrameSector =3;return currentFrameSector; break;}


    case  2:

         if ((direction.alpha*(180/3.14) >= 0)&& (direction.alpha*(180/3.14) <= 180)){currentFrameSector =1;return currentFrameSector; break;}
         if ((direction.alpha*(180/3.14) > -180)&& (direction.alpha*(180/3.14) <= 0)){currentFrameSector =2;return currentFrameSector; break;}


    default: return -1;
    }
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

std::vector<MACAddress> DcfUpperMac:: getListaMac(){

        if (!mobilityList.empty()) {
            std::vector<MACAddress>lista;
            for (const auto &elem : mobilityList) {
                lista.push_back(elem.first);
            }
            return lista;
        }

        cModule * myHost = getContainingNode(this);
        std::vector<MACAddress>lista;
        cTopology topology ("topology");
        topology.extractByProperty("networkNode");
        int n = topology.getNumNodes();
        if (topology.getNumNodes() == 0)
            throw cRuntimeError("Empty network!");
        for (int i = 0; i < n; i++) {
            NodeData data;
            cTopology::Node *destNode = topology.getNode(i);
            cModule *host = destNode->getModule();
            data.mob = check_and_cast<IMobility *>(host->getSubmodule("mobility"));

            IInterfaceTable * ifTable = L3AddressResolver().findInterfaceTableOf(host);

            for (int i = 0; i < ifTable->getNumInterfaces(); i++) {
                InterfaceEntry *entry = ifTable->getInterface(i);
                if (strstr(entry->getName(),"wlan") != nullptr) {

                    cModule *ifaceMod = entry->getInterfaceModule();
                    IRadio *radioMod = check_and_cast<IRadio *>(ifaceMod->getSubmodule("radio"));
                    IAntenna *pa = const_cast<IAntenna*>(radioMod->getAntenna());
                    data.phaseAr = dynamic_cast<PhasedArray *>(pa);
                    IPv4Address addr = L3AddressResolver().getAddressFrom(L3AddressResolver().findInterfaceTableOf(host)).toIPv4();
                    MACAddress ma = entry->getMacAddress();
                    lista.push_back(ma);
                    mobilityList[ma] = data;
                    if (myHost == host)
                        myAddress = ma;
                    break;
                }
            }
        }
        if (lista.empty())
            throw cRuntimeError("Lista is empty");
        return lista;
    }




} // namespace ieee80211
} // namespace inet

