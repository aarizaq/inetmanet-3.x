//
// Copyright (C) 2008 Alfonso Ariza
// Copyright (C) 2010 Alfonso Ariza
// Copyright (C) 2012 Alfonso Ariza
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

#define CHEAT_IEEE80211MESH
#include <string.h>
#include <algorithm>
#include "inet/linklayer/ieee80211mesh/mgmt/Ieee80211Mesh.h"
#include "inet/routing/extras/base/MeshControlInfo_m.h"
#include "inet/routing/extras/base/ControlInfoBreakLink_m.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"
#include "inet/linklayer/ieee80211mesh/locator/locatorPkt_m.h"
#include "inet/linklayer/ethernet//EtherFrame_m.h"
#include "inet/routing/extras/olsr/OLSR.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"

//#define LIMITBROADCAST

/* WMPLS */
namespace inet{

namespace ieee80211 {

using namespace physicallayer;

using namespace inetmanet;

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("inet::ieee80211::SelectionCriteria");
    if (!e) enums.getInstance()->add(e = new cEnum("inet::ieee80211::SelectionCriteria"));
    e->insert(Ieee80211Mesh::ETX, "Etx");
    e->insert(Ieee80211Mesh::MINQUEUE, "MinQueue");
    e->insert(Ieee80211Mesh::LASTUSED, "LastUsed");
    e->insert(Ieee80211Mesh::MINQUEUELASTUSED, "MinQueueLastUsed");
    e->insert(Ieee80211Mesh::LASTUSEDMINQUEUE, "LastUsedMinQueue");
);

#if !defined (UINT32_MAX)
#   define UINT32_MAX  4294967295UL
#endif

#ifdef CHEAT_IEEE80211MESH
Ieee80211Mesh::GateWayDataMap * Ieee80211Mesh::gateWayDataMap;
#endif

simsignal_t Ieee80211Mesh::numHopsSignal = SIMSIGNAL_NULL;
simsignal_t Ieee80211Mesh::numFixHopsSignal = SIMSIGNAL_NULL;

const int Ieee80211Mesh::MaxSeqNum = 1;

std::ostream& operator<<(std::ostream& os, const LWmpls_Forwarding_Structure& e)
{
    os << e.info();
    return os;
};

std::ostream& operator<<(std::ostream& os, const LWMPLSKey& e)
{
    os <<  "label : " << e.label << " MAC addr " << MACAddress(e.mac_addr).str();
    return os;
};

Define_Module(Ieee80211Mesh);

Ieee80211Mesh::~Ieee80211Mesh()
{
//  gateWayDataMap.clear();
    if (mplsData)
        delete mplsData;
    if (WMPLSCHECKMAC)
        cancelAndDelete(WMPLSCHECKMAC);
    if (gateWayTimeOut)
        cancelAndDelete(gateWayTimeOut);
    associatedAddress.clear();
    if (getGateWayDataMap())
    {
        for (auto it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
        {
            it->second.proactive=nullptr;
            it->second.reactive=nullptr;
            it->second.associatedAddress=nullptr;
        }
        // getGateWayDataMap()->clear();
        delete gateWayDataMap;
        gateWayDataMap = nullptr;
    }
    if (getOtpimunRoute)
        delete getOtpimunRoute;

    while (!confirmationFrames.empty())
    {
        cancelAndDelete(confirmationFrames.back().frame);
        confirmationFrames.pop_back();
    }
    timeReceptionInterface.clear();
    macInterfaces.clear();
    //radioInterfaces.clear();
}

Ieee80211Mesh::Ieee80211Mesh()
{
    // Mpls data
    mplsData = nullptr;
    gateWayDataMap=nullptr;
   // gateWayDataMap.clear();
    // subprocess
    ETXProcess = nullptr;
    routingModuleProactive = nullptr;
    routingModuleReactive = nullptr;
    routingModuleHwmp = nullptr;
    // packet timers
    WMPLSCHECKMAC = nullptr;
    gateWayTimeOut = nullptr;
    //
    macBaseGateId = -1;
    gateWayIndex = -1;
    isGateWay = false;
    hasLocator = false;
    hasRelayUnit = false;
    numRoutingBytes = 0;
    numDataBytes = 0;

    getOtpimunRoute = nullptr;

    proactiveFeedback = false;
    maxHopProactiveFeedback = -1;
    maxHopProactive = -1;
    maxHopReactive = -1;

    floodingConfirmation = false;
    confirmationFrames.clear();
    timeReceptionInterface.clear();
    macInterfaces.clear();
    //radioInterfaces.clear();
    hasSecurity = false;
}

void Ieee80211Mesh::initializeBase(int stage)
{

    if (stage == INITSTAGE_LOCAL)
    {
        Ieee80211MgmtBase::initialize(stage);
        inteligentBroadcastRouting = par("inteligentBroadcastRouting").boolValue();
        numMac = 0;
    }
    else if (stage == INITSTAGE_LINK_LAYER)
    {
        Ieee80211MgmtBase::initialize(stage);
    }
    else if (stage==INITSTAGE_PHYSICAL_ENVIRONMENT)
    {
        // obtain our address from MAC

        cModule *nic = getContainingNicModule(this);
        cModule *mac = nic->getSubmodule("mac");
        if (mac == nullptr) {
            // search for vector of mac:
            unsigned int numRadios = 0;
            do {
                mac = nic->getSubmodule("mac",numMac);
                if (mac)
                    numMac++;
            } while (mac);

            if (numMac == 0)
                throw cRuntimeError("MAC module not found; it is expected to be next to this submodule and called 'mac'");

            cModule *radio = nullptr;
            do {
                radio = nic->getSubmodule("radio",numRadios);
                if (radio)
                    numRadios++;
            } while (radio);
            if (numRadios != numMac)
                throw cRuntimeError("numRadios != numMac");

            mac = nic->getSubmodule("mac",0);
        }
        if (!mac)
            throw cRuntimeError("MAC module not found; it is expected to be next to this submodule and called 'mac'");
        myAddress = check_and_cast<Ieee80211Mac *>(mac)->getAddress();
    }
}


void Ieee80211Mesh::initialize(int stage)
{
    EV << "Init mesh proccess \n";
    initializeBase(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        limitDelay = par("maxDelay").doubleValue();
        useLwmpls = par("UseLwMpls");
        maxTTL = par("maxTTL");
        if (gate("upperLayerOut")->getPathEndGate()->isConnected() &&
                (strcmp(gate("upperLayerOut")->getPathEndGate()->getOwnerModule()->getName(),"relayUnit")==0 || par("forceRelayUnit").boolValue()))
        {
            hasRelayUnit = true;
        }
        if (gate("locatorOut")->getPathEndGate()->isConnected() &&
                       (strcmp(gate("locatorOut")->getPathEndGate()->getOwnerModule()->getName(),"locator")==0 || par("locatorActive").boolValue()))
            hasLocator = true;

        if (gate("macOut")->getPathEndGate()->isConnected())
            isMultiMac = false;
        else if (gate("macOutMulti",0)->getPathEndGate()->isConnected())
            isMultiMac = true;
        else
            throw cRuntimeError("mac not connected");


        if (gate("securityOut")->getPathEndGate()->isConnected() &&
                        (strcmp(gate("securityOut")->getPathEndGate()->getOwnerModule()->getName(),"security")==0 || par("securityActive").boolValue()))
            hasSecurity = true;
        const char *addrModeStr = par("selectionCriteria").stringValue();
        selectionCriteria = (SelectionCriteria) cEnum::get("inet::ieee80211::SelectionCriteria")->lookup(addrModeStr);
    }
    else if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT)
    {
        bool useReactive = par("useReactive");
        bool useProactive = par("useProactive");
        bool ETXEstimate = par("ETXEstimate");
        bool useHwmp = par("useHwmp");
        bool useGreenie = par("useGreenie");

        if (!useReactive && !useProactive && !useHwmp)
            useHwmp = true;

        if (useHwmp)
        {
            useReactive = false;
            useProactive = false;
            useLwmpls = false;
        }

        //if (useReactive)
        //    useProactive = false;


        if (useGreenie)
        {

            if (this->hasPar("maxHopProactiveFeedback"))
                maxHopProactiveFeedback = par("maxHopProactiveFeedback");
            if (this->hasPar("maxHopProactive"))
                maxHopProactive = par("maxHopProactive");
            if (this->hasPar("maxHopReactive"))
                   maxHopReactive = par("maxHopReactive");
            if (par("greenieCompleteNode"))
                useProactive = true;
            useReactive = true;
            if (useReactive && useProactive)
                proactiveFeedback = true;

        }
        else if (useReactive && useProactive)
        {
            if (this->hasPar("ProactiveFeedback"))
                proactiveFeedback  = par("ProactiveFeedback");
            else
                proactiveFeedback = true;

            if (this->hasPar("maxHopProactiveFeedback"))
                maxHopProactiveFeedback = par("maxHopProactiveFeedback");
            if (this->hasPar("maxHopProactive"))
                maxHopProactive = par("maxHopProactive");
            if (this->hasPar("maxHopReactive"))
                   maxHopReactive = par("maxHopReactive");
        }

        mplsData = new LWMPLSDataStructure;
        WATCH(hasRelayUnit);
        WATCH_PTRMAP(*(mplsData->forwardingTableOutput));

         //
        // cambio para evitar que puedan estar los dos protocolos simultaneamente
        // cuidado con esto
        //
        // Proactive protocol
        if (useReactive)
            startReactive();
        // Reactive protocol
        if (useProactive)
            startProactive();
        // Hwmp protocol
        if (useHwmp)
            startHwmp();

        if (routingModuleProactive == nullptr && routingModuleReactive ==nullptr && routingModuleHwmp==nullptr)
            error("Ieee80211Mesh doesn't have active routing protocol");

        mplsData->mplsMaxTime() = 35;
        activeMacBreak = false;
        if (activeMacBreak)
            WMPLSCHECKMAC = new cMessage();

        ETXProcess = nullptr;
        if (selectionCriteria == ETX && numMac > 1)
            ETXEstimate = true;

        if (ETXEstimate)
            startEtx();

        numHopsSignal = registerSignal("numHopsSignal");
        numFixHopsSignal = registerSignal("numFixHopsSignal");

    }
    else if (stage == INITSTAGE_LINK_LAYER)
    {
        // macBaseGateId = gateSize("macOut")==0 ? -1 : gate("macOut",0)->getId(); // FIXME macBaseGateId is unused, what is it?
        macBaseGateId = 0;
        EV << "macBaseGateId :" << macBaseGateId << "\n";
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        cModule *host = getContainingNode(this);
        host->subscribe(NF_LINK_BREAK,this);
        host->subscribe(NF_LINK_REFRESH,this);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2)
    {
        if (routingModuleProactive)
            routingModuleProactive->setStaticNode(par("FixNode").boolValue());
        if (routingModuleReactive)
            routingModuleReactive->setStaticNode(par("FixNode").boolValue());
        if (routingModuleReactive && routingModuleProactive)
        {
            routingModuleReactive->setCollaborativeProtocol(routingModuleProactive);
            routingModuleProactive->setCollaborativeProtocol(routingModuleReactive);
        }
        if (par("IsGateWay"))
            startGateWay();

        if (par("coverageArea").doubleValue() > 0)
        {
            getOtpimunRoute = new WirelessNumHops();
            getOtpimunRoute->setRoot(myAddress);
        }
        //end Gateway and group address code
        if (numMac > 1 && selectionCriteria != ETX)
             timeReceptionInterface.resize(numMac);
    }
}

void Ieee80211Mesh::startProactive()
{
    cModuleType *moduleType;
    cModule *module;
    //if (isEtx)
    //  moduleType = cModuleType::find("inet.networklayer.manetrouting.OLSR_ETX");
    //else
    moduleType = cModuleType::find("inet.routing.extras.OLSR");
    if (!moduleType)
        throw cRuntimeError("Module inet.routing.extras.OLSR not found");
    module = moduleType->create("ManetRoutingProtocolProactive", this);
    routingModuleProactive = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleProactive->gate("ipOut")->connectTo(gate("routingInProactive"));
    gate("routingOutProactive")->connectTo(routingModuleProactive->gate("ipIn"));
    routingModuleProactive->par("interfaceTableModule").setStringValue(par("interfaceTableModule").stringValue());
    routingModuleProactive->par("routingTableModule").setStringValue("");
    routingModuleProactive->par("icmpModule").setStringValue("");
    routingModuleProactive->buildInside();
    routingModuleProactive->scheduleStart(simTime());
}


void Ieee80211Mesh::startReactive()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find(par("meshReactiveRoutingProtocol").stringValue());
    if (!moduleType)
        throw cRuntimeError("Module %s not found",par("meshReactiveRoutingProtocol").stringValue());
    module = moduleType->create("ManetRoutingProtocolReactive", this);
    routingModuleReactive = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleReactive->gate("ipOut")->connectTo(gate("routingInReactive"));
    gate("routingOutReactive")->connectTo(routingModuleReactive->gate("ipIn"));
    routingModuleReactive->par("interfaceTableModule").setStringValue(par("interfaceTableModule").stringValue());
    routingModuleReactive->par("routingTableModule").setStringValue("");
    routingModuleReactive->par("icmpModule").setStringValue("");
    routingModuleReactive->buildInside();
    routingModuleReactive->scheduleStart(simTime());
}

void Ieee80211Mesh::startHwmp()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find("inet.linklayer.ieee80211mesh.hwmp.HwmpProtocol");
    if (!moduleType)
        throw cRuntimeError("Module inet.linklayer.ieee80211mesh.hwmp.HwmpProtocol not found");
    module = moduleType->create("HwmpProtocol", this);
    routingModuleHwmp = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleHwmp->gate("ipOut")->connectTo(gate("routingInHwmp"));
    gate("routingOutHwmp")->connectTo(routingModuleHwmp->gate("ipIn"));
    routingModuleHwmp->par("interfaceTableModule").setStringValue(par("interfaceTableModule").stringValue());
    routingModuleHwmp->buildInside();
    routingModuleHwmp->scheduleStart(simTime());
}

void Ieee80211Mesh::startEtx()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find("inet.linklayer.ieee80211.mgmt.Ieee80211Etx");
    if (!moduleType)
        throw cRuntimeError("Module %s not found",par("meshReactiveRoutingProtocol").stringValue());
    module = moduleType->create("ETXproc", this);
    ETXProcess = dynamic_cast <Ieee80211Etx*> (module);
    ETXProcess->gate("toMac")->connectTo(gate("ETXProcIn"));
    gate("ETXProcOut")->connectTo(ETXProcess->gate("fromMac"));
    ETXProcess->par("interfaceTableModule").setStringValue(par("interfaceTableModule").stringValue());
    ETXProcess->buildInside();
    ETXProcess->scheduleStart(simTime());
    ETXProcess->setAddress(myAddress);
    ETXProcess->setNumInterfaces(numMac);
}

void Ieee80211Mesh::startGateWay()
{
// check if the ethernet exist
// check: datarate is forbidden with EtherMAC -- module's txrate must be used
    isGateWay = true;
    if (gateWayDataMap == nullptr)
        gateWayDataMap = new GateWayDataMap;
    char mameclass[60];
    cGate *g = gate("toEthernet")->getPathEndGate();
    MACAddress ethAddr;
    strcpy(mameclass,g->getOwnerModule()->getClassName());
    if (strcmp(mameclass, "EtherEncapMesh")!=0)
        return;
    // find the interface
    char interfaceName[100];
    for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry * ie= ift->getInterface(i);
        if (ie->isLoopback())
            continue;
        if (ie->getMacAddress()==myAddress)
            continue;
        memset(interfaceName,0,sizeof(interfaceName));
        char *d = interfaceName;
        for (const char *s = g->getOwnerModule()->getParentModule()->getFullName(); *s; s++)
            if (isalnum(*s))
                *d++ = *s;
        *d = '\0';
        if (strcmp(interfaceName,ie->getName())==0)
        {
            ethAddr=ie->getMacAddress();
            break;
        }
    }
    if (ethAddr.isUnspecified())
        throw cRuntimeError("Mesh gateway not initialized, Ethernet interface not found");
    GateWayData data;
    data.idAddress=myAddress;
    data.ethAddress=ethAddr;
    #ifdef CHEAT_IEEE80211MESH
    data.associatedAddress=&associatedAddress;
    data.proactive=routingModuleProactive;
    data.reactive=routingModuleReactive;
    #endif
    getGateWayDataMap()->insert(std::pair<L3Address,GateWayData>(L3Address(myAddress),data));
    if(routingModuleProactive)
        routingModuleProactive->addInAddressGroup(L3Address(myAddress));
    if (routingModuleReactive)
        routingModuleReactive->addInAddressGroup(L3Address(myAddress));
    gateWayIndex = 0;
    for(auto it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
    {
        if (it->first == L3Address(myAddress))
            break;
        gateWayIndex++;
    }
    gateWayTimeOut = new cMessage();
    double delay=gateWayIndex*par("GateWayAnnounceInterval").doubleValue ();
    scheduleAt(simTime()+delay+uniform(0,2),gateWayTimeOut);
}

void Ieee80211Mesh::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage())
    {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
        return;
    }
    cGate * msggate = msg->getArrivalGate();
    char gateName [40];
    memset(gateName,0,40);
    strcpy(gateName,msggate->getBaseName());
    //if (msg->arrivedOn("macIn"))
    if (strstr(gateName,"macIn")!=nullptr)
    {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        if (msggate->isVector())
        {
            msg->addPar("indexGate") = msggate->getIndex() ;
        }


        if (!timeReceptionInterface.empty())
        {
            Ieee80211TwoAddressFrame *frame = dynamic_cast<Ieee80211TwoAddressFrame *>(msg);
            timeReceptionInterface[msggate->getIndex()][frame->getTransmitterAddress()] = simTime();

/*            Ieee80211MeshFrame *frameAux = dynamic_cast<Ieee80211MeshFrame *>(msg);
            if (frameAux && frameAux->getSubType() == ROUTING && frameAux->getChannelsArraySize() > 0)
            {
                for (unsigned int i = 0; i < numMac; i++)
                {
                    for (unsigned int j = 0; j < frameAux->getChannelsArraySize(); j++)
                    {
                        if (radioInterfaces[i]->getChannel() == frameAux->getChannels(j))
                        {
                            timeReceptionInterface[i][frame->getTransmitterAddress()] = simTime();
                            break;
                        }
                    }
                }
            }*/
        }
        // if encrypted
        if(strstr(msg->getName() ,"CCMPFrame")!=nullptr && hasSecurity)
        {
            EV << "CCMPFrame Frame arrived from MAC, send it to SecurityModule" << msg << "\n";

            if(dynamic_cast<Ieee80211MeshFrame *>(msg))
            {
                Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
                actualizeReactive(frame,false);
                Ieee80211MeshFrame *frame2 = (check_and_cast<Ieee80211MeshFrame *>(msg));
                if(frame2->getFinalAddress().compareTo(myAddress)==0)
                    send(msg, "securityOut");
                else
                    processFrame(frame);
            }
            else
                send(msg, "securityOut");
        }
        else if (dynamic_cast<Ieee80211ActionMeshFrame *>(msg))
        {
            if ((routingModuleHwmp != nullptr) && (routingModuleHwmp->isOurType(PK(msg)))) {
                if (PK(msg)->getControlInfo())
                    delete PK(msg)->removeControlInfo();

                send(msg,"routingOutHwmp");
            }
            else
                delete msg;
        }
        // if encrypted
          else if(hasSecurity && (strstr(msg->getName() ,"AMPE msg 1/4")!=nullptr || strstr(msg->getName() ,"AMPE msg 2/4")!=nullptr
                  || strstr(msg->getName() ,"AMPE msg 3/4")!=nullptr  || strstr(msg->getName() ,"AMPE msg 4/4")!=nullptr
                  || strstr(msg->getName() ,"Group msg 1/2")!=nullptr  || strstr(msg->getName() ,"Group msg 2/2")!=nullptr
                  ))
          {
              EV << " Frame arrived from MAC, send it to SecurityModule" << msg << "\n";
              send(msg, "securityOut");
          }

        else
        {
            Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
            Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(msg);
            if (frame2)
            {
                frame2->setTTL(frame2->getTTL()-1);
                frame2->setTotalHops(frame2->getTotalHops()+1);
                if (par("FixNode").boolValue())
                {
                    frame2->setTotalStaticHops(frame2->getTotalStaticHops()+1);
                }
            }
            actualizeReactive(frame,false);
            processFrame(frame);
        }
    }
    //else if (msg->arrivedOn("agentIn"))
    else if (strstr(gateName,"agentIn")!=nullptr)
    {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cObject *ctrl = msg->removeControlInfo();
        delete msg;
        handleCommand(msgkind, ctrl);
    }
    //else if (msg->arrivedOn("routingIn"))
    else if (strstr(gateName,"routingIn")!=nullptr)
    {
        handleRoutingMessage(PK(msg));
    }
    else if (strstr(gateName,"ETXProcIn")!=nullptr)
    {
        handleEtxMessage(PK(msg));
    }
    //else if (strstr(gateName,"interGateWayConect")!=nullptr)
    else if (strstr(gateName,"fromEthernet")!=nullptr)
    {
        handleWateGayDataReceive(PK(msg));
    }
    else if(strstr(msg->getName() ,"Beacon")!=nullptr ){sendFrameDown(PK(msg));}
    else if(   strstr(msg->getName() ,"Beacon")!=nullptr
            || strstr(msg->getName() ,"Open Authentication Request")!=nullptr || strstr(msg->getName() ,"Open Authentication Response")!=nullptr
            || strstr(msg->getName() ,"Auth")!=nullptr || strstr(msg->getName() ,"Auth-OK")!=nullptr || strstr(msg->getName() ,"Auth-ERROR")!=nullptr
            || strstr(msg->getName() ,"Auth msg 1/4")!=nullptr || strstr(msg->getName() ,"Auth msg 2/4")!=nullptr ||strstr(msg->getName() ,"Auth msg 3/4")!=nullptr
            || strstr(msg->getName() ,"Auth msg 4/4")!=nullptr || strstr(msg->getName() , "Group msg 1/2")!=nullptr || strstr(msg->getName() ,"Group msg 2/2")!=nullptr
            || strstr(msg->getName() ,"SAE msg 1/4")!=nullptr  || strstr(msg->getName() ,"SAE msg 2/4")!=nullptr
            || strstr(msg->getName() ,"SAE msg 3/4")!=nullptr  || strstr(msg->getName() ,"SAE-OK msg 4/4")!=nullptr
            || strstr(msg->getName() ,"AMPE msg 1/4")!=nullptr || strstr(msg->getName() ,"AMPE msg 2/4")!=nullptr
            || strstr(msg->getName() ,"AMPE msg 3/4")!=nullptr || strstr(msg->getName() ,"AMPE msg 4/4")!=nullptr )
    {
        sendFrameDown(PK(msg));
    }
    else if (strstr(gateName,"securityIn")!=nullptr && hasSecurity)
    {
        if(strstr(msg->getName() ,"CCMPFrame")!=nullptr)
        {
            EV << "CCMPFrame Frame arrived from Security, send it to Mac_" <<endl;
            sendFrameDown(PK(msg));

        }
        else if(strstr(msg->getName() ,"DecCCMP")!=nullptr)
        {
            EV << "Frame arrived from Security, send it to upper layers: " << msg << "\n";

            Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
            Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(msg);
            if (frame2)
                frame2->setTTL(frame2->getTTL()-1);
            actualizeReactive(frame,false);

            if (dynamic_cast<Ieee80211ActionMeshFrame *>(msg))
            {
                if ((routingModuleHwmp != nullptr) && (routingModuleHwmp->isOurType(PK(msg)))) {
                    if (PK(msg)->getControlInfo())
                        delete PK(msg)->removeControlInfo();

                    send(msg,"routingOutHwmp");
                }
                else
                    delete msg;
            }
            else
               // delete msg;
                processFrame(frame);
        }
        else
        {

            sendFrameDown(PK(msg));
        }
    }

    else
    {
        cPacket *pk = PK(msg);
        // packet from upper layers, to be sent out
        EV << "Packet arrived from upper layers: " << pk << "\n";
        if (pk->getByteLength() > 2312)
            error("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                  pk->getClassName(), pk->getName(), pk->getByteLength());
        handleUpperMessage(pk);
    }
}


void Ieee80211Mesh::handleTimer(cMessage *msg)
{
    //ASSERT(false);
    mplsData->lwmpls_interface_delete_old_path();
    if (WMPLSCHECKMAC==msg)
        mplsCheckRouteTime();
    else if (gateWayTimeOut==msg)
        publishGateWayIdentity();
    else if (dynamic_cast<Ieee80211DataFrame*>(msg))
    {
        if (floodingConfirmation && !confirmationFrames.empty())
        {
            for (unsigned int i = 0; i < confirmationFrames.size(); i++)
            {
                if (confirmationFrames[i].frame == msg)
                {
                    confirmationFrames.erase(confirmationFrames.begin()+i);
                    break;
                }
            }
        }
        sendFrameDown(PK(msg));
    }
    else
        throw cRuntimeError("message timer error");
}


void Ieee80211Mesh::handleRoutingMessage(cPacket *msg)
{

    cObject *temp  = msg->removeControlInfo();
    Ieee802Ctrl * ctrl = dynamic_cast<Ieee802Ctrl*> (temp);
    if (!ctrl)
    {
        char name[50];
        strcpy(name,msg->getName());
        error ("Message error, the routing message %s doesn't have Ieee802Ctrl control info",name);
    }
    if (dynamic_cast<Ieee80211ActionMeshFrame *>(msg))
    {
        if (!msg->hasPar("indexGate"))
            msg->addPar("indexGate") = ctrl->getInterfaceId();
        else
            msg->par("indexGate") = ctrl->getInterfaceId();
        delete ctrl;
        sendFrameDown(msg);
    }
    else
    {
        Ieee80211DataFrame * frame = encapsulate(msg,ctrl->getDest());
        Ieee80211MeshFrame *frameMesh = check_and_cast<Ieee80211MeshFrame*>(frame);
        if (frameMesh->getSubType() == 0)
            frameMesh->setSubType(ROUTING);

        if (!msg->hasPar("indexGate"))
            frame->addPar("indexGate") = ctrl->getInterfaceId();
         else
             frame->par("indexGate") = ctrl->getInterfaceId();
        delete ctrl;
        sendFrameDown(frame);
    }
}

void Ieee80211Mesh::handleUpperMessage(cPacket *msg)
{
    EV<<"HandleUpperMessage"<<endl;
    Ieee80211DataFrame *frame = encapsulate(msg);
    if (frame)
    {
        if (!isGateWay)
            sendFrameDown(frame);
        else
        {
            MACAddress gw;
            if (!frame->getAddress4().isBroadcast() && !frame->getAddress4().isUnspecified())
            {
                if (selectGateWay(L3Address(frame->getAddress4()), gw))
                {
                    if (gw != myAddress)
                    {
                        frame->setReceiverAddress(gw);
                        sendFrameDown(frame);
                        return;
                    }
                }
            }
            handleReroutingGateway(frame);
        }
    }
}

void Ieee80211Mesh::handleCommand(int msgkind, cObject *ctrl)
{
    error("handleCommand(): no commands supported");
}

Ieee80211DataFrame *Ieee80211Mesh::encapsulate(cPacket *msg)
{
    EV<<"Encapsulate"<<endl;
    Ieee80211MeshFrame *frame = new Ieee80211MeshFrame(msg->getName());
    frame->setSubType(UPPERMESSAGE);
    frame->setTTL(maxTTL);
    frame->setTimestamp(msg->getCreationTime());
    LWMPLSPacket *lwmplspk = nullptr;
    LWmpls_Forwarding_Structure *forwarding_ptr=nullptr;
    MACAddress next;
    MACAddress dest;

    if (hasRelayUnit)
    {
        EtherFrame *etherframe = dynamic_cast<EtherFrame *>(msg);
        if (etherframe)
        {
        // create new frame
            frame->setFromDS(true);
            // copy addresses from ethernet frame (transmitter addr will be set to our addr by MAC)
            frame->setAddress4(etherframe->getDest());
            frame->setAddress3(etherframe->getSrc());
            dest = etherframe->getDest();
            next = etherframe->getDest();
            frame->setFinalAddress(dest);
            // encapsulate payload
            cPacket *payload = etherframe->decapsulate();
            if (!payload)
                error("received empty EtherFrame from upper layer");

            delete etherframe;
            msg = payload;
        }
        else
        {
            frame = check_and_cast<Ieee80211MeshFrame *>(msg);
        }
    }
    else
    {

    // copy receiver address from the control info (sender address will be set in MAC)
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
        dest = ctrl->getDest();
        next = ctrl->getDest();
        int up = ctrl->getUserPriority();
        if (up >= 0) {
            // make it a QoS frame, and set TID
            frame->setType(ST_DATA_WITH_QOS);
            frame->addBitLength(QOSCONTROL_BITS);
            frame->setTid(up);
        }
        delete ctrl;
        frame->setAddress3(myAddress);
        frame->setFinalAddress(dest);
        frame->setAddress4(dest);
    }

    if (dest.isBroadcast())
    {
        frame->setReceiverAddress(dest);
        frame->setTTL(1);
        uint32_t cont;

        mplsData->getBroadCastCounter(cont);

        lwmplspk = new LWMPLSPacket(msg->getName());
        cont++;
        mplsData->setBroadCastCounter(cont);
        lwmplspk->setCounter(cont);
        lwmplspk->setSource(myAddress);
        lwmplspk->setDest(dest);
        lwmplspk->setType(WMPLS_BROADCAST);
        lwmplspk->encapsulate(msg);
        frame->encapsulate(lwmplspk);
        return frame;
    }
    //
    // Search in the data base
    //

    int label = -1;
    if (useLwmpls)
        label = mplsData->getRegisterRoute(dest.getInt());

    if (label!=-1)
    {
        forwarding_ptr = mplsData->lwmpls_forwarding_data(label,-1,0);
        if (!forwarding_ptr)
            mplsData->deleteRegisterRoute(dest.getInt());

    }
    bool toGateWay=false;
    if (routingModuleReactive)
    {
        if (routingModuleReactive->findInAddressGroup(L3Address(dest)))
            toGateWay = true;
    }
    else if (routingModuleProactive)
    {
        if (routingModuleProactive->findInAddressGroup(L3Address(dest)))
             toGateWay = true;
    }
    else if (routingModuleHwmp)
    {
        if (routingModuleHwmp->findInAddressGroup(L3Address(dest)))
             toGateWay = true;
    }


    if (forwarding_ptr)
    {
        lwmplspk = new LWMPLSPacket(msg->getName());
        lwmplspk->setTTL(maxTTL);
        lwmplspk->setSource(myAddress);
        lwmplspk->setDest(dest);

        if (forwarding_ptr->order == LWMPLS_EXTRACT)
        {
// Source or destination?
            if (forwarding_ptr->output_label>0 || forwarding_ptr->return_label_output>0)
            {
                lwmplspk->setType(WMPLS_NORMAL);
                if (forwarding_ptr->return_label_input==label && forwarding_ptr->output_label>0)
                {
                    next = MACAddress(forwarding_ptr->mac_address);
                    lwmplspk->setLabel(forwarding_ptr->output_label);
                }
                else if (forwarding_ptr->input_label==label && forwarding_ptr->return_label_output>0)
                {
                    next = MACAddress(forwarding_ptr->input_mac_address);
                    lwmplspk->setLabel(forwarding_ptr->return_label_output);
                }
                else
                {
                    throw cRuntimeError("lwmpls data base error");
                }
            }
            else
            {
                lwmplspk->setType(WMPLS_BEGIN_W_ROUTE);

                int dist = forwarding_ptr->path.size()-2;
                lwmplspk->setVectorAddressArraySize(dist);
                //lwmplspk->setDist(dist);
                next= MACAddress(forwarding_ptr->path[1]);

                for (int i=0; i<dist; i++)
                    lwmplspk->setVectorAddress(i, MACAddress(forwarding_ptr->path[i+1]));
                lwmplspk->setLabel (forwarding_ptr->return_label_input);
            }
        }
        else
        {
            lwmplspk->setType(WMPLS_NORMAL);
            if (forwarding_ptr->input_label==label && forwarding_ptr->output_label>0)
            {
                next = MACAddress(forwarding_ptr->mac_address);
                lwmplspk->setLabel(forwarding_ptr->output_label);
            }
            else if (forwarding_ptr->return_label_input==label && forwarding_ptr->return_label_output>0)
            {
                next = MACAddress(forwarding_ptr->input_mac_address);
                lwmplspk->setLabel(forwarding_ptr->return_label_output);
            }
            else
            {
                throw cRuntimeError("lwmpls data base error");
            }
        }
        forwarding_ptr->last_use=simTime();
    }
    else
    {
        std::vector<L3Address> add;
        int dist = 0;
        bool noRoute;

        if (routingModuleProactive)
        {
            if (toGateWay)
            {
                bool isToGt;
                L3Address gateWayAddress;
                dist = routingModuleProactive->getRouteGroup(L3Address(dest),add,gateWayAddress,isToGt);
                noRoute = false;
            }
            else
            {
                dist = routingModuleProactive->getRoute(L3Address(dest),add);
                noRoute = false;
            }
        }

        if (dist==0)
        {
            // Search in the reactive routing protocol
            // Destination unreachable
            if (routingModuleReactive)
            {
                add.resize(1);
                if (toGateWay)
                {
                    int iface;
                    noRoute = true;
                    L3Address gateWayAddress;
                    bool isToGt;

                    if (!routingModuleReactive->getNextHopGroup(L3Address(dest),add[0],iface,gateWayAddress,isToGt)) //send the packet to the routingModuleReactive
                    {
                        ControlManetRouting *ctrlmanet = new ControlManetRouting();
                        ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
                        ctrlmanet->setDestAddress(L3Address(dest));
                        ctrlmanet->setSrcAddress(L3Address(myAddress));
                        frame->encapsulate(msg);
                        ctrlmanet->encapsulate(frame);
                        send(ctrlmanet,"routingOutReactive");
                        return nullptr;
                    }
                    else
                    {
                        if (gateWayAddress == L3Address(dest))
                            dist=1;
                        else
                            dist = 2;
                    }
                }
                else
                {
                    int iface;
                    noRoute = true;
                    double cost;
                    if (!routingModuleReactive->getNextHop(L3Address(dest),add[0],iface,cost)) //send the packet to the routingModuleReactive
                    {
                        ControlManetRouting *ctrlmanet = new ControlManetRouting();
                        ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
                        ctrlmanet->setDestAddress(L3Address(dest));
                        ctrlmanet->setSrcAddress(L3Address(myAddress));
                        frame->encapsulate(msg);
                        ctrlmanet->encapsulate(frame);
                        send(ctrlmanet,"routingOutReactive");
                        return nullptr;
                    }
                    else
                    {
                        if (add[0] == L3Address(dest))
                            dist=1;
                        else
                            dist = 2;
                    }
                }
            }
            else if (routingModuleHwmp) //send the packet to the routingModuleReactive
            {
                  add.resize(1);
                  if (toGateWay)
                  {
                      int iface;
                      noRoute = true;
                      L3Address gateWayAddress;
                      bool isToGt;

                      if (!routingModuleHwmp->getNextHopGroup(L3Address(dest),add[0],iface,gateWayAddress,isToGt)) //send the packet to the routingModuleReactive
                      {
                          frame->encapsulate(msg);
                          if (frame->getControlInfo())
                              delete frame->removeControlInfo();
                          send(frame,"routingOutHwmp");
                          return nullptr;
                      }
                      else
                      {
                          if (gateWayAddress == L3Address(dest))
                              dist=1;
                          else
                              dist = 2;
                      }
                  }
                  else
                  {
                      int iface;
                      noRoute = true;
                      double cost;
                      if (!routingModuleHwmp->getNextHop(L3Address(dest),add[0],iface,cost)) //send the packet to the routingModuleReactive
                      {
                           frame->encapsulate(msg);
                           if (frame->getControlInfo())
                               delete frame->removeControlInfo();

                           send(frame,"routingOutHwmp");
                           return nullptr;
                      }
                      else
                      {
                           if (add[0] == L3Address(dest))
                               dist=1;
                           else
                        	   dist = 2;
                       }
                  }
            }
            else
            {
                delete frame;
                delete msg;
                return nullptr;
            }
        }
        next = add[0].toMAC();
        if (dist >1 && useLwmpls)
        {
            lwmplspk = new LWMPLSPacket(msg->getName());
            lwmplspk->setTTL(maxTTL);
            if (!noRoute)
                lwmplspk->setType(WMPLS_BEGIN_W_ROUTE);
            else
                lwmplspk->setType(WMPLS_BEGIN);

            lwmplspk->setSource(myAddress);
            lwmplspk->setDest(dest);
            if (!noRoute)
            {
                next = add[0].toMAC();
                lwmplspk->setVectorAddressArraySize(dist-1);
                //lwmplspk->setDist(dist-1);
                for (int i=0; i<dist-1; i++)
                    lwmplspk->setVectorAddress(i, add[i].toMAC());
                lwmplspk->setByteLength(lwmplspk->getByteLength()+((dist-1)*6));
            }

            int label_in =mplsData->getLWMPLSLabel();

            /* es necesario introducir el nuevo path en la lista de enlace */
            //lwmpls_initialize_interface(lwmpls_data_ptr,&interface_str_ptr,label_in,sta_addr, ip_address,LWMPLS_INPUT_LABEL);
            /* es necesario ahora introducir los datos en la tabla */
            forwarding_ptr = new LWmpls_Forwarding_Structure();
            forwarding_ptr->input_label=-1;
            forwarding_ptr->return_label_input=label_in;
            forwarding_ptr->return_label_output=-1;
            forwarding_ptr->order=LWMPLS_EXTRACT;
            forwarding_ptr->mac_address= next.getInt();
            forwarding_ptr->label_life_limit=mplsData->mplsMaxTime();
            forwarding_ptr->last_use=simTime();

            forwarding_ptr->path.push_back(myAddress.getInt());
            for (int i=0; i<dist-1; i++)
                forwarding_ptr->path.push_back(add[i].toMAC().getInt());
            forwarding_ptr->path.push_back(dest.getInt());

            mplsData->lwmpls_forwarding_input_data_add(label_in,forwarding_ptr);
            // lwmpls_forwarding_output_data_add(label_out,sta_addr,forwarding_ptr,true);
            /*lwmpls_label_fw_relations (lwmpls_data_ptr,label_in,forwarding_ptr);*/
            lwmplspk->setLabel (label_in);
            mplsData->registerRoute(dest.getInt() , label_in);
        }
    }

    frame->setReceiverAddress(next);
    if (lwmplspk)
    {
        lwmplspk->encapsulate(msg);
        frame->setTTL(lwmplspk->getTTL());
        frame->encapsulate(lwmplspk);
    }
    else
        frame->encapsulate(msg);

    if (frame->getReceiverAddress().isUnspecified())
        ASSERT(!frame->getReceiverAddress().isUnspecified());
    return frame;
}

Ieee80211DataFrame *Ieee80211Mesh::encapsulate(cPacket *msg,MACAddress dest)
{
    Ieee80211MeshFrame *frame = dynamic_cast<Ieee80211MeshFrame*>(msg);
    if (frame==nullptr)
    {
        frame = new Ieee80211MeshFrame(msg->getName());
        frame->setTimestamp(msg->getCreationTime());
        frame->setTTL(maxTTL);
    }

    if (msg->getControlInfo())
        delete msg->removeControlInfo();
    LWMPLSPacket* msgAux = dynamic_cast<LWMPLSPacket*> (msg);
    if (msgAux)
    {
        frame->setTTL(msgAux->getTTL());
    }
    frame->setReceiverAddress(dest);
    if (msg!=frame)
        frame->encapsulate(msg);

    if (frame->getReceiverAddress().isUnspecified())
    {
        char name[50];
        strcpy(name,msg->getName());
        throw cRuntimeError ("Ieee80211Mesh::encapsulate Bad Address");
    }
    if (frame->getReceiverAddress().isBroadcast())
        frame->setTTL(1);
    return frame;
}


void Ieee80211Mesh::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);

    if (obj==nullptr)
        return;

    if (signalID == NF_LINK_BREAK)
    {
        if (obj==nullptr)
            return;
        Ieee80211TwoAddressFrame *frame  = dynamic_cast<Ieee80211TwoAddressFrame *>(obj);
        if (frame)
        {
            MACAddress add = frame->getReceiverAddress();
            mplsBreakMacLink(add);
        }
    }
    else if (signalID == NF_LINK_REFRESH)
    {
        Ieee80211TwoAddressFrame *frame  = check_and_cast<Ieee80211TwoAddressFrame *>(obj);
        if (frame)
            mplsData->lwmpls_refresh_mac (frame->getTransmitterAddress().getInt(), simTime());
    }
}

void Ieee80211Mesh::handleDataFrame(Ieee80211DataFrame *frame)
{
    if (frame->getControlInfo())
        delete frame->removeControlInfo();

    // The message is forward
    if (forwardMessage(frame))
        return;

    MACAddress finalAddress;
    MACAddress source = frame->getTransmitterAddress();
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(frame);
    bool upperPacket = (frame2 && (frame2->getSubType() == UPPERMESSAGE));
    bool isRouting = (frame2 && (frame2->getSubType() == ROUTING));
    short ttl = maxTTL;
    MACAddress destination = frame->getAddress4();
    MACAddress origin = frame->getAddress3();
    int totalHops = -1;
    int totalFixHops = -1;
    if (frame2)
    {
        ttl = frame2->getTTL();
        finalAddress = frame2->getFinalAddress();
        totalHops = frame2->getTotalHops();
        totalFixHops = frame2->getTotalStaticHops();
        EV<<"totalHops"<<totalHops<<endl;
        EV<<"totalFixHops"<< totalFixHops<<endl;

    }



    cPacket *msg = decapsulate(frame);
    ///
    /// If it's a ETX packet to send to the appropriate module
    ///
    if (dynamic_cast<ETXBasePacket*>(msg))
    {
        if (ETXProcess)
        {
            if (msg->getControlInfo())
                delete msg->removeControlInfo();
            send(msg,"ETXProcOut");
        }
        else
            delete msg;
        return;
    }

    LWMPLSPacket *lwmplspk = dynamic_cast<LWMPLSPacket*> (msg);
    mplsData->lwmpls_refresh_mac(source.getInt(), simTime());

    if(isGateWay && lwmplspk)
    {

        if (lwmplspk->getDest()!=myAddress)
        {
            auto it = getGateWayDataMap()->find((L3Address)lwmplspk->getDest());
            if (it!=getGateWayDataMap()->end() && destination != myAddress)
                associatedAddress[lwmplspk->getSource().getInt()] = simTime();
        }
        else
            associatedAddress[lwmplspk->getSource().getInt()] = simTime();
    }

    if (!lwmplspk)
    {
        //cGate * msggate = msg->getArrivalGate();
        //int baseId = gateBaseId("macIn");
        //int index = baseId - msggate->getId();
        if ((routingModuleProactive != nullptr) && (routingModuleProactive->isOurType(msg)))
        {
            //sendDirect(msg,0, routingModule, "ipIn");
            send(msg,"routingOutProactive");
        }
        // else if (dynamic_cast<AODV_msg  *>(msg) || dynamic_cast<DYMO_element  *>(msg))
        else if ((routingModuleReactive != nullptr) && routingModuleReactive->isOurType(msg))
        {
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
            MeshControlInfo *controlInfo = new MeshControlInfo;
            Ieee802Ctrl *ctrlAux = controlInfo;
            *ctrlAux=*ctrl;
            delete ctrl;
            L3Address dest;
            controlInfo->setCollaborativeFeedback(proactiveFeedback);
            controlInfo->setMaxHopCollaborative(maxHopProactive);
            msg->setControlInfo(controlInfo);
            if (routingModuleReactive->getDestAddress(msg, dest))
            {
                std::vector<L3Address>add;
                if (routingModuleProactive && proactiveFeedback)
                {
                    // int neig = routingModuleProactive))->getRoute(src,add);
                    controlInfo->setPreviousFix(true); // This node is fix
                }
                else
                    controlInfo->setPreviousFix(false); // This node is not fix
            }
            send(msg,"routingOutReactive");
        }
        else if (isRouting)
        {
            delete msg;
        }
        else if (dynamic_cast<LocatorPkt *>(msg) != nullptr && hasLocator)
            send(msg, "locatorOut");
        else if (upperPacket)// Normal frame test if upper layer frame in other case delete
        {
            if (hasRelayUnit)
            {
                EthernetIIFrame *ethframe = new EthernetIIFrame(msg->getName()); //TODO option to use EtherFrameWithSNAP instead
                ethframe->setDest(destination);
                ethframe->setSrc(origin);
                ethframe->setEtherType(0);
                ethframe->encapsulate(msg);
                if (ethframe->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                    ethframe->setByteLength(MIN_ETHERNET_FRAME_BYTES);
                sendUp(ethframe);
            }
            else
            {
                if (totalHops >= 0)
                {
                    std::deque<MACAddress> path;
                    int patSize = 0;
                    if (getOtpimunRoute)
                    {
                        if (getOtpimunRoute->findRoute(120,origin,path))
                        {
                            patSize = path.size();
                            emit(numHopsSignal,totalHops - patSize);
                        }
                    }
                    if(totalHops)
                        emit(numFixHopsSignal,totalFixHops/totalHops);
                }
                sendUp(msg);
            }
        }
        else
            delete msg;
        return;
    }
    lwmplspk->setTTL(ttl);
    mplsDataProcess(lwmplspk,source);
}

void Ieee80211Mesh::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    if (!hasSecurity)
    {
        dropManagementFrame(frame);
        return;
    }
    EV << "Authentication from MAC, send it to SecurityModule" << frame << endl;
    send(frame, "securityOut");
}
void Ieee80211Mesh::handleCCMPFrame(CCMPFrame *frame)
{
    if (!hasSecurity)
    {
        delete frame;
        return;
    }
    EV << "CCMP Frame from MAC, send it to SecurityModule" << frame << endl;
    send(frame, "securityOut");
}

void Ieee80211Mesh::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    if (!hasSecurity)
    {
        dropManagementFrame(frame);
        return;
    }
    EV << "Beacon from MAC, send it to SecurityModule" << frame << endl;
    send(frame, "securityOut");
}

void Ieee80211Mesh::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}


void Ieee80211Mesh::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211Mesh::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}

// Cada vez que se envia un mensaje sirve para generar mensajes de permanencia. usa los propios hellos para garantizar que se env�an mensajes

void Ieee80211Mesh::sendOut(cMessage *msg)
{
    //send(msg, macBaseGateId + ie->getNetworkLayerGateIndex());
    int macOut = 0;
    if (msg->hasPar("indexGate"))
    {
        macOut = msg->par("indexGate");
        delete msg->removeObject("indexGate");
    }

    Ieee80211MeshFrame *frameMesh = dynamic_cast<Ieee80211MeshFrame*>(msg);
    if (frameMesh && frameMesh->getSubType() == ROUTING)
        numRoutingBytes += frameMesh->getByteLength();
    else if (dynamic_cast<Ieee80211ActionMeshFrame *>(msg))
    {
        Ieee80211ActionMeshFrame *hwmpFrame = dynamic_cast<Ieee80211ActionMeshFrame *>(msg);
        numRoutingBytes += hwmpFrame->getByteLength();
    }
    else if (frameMesh && frameMesh->getSubType() == UPPERMESSAGE)
        numDataBytes += frameMesh->getByteLength();
    //mhn
    if(hasSecurity)
    {
        if (msg->arrivedOn("securityIn"))
        {
            if (isMultiMac)
                 send(msg, "macOutMulti",macOut);
            else
                 send(msg, "macOut");
        }
        else  if (dynamic_cast<CCMPFrame *>(msg))
        {
            EV << "CCMPFrame Frame arrived from Security, send it to Mac" <<endl;
            error("mhn");
            if (isMultiMac)
                 send(msg, "macOutMulti",macOut);
            else
                 send(msg, "macOut");
        }
        else
        {
            send(msg, "securityOut");
        }
    }
    else
    {
        if (isMultiMac)
            send(msg, "macOutMulti",macOut);
        else
            send(msg, "macOut");

    }
}


//
// mac label address method
// Equivalent to the 802.11s forwarding mechanism
//

bool Ieee80211Mesh::forwardMessage (Ieee80211DataFrame *frame)
{
    cPacket *msg = frame->getEncapsulatedPacket();
    LWMPLSPacket *lwmplspk = dynamic_cast<LWMPLSPacket*> (msg);

    if (lwmplspk)
        return false;
    if ((routingModuleProactive != nullptr) && (routingModuleProactive->isOurType(msg)))
        return false;
    else if ((routingModuleReactive != nullptr) && routingModuleReactive->isOurType(msg))
        return false;
    else // Normal frame test if use the mac label address method
        return macLabelBasedSend(frame);

}

bool Ieee80211Mesh::macLabelBasedSend(Ieee80211DataFrame *frame)
{

    if (!frame)
        return false;

    if (isGateWay)
    {
        // if this is gateway and the frame is for other gateway send it
        Ieee80211MeshFrame * frame2 = dynamic_cast<Ieee80211MeshFrame*>(frame);
        if (frame2 && !frame2->getFinalAddress().isUnspecified() && frame2->getFinalAddress()!=frame->getAddress4())
            frame2->setAddress4(frame2->getFinalAddress());

        bool toGateWay=false;
        if (routingModuleReactive)
        {
            if (routingModuleReactive->findInAddressGroup(L3Address(frame->getAddress4())))
                toGateWay = true;
        }
        else if (routingModuleProactive)
        {
            if (routingModuleProactive->findInAddressGroup(L3Address(frame->getAddress4())))
                 toGateWay = true;
        }
        else if (routingModuleHwmp)
        {
            if (routingModuleHwmp->findInAddressGroup(L3Address(frame->getAddress4())))
                 toGateWay = true;
        }

        if (toGateWay)
            associatedAddress[frame2->getAddress3().getInt()]=simTime();
        if (toGateWay && !isAddressForUs(frame->getAddress4()))
        {
            frame2->setTransmitterAddress(myAddress);
            if (!frame2->getReceiverAddress().isBroadcast())
                frame2->setReceiverAddress(frame->getAddress4());
            sendFrameDown(frame2);
            return true;
        }
    }

    if (frame->getAddress4().isUnspecified())
        return false;
    if (isAddressForUs(frame->getAddress4()))
        return false;

    MACAddress dest = frame->getAddress4();
    MACAddress src = frame->getAddress3();
    MACAddress prev = frame->getTransmitterAddress();
    MACAddress next = MACAddress(mplsData->getForwardingMacKey(src.getInt(),dest.getInt(),prev.getInt()));
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(frame);

    double  delay = SIMTIME_DBL(simTime() - frame->getTimestamp());
    if ((frame2 && frame2->getTTL()<=0) || (delay > limitDelay))
    {
        delete frame;
        return true;
    }

    if (!next.isUnspecified())
    {
        frame->setReceiverAddress(MACAddress(next));
    }
    else
    {
        std::vector<L3Address> add;
        int dist=0;
        int iface;

        bool toGateWay=false;
        if (routingModuleReactive)
        {
            if (routingModuleReactive->findInAddressGroup(L3Address(dest)))
                toGateWay = true;
        }
        else if (routingModuleProactive)
        {
            if (routingModuleProactive->findInAddressGroup(L3Address(dest)))
                 toGateWay = true;
        }
        else if (routingModuleHwmp)
        {
            if (routingModuleHwmp->findInAddressGroup(L3Address(dest)))
                 toGateWay = true;
        }
        L3Address gateWayAddress;
        if (routingModuleProactive)
        {
             add.resize(1);
            if (toGateWay)
            {
                bool isToGw;
                if (routingModuleProactive->getNextHopGroup(L3Address(dest),add[0],iface,gateWayAddress,isToGw))
                   dist = 1;
            }
            else
            {
                double cost;
                if (routingModuleProactive->getNextHop(L3Address(dest),add[0],iface,cost))
                   dist = 1;
            }
        }

        if (dist==0 && routingModuleReactive)
        {
            add.resize(1);
            if (toGateWay)
            {
                bool isToGw;
                if (routingModuleReactive->getNextHopGroup(L3Address(dest),add[0],iface,gateWayAddress,isToGw))
                   dist = 1;
            }
            else
            {
                double cost;
                if (routingModuleReactive->getNextHop(L3Address(dest),add[0],iface,cost))
                   dist = 1;
            }
        }

        if (routingModuleHwmp) //send the packet to the routingModuleReactive
        {
            add.resize(1);
            if (toGateWay)
            {
                bool isToGw;
                if (routingModuleHwmp->getNextHopGroup(L3Address(dest),add[0],iface,gateWayAddress,isToGw))
                   dist = 1;
            }
            else
            {
                 double cost;
                 if (routingModuleHwmp->getNextHop(L3Address(dest),add[0],iface,cost)) //send the packet to the routingModuleReactive
                    dist=1;
            }
        }
        if (dist==0)
        {
// Destination unreachable
            if (routingModuleReactive)
            {
                ControlManetRouting *ctrlmanet = new ControlManetRouting();
                ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
                ctrlmanet->setDestAddress(L3Address(dest));
                //  ctrlmanet->setSrcAddress(myAddress);
                ctrlmanet->setSrcAddress(L3Address(src));
                ctrlmanet->encapsulate(frame);
                frame = nullptr;
                send(ctrlmanet,"routingOutReactive");
            }
            if (routingModuleHwmp)
            {
                if (frame->getControlInfo())
                    delete frame->removeControlInfo();
                send(frame,"routingOutHwmp");
                frame = nullptr;
            }
            else
            {
                delete frame;
                frame=nullptr;
            }
        }
        else
        {
            frame->setReceiverAddress(add[0].toMAC());
        }

    }
    //send(msg, macBaseGateId + ie->getNetworkLayerGateIndex());
    if (frame)
        sendFrameDown(frame);
    return true;
}


cPacket *Ieee80211Mesh::decapsulate(Ieee80211DataFrame *frame)
{
    cPacket *payload = frame->decapsulate();
    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    ctrl->setSrc(frame->getTransmitterAddress());
    ctrl->setDest(frame->getReceiverAddress());
    payload->setControlInfo(ctrl);
    delete frame;
    return payload;
}

void Ieee80211Mesh::actualizeReactive(cPacket *pkt,bool out)
{
    L3Address dest,next;
    if (!routingModuleReactive && !routingModuleHwmp)
        return;

    Ieee80211DataFrame * frame = dynamic_cast<Ieee80211DataFrame*>(pkt);

    if (!frame )
        return;
/*
    if (!out)
        return;
*/

    bool isReverse=false;
    if (out)
    {
        if (!frame->getAddress4().isUnspecified() && !frame->getAddress4().isBroadcast())
            dest = L3Address(frame->getAddress4());
        if (!frame->getReceiverAddress().isUnspecified() && !frame->getReceiverAddress().isBroadcast())
            next = L3Address(frame->getReceiverAddress());
        else
            return;

    }
    else
    {
        if (!frame->getAddress3().isUnspecified() && !frame->getAddress3().isBroadcast())
            dest = L3Address(frame->getAddress3());
        if (!frame->getTransmitterAddress().isUnspecified() && !frame->getTransmitterAddress().isBroadcast())
            next = L3Address(frame->getTransmitterAddress());
        else
            return;
        isReverse=true;
    }

    if (routingModuleHwmp && dest != L3Address())
        routingModuleHwmp->setRefreshRoute(dest,next,isReverse);
    if (routingModuleReactive && dest != L3Address())
        routingModuleReactive->setRefreshRoute(dest,next,isReverse);
    // actualize route to neighbor
    if (routingModuleHwmp)
        routingModuleHwmp->setRefreshRoute(next,next,isReverse);
    if (routingModuleReactive)
        routingModuleReactive->setRefreshRoute(next,next,isReverse);
}


void Ieee80211Mesh::sendFrameDown(cPacket *frame)
{
    Ieee80211MeshFrame * frameAux = dynamic_cast<Ieee80211MeshFrame*>(frame);
    Ieee80211DataOrMgmtFrame *frameDataorMgm = dynamic_cast <Ieee80211DataOrMgmtFrame*> (frame);
    if (frameAux && frameAux->getTTL() <=0 )
    {
        delete frame;
        return;
    }
    if (frameDataorMgm)
        if (isSendToGateway(frameDataorMgm))
            return;
    actualizeReactive(frame,true);
    if (frame->getControlInfo())
        delete frame->removeControlInfo();
    if (frameDataorMgm->getReceiverAddress().isBroadcast())
    {
        if (isMultiMac)
        {
            for (unsigned int i = 1; i < numMac; i++)
            {
                cPacket *pkt = frame->dup();
                sendDownMulti(pkt, i);
            }
            sendDownMulti(frame, 0);
            /*
             if (inteligentBroadcastRouting && (frameAux && frameAux->getSubType() == ROUTING))
             {
             frameAux->setChannelsArraySize(numMac);
             for (unsigned int i = 0; i < numMac; i++)
             {
             frameAux->setChannels(i,radioInterfaces[i]->getChannel());
             }
             ///// CUIDADO : FIXME
             if (dynamic_cast<OLSR_pkt*>(frame->getEncapsulatedPacket()) && routingModuleProactive && routingModuleReactive)
             frame->setKind(1);
             }
             else
             {
             for (unsigned int i = 1; i < numMac; i++)
             {
             cPacket *pkt = frame->dup();
             pkt->setKind(i);
             PassiveQueueBase::handleMessage(pkt);
             }
             }*/
        }
        else
            sendDown(frame);
    }
    else
    {
        if (isMultiMac)
        {
            int index =  getBestInterface(frameDataorMgm);
            sendDownMulti(frame,index);
        }
        else
            sendDown(frame);
    }
}

bool Ieee80211Mesh::isSendToGateway(Ieee80211DataOrMgmtFrame *frame)
{
    if (!isGateWay)
        return false;
    if (frame == nullptr)
        return false;
    if (isGateWay)
    {
        if (frame->getControlInfo() == nullptr || !dynamic_cast<MeshControlInfo*>(frame->getControlInfo()))
        {
            GateWayDataMap::iterator it;
            frame->setTransmitterAddress(myAddress);
            if (frame->getReceiverAddress().isBroadcast())
            {
                MACAddress origin;
                if (dynamic_cast<LWMPLSPacket*>(frame->getEncapsulatedPacket()))
                {
                    int code = dynamic_cast<LWMPLSPacket*>(frame->getEncapsulatedPacket())->getType();
                    if (code == WMPLS_BROADCAST
                            || code == WMPLS_ANNOUNCE_GATEWAY
                            || code == WMPLS_REQUEST_GATEWAY)
                        origin = dynamic_cast<LWMPLSPacket*>(frame->getEncapsulatedPacket())->getSource();
                }
                for (it = getGateWayDataMap()->begin(); it != getGateWayDataMap()->end(); it++)
                {
                    if (it->second.idAddress == myAddress || it->second.idAddress == origin)
                        continue;
                    MeshControlInfo *ctrl = new MeshControlInfo();
                    ctrl->setSrc(MACAddress::UNSPECIFIED_ADDRESS); // the Ethernet will fill the field
                    //ctrl->setDest(frameAux->getReceiverAddress());
                    ctrl->setDest(it->second.ethAddress);
                    cPacket *pktAux = frame->dup();
                    pktAux->setControlInfo(ctrl);
                    //sendDirect(pktAux,it->second.gate);
                    send(pktAux, "toEthernet");
                }
            }
            else
            {
                if (dynamic_cast<Ieee80211DataFrame*>(frame))
                    it = getGateWayDataMap()->find(L3Address(dynamic_cast<Ieee80211DataFrame*>(frame)->getAddress4()));
                else
                    it = getGateWayDataMap()->end();
                if (it != getGateWayDataMap()->end())
                {
                    MeshControlInfo *ctrl = new MeshControlInfo();
                    ctrl->setSrc(MACAddress::UNSPECIFIED_ADDRESS); // the Ethernet will fill the field
                    //ctrl->setDest(frameAux->getReceiverAddress());
                    ctrl->setDest(it->second.ethAddress);
                    if (frame->getControlInfo())
                        delete frame->removeControlInfo();
                    frame->setControlInfo(ctrl);
                    actualizeReactive(frame, true);
                    // The packet are fragmented in EtherEncapMesh
                    //sendDirect(frameAux,5e-6,frameAux->getBitLength()/1e9,it->second.gate);
                    send(frame, "toEthernet");
                    return true;
                }
                it = getGateWayDataMap()->find(L3Address(frame->getReceiverAddress()));
                if (it != getGateWayDataMap()->end())
                {
                    MeshControlInfo *ctrl = new MeshControlInfo();
                    //ctrl->setSrc(myAddress);
                    ctrl->setSrc(MACAddress::UNSPECIFIED_ADDRESS); // the Ethernet will fill the field
                    //ctrl->setDest(frameAux->getReceiverAddress());
                    ctrl->setDest(it->second.ethAddress);
                    if (frame->getControlInfo())
                        delete frame->removeControlInfo();
                    frame->setControlInfo(ctrl);
                    actualizeReactive(frame, true);
                    // sendDirect(frameAux,5e-6,frameAux->getBitLength()/1e9,it->second.gate);
                    send(frame, "toEthernet");
                    return true;
                }
            }
        }
    }
    return false;
}

int Ieee80211Mesh::getBestInterface(Ieee80211DataOrMgmtFrame *frame)
{
    if (numMac<=1)
        return 0;

    if (selectionCriteria == ETX)
    {
        std::multimap<double,int> cost;
        for (unsigned int i = 0; i < numMac; i++)
        {
            double val = ETXProcess->getEtx(frame->getReceiverAddress(),(int)i);
            if (val == -1)
                continue;
            cost.insert(std::pair<double,int>(val,i));
        }
        if (cost.empty())
            return 0;
        if (cost.size() == 1)
            return cost.begin()->second;
        double val = cost.begin()->first;
        int index = 0;
        auto it = cost.begin();
        auto itaux = cost.begin();
        itaux++;
        while (val == it->first && itaux != cost.end())
        {
            index++;
            it = itaux;
            itaux++;
        }

        int iface = frame->par("indexGate");
        frame->getParList().remove("indexGate");

        if (index == 0)
        {
            if (iface != it->second)
                return it->second;
            else
            {
                it = cost.begin();
                it++;
                return it->second; // avoid the intra frame interference
                //if ((val/it->first)<0.9)
                //    return it->second;
            }
        }
        else
        {
            // select randomly
            do
            {
                int i = intuniform(0,index);
                it = cost.begin();
                while (i)
                {
                    ++it;
                    i--;
                }
            } while (it->second == iface);
            return it->second;
        }
    }
    else
    {
        return intuniform(0,numMac-1);
    }
#if 0 // no funciona con los nuevos interfaces repasar como sacar la informacion
    else if (!timeReceptionInterface.empty())
    {
        std::vector<double> validInterface;
        unsigned int queueSize = 1000;
        double recent = 300;
        int bestQueue = -1;
        int bestTime = -1;
        validInterface.resize(timeReceptionInterface.size());
        for (int i = (int)timeReceptionInterface.size() - 1; i >=0 ; i--)
        {
            validInterface[i] = -1;
            auto it = timeReceptionInterface[i].find(frame->getReceiverAddress());
            if (it == timeReceptionInterface[i].end())
                continue;
            double lastMessageReceived = SIMTIME_DBL(simTime() - it->second);
            if (lastMessageReceived > par("lifeTimeForInterface").doubleValue())
                timeReceptionInterface[i].erase(it);  // erase
            else
                validInterface[i] = lastMessageReceived;
            if (macInterfaces[i]->getState() == Ieee80211Mac::IDLE)
                return i;
            int macOut = 0;
            if (frame->hasPar("indexGate"))
                macOut = frame->par("indexGate");

            if(queueSize >  macInterfaces[i]->getQueueSize() || (queueSize ==  macInterfaces[i]->getQueueSize() && bestQueue == macOut && i != bestQueue))
            {
                queueSize = macInterfaces[i]->getQueueSize();
                bestQueue = i;
            }
            else if (selectionCriteria == MINQUEUELASTUSED && queueSize &&  macInterfaces[i]->getQueueSize() && recent > lastMessageReceived)
            {
                queueSize = macInterfaces[i]->getQueueSize();
                bestQueue = i;
            }


            if (recent > lastMessageReceived)
            {
                recent = lastMessageReceived;
                bestTime = i;
            }
            else if (selectionCriteria == LASTUSEDMINQUEUE && abs(recent-lastMessageReceived) < 0.01  && queueSize >  macInterfaces[i]->getQueueSize())
            {
                recent = lastMessageReceived;
                bestTime = i;
            }
        }

        if (selectionCriteria == MINQUEUE || selectionCriteria == MINQUEUELASTUSED)
        {
            if (bestQueue >= 0 && validInterface[bestQueue] >= 0)
                return bestQueue;
            else
                return 0;
        }
        else if (selectionCriteria == LASTUSED || selectionCriteria == LASTUSEDMINQUEUE)
        {
            if (bestTime >= 0 && validInterface[bestTime] >= 0)
                return bestTime;
            else
                return 0;
        }
        else
            throw cRuntimeError("Invalid selectionCriteria");

    }
#endif
    return 0;
}

#if 0
void Ieee80211Mesh::sendOrEnqueue(cPacket *frame)
{
    Ieee80211MeshFrame * frameAux = dynamic_cast<Ieee80211MeshFrame*>(frame);
    if (frameAux && frameAux->getTTL()<=0)
    {
        delete frame;
        return;
    }
    // Check if the destination is other gateway if true send to it
    if (isGateWay)
    {
        if (frameAux &&  (frame->getControlInfo()==nullptr || !dynamic_cast<MeshControlInfo*>(frame->getControlInfo())))
        {
            auto it;
            if (frameAux->getReceiverAddress().isBroadcast())
            {
                MACAddress origin;
                if (dynamic_cast<LWMPLSPacket*> (frameAux->getEncapsulatedPacket()))
                {
                    int code = dynamic_cast<LWMPLSPacket*> (frameAux->getEncapsulatedPacket())->getType();
                    if (code==WMPLS_BROADCAST || code == WMPLS_ANNOUNCE_GATEWAY || code== WMPLS_REQUEST_GATEWAY)
                        origin=dynamic_cast<LWMPLSPacket*> (frameAux->getEncapsulatedPacket())->getSource();
                }
                for (it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
                {
                    if (it->second.idAddress==myAddress || it->second.idAddress==origin)
                        continue;
                    MeshControlInfo *ctrl = new MeshControlInfo();
                    ctrl->setSrc(myAddress);
                    ctrl->setDest(frameAux->getReceiverAddress());
                    cPacket *pktAux=frameAux->dup();
                    pktAux->setControlInfo(ctrl);
                    //sendDirect(pktAux,it->second.gate);
                    sendDirect(pktAux,5e-6,pktAux->getBitLength()/1e9,it->second.gate);
                }
            }
            else
            {
                it = getGateWayDataMap()->find((L3Address)frameAux->getAddress4());
                if (it!=getGateWayDataMap()->end())
                {
                    MeshControlInfo *ctrl = new MeshControlInfo();
                    ctrl->setSrc(myAddress);
                    ctrl->setDest(frameAux->getReceiverAddress());
                    frameAux->setControlInfo(ctrl);
                    actualizeReactive(frameAux,true);
                    //sendDirect(frameAux,it->second.gate);
                    sendDirect(frameAux,5e-6,frameAux->getBitLength()/1e9,it->second.gate);
                    return;
                }
                it = getGateWayDataMap()->find((L3Address)frameAux->getReceiverAddress());
                if (it!=getGateWayDataMap()->end())
                {
                    MeshControlInfo *ctrl = new MeshControlInfo();
                    ctrl->setSrc(myAddress);
                    ctrl->setDest(frameAux->getReceiverAddress());
                    frameAux->setControlInfo(ctrl);
                    actualizeReactive(frameAux,true);
                    //sendDirect(frameAux,it->second.gate);
                    sendDirect(frameAux,5e-6,frameAux->getBitLength()/1e9,it->second.gate);
                    return;
                }
            }
        }
    }
    actualizeReactive(frame,true);
    PassiveQueueBase::handleMessage(frame);
}
#endif

void Ieee80211Mesh::handleEtxMessage(cPacket *pk)
{
    ETXBasePacket * etxMsg = dynamic_cast<ETXBasePacket*>(pk);
    if (etxMsg)
    {
        Ieee80211DataFrame * frame = encapsulate(etxMsg,etxMsg->getDest());
        if (!frame->hasPar("indexGate"))
            frame->addPar("indexGate") = etxMsg->par("indexGate");
        else
            frame->par("indexGate") = etxMsg->par("indexGate");
        if (frame)
            sendFrameDown(frame);
    }
    else
        delete pk;
}

void Ieee80211Mesh::publishGateWayIdentity()
{
    LWMPLSControl * pkt = new LWMPLSControl();
#ifndef CHEAT_IEEE80211MESH
    cGate * gt=gate("interGateWayConect");
    unsigned char *ptr;
    pkt->setGateAddressPtrArraySize(sizeof(ptr));
    pkt->setAssocAddressPtrArraySize(sizeof(ptr));
    ptr = (unsigned char*)gt;
    for (unsigned int i=0;i<sizeof(ptr);i++)
        pkt->setGateAddressPtr(i,ptr[i]);
    ptr=(unsigned char*) &associatedAddress;
    for (unsigned int i=0;i<sizeof(ptr);i++)
        pkt->setGateAddressPtr(i,ptr[i]);
#endif
    // copy receiver address from the control info (sender address will be set in MAC)
    pkt->setType(WMPLS_ANNOUNCE_GATEWAY);
    Ieee80211MeshFrame *frame = new Ieee80211MeshFrame();
    frame->setReceiverAddress(MACAddress::BROADCAST_ADDRESS);
    frame->setTTL(1);
    uint32_t cont;
    mplsData->getBroadCastCounter(cont);
    cont++;
    mplsData->setBroadCastCounter(cont);
    pkt->setCounter(cont);
    pkt->setSource(myAddress);
    pkt->setDest(MACAddress::BROADCAST_ADDRESS);
    pkt->setType(WMPLS_ANNOUNCE_GATEWAY);
    if (this->getGateWayDataMap() && this->getGateWayDataMap()->size()>0)
    {
        int size = this->getGateWayDataMap()->size();
        pkt->setVectorAddressArraySize(size);
        cont = 0;
        for (auto it=this->getGateWayDataMap()->begin();it!=this->getGateWayDataMap()->end();it++)
        {
            pkt->setVectorAddress(cont,it->second.idAddress);
            cont++;
            pkt->setByteLength(pkt->getByteLength()+6); // 40 bits address
        }
    }
    frame->encapsulate(pkt);
    double delay=this->getGateWayDataMap()->size()*par("GateWayAnnounceInterval").doubleValue ();
    scheduleAt(simTime()+delay+uniform(0,2),gateWayTimeOut);
    if (frame)
        sendFrameDown(frame);
}


void Ieee80211Mesh::processControlPacket (LWMPLSControl *pkt)
{
#ifndef CHEAT_IEEE80211MESH
    if (getGateWayData())
    {
        GateWayData data;
        unsigned char *ptr;
        for (unsigned int i=0;i<sizeof(ptr);i++)
            pkt->getGateAddressPtr(i,ptr[i]);
        data.gate=(cGate*)ptr;
        for (unsigned int i=0;i<sizeof(ptr);i++)
            pkt->getGateAddressPtr(i,ptr[i]);
        data.associatedAddress= (AssociatedAddress *)ptr;
        getGateWayData()->insert(std::pair<L3Address,GateWayData>(pkt->getSource(),data));
    }
#endif
    for (unsigned int i=0;i<pkt->getVectorAddressArraySize();i++)
    {
        MACAddress addI = MACAddress(pkt->getVectorAddress(i));
        if(routingModuleProactive)
            routingModuleProactive->addInAddressGroup(L3Address(addI));
        if (routingModuleReactive)
            routingModuleReactive->addInAddressGroup(L3Address(addI));
    }
}

bool Ieee80211Mesh::selectGateWay(const L3Address &dest,MACAddress &gateway)
{
    if (!isGateWay)
        return false;
#ifdef CHEAT_IEEE80211MESH
    auto best=this->getGateWayDataMap()->end();
    double bestCost;
    double myCost=10E10;
    simtime_t timeAsoc=0;
    for (auto it=this->getGateWayDataMap()->begin();it!=this->getGateWayDataMap()->end();it++)
    {
        int iface;
        L3Address next;
        if (best==this->getGateWayDataMap()->end())
        {
            bool destinationFind=false;
            if(it->second.proactive && it->second.proactive->getNextHop(dest,next,iface,bestCost))
                destinationFind=true;
            else if(it->second.reactive && it->second.reactive->getNextHop(dest,next,iface,bestCost))
                destinationFind=true;
            if (destinationFind)
            {
                if (it->second.idAddress==myAddress)
                    myCost=bestCost;
                best=it;
                auto itAssoc = best->second.associatedAddress->find(dest.toMAC().getInt());
                if (itAssoc!=best->second.associatedAddress->end())
                    timeAsoc=itAssoc->second;
            }
        }
        else
        {
            int iface;
            L3Address next;
            double cost;
            bool destinationFind=false;

            if(it->second.proactive && it->second.proactive->getNextHop(dest,next,iface,cost))
                destinationFind=true;
            else if(it->second.reactive && it->second.reactive->getNextHop(dest,next,iface,cost))
                destinationFind=true;
            if (destinationFind)
            {
                if (it->second.idAddress==myAddress)
                    myCost=cost;
                if (cost<bestCost)
                {
                    best=it;
                    bestCost=cost;
                    best=it;
                    auto itAssoc = best->second.associatedAddress->find(dest.toMAC().getInt());
                    if (itAssoc!=best->second.associatedAddress->end())
                        timeAsoc=itAssoc->second;
                    else
                        timeAsoc=0;
                }
                else if (cost==bestCost)
                {
                    auto itAssoc = best->second.associatedAddress->find(dest.toMAC().getInt());
                    if (itAssoc!=best->second.associatedAddress->end())
                    {
                        if (timeAsoc==0 || timeAsoc>itAssoc->second)
                        {
                               best=it;
                               timeAsoc=itAssoc->second;
                        }
                    }
                }
            }
        }
    }
    if (best!=this->getGateWayDataMap()->end())
    {
        // check my address
        if (myCost<=bestCost && (timeAsoc==0 || timeAsoc>10))
            gateway=myAddress;
        else
            gateway=best->second.idAddress;
        return true;
    }
    else
        return false;
#endif
}
//
// TODO : Hacer que los gateway se comporten como un gran nodo �nico. Si llega un rreq este lo retransmite no solo �l sino tambien los otros, f�cil en reactivo
// necesario pensar en proactivo.
//
void Ieee80211Mesh::handleWateGayDataReceive(cPacket *pkt)
{

    if (dynamic_cast<Ieee80211ActionMeshFrame *>(pkt))
    {
        if ((routingModuleHwmp != nullptr) && routingModuleHwmp->isOurType(pkt)) {
            if (pkt->getControlInfo())
                delete pkt->removeControlInfo();
            send(pkt,"routingOutHwmp");
        }
        else
            delete pkt;
        return;
    }
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(pkt);
    cPacket *encapPkt=nullptr;
    encapPkt = pkt->getEncapsulatedPacket();
    if ((routingModuleProactive != nullptr) && (routingModuleProactive->isOurType(encapPkt)))
    {
        //sendDirect(msg,0, routingModule, "ipIn");
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(pkt->removeControlInfo());
        encapPkt = pkt->decapsulate();
        MeshControlInfo *controlInfo = new MeshControlInfo;
        Ieee802Ctrl *ctrlAux = controlInfo;
        *ctrlAux=*ctrl;
        if (frame2->getReceiverAddress().isUnspecified())
            throw cRuntimeError("transmitter address is unspecified");
        else if (frame2->getReceiverAddress() != myAddress && !frame2->getReceiverAddress().isBroadcast())
            throw cRuntimeError("bad address");
        else
            controlInfo->setDest(frame2->getReceiverAddress());
        if (getGateWayDataMap()==nullptr)
            throw cRuntimeError("error GateWayMap not found");
        for (auto it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
        {
            if (ctrl->getSrc()==it->second.ethAddress)
                controlInfo->setSrc(it->first.toMAC());
        }
        delete ctrl;
        encapPkt->setControlInfo(controlInfo);
        send(encapPkt,"routingOutProactive");
        delete pkt;
        return;
    }
    else if ((routingModuleReactive != nullptr) && routingModuleReactive->isOurType(encapPkt))
    {
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(pkt->removeControlInfo());
        encapPkt = pkt->decapsulate();
        MeshControlInfo *controlInfo = new MeshControlInfo;
        Ieee802Ctrl *ctrlAux = controlInfo;
        *ctrlAux=*ctrl;
        if (frame2->getReceiverAddress().isUnspecified())
            throw cRuntimeError("transmitter address is unspecified");
        else if (frame2->getReceiverAddress() != myAddress && !frame2->getReceiverAddress().isBroadcast())
            throw cRuntimeError("bad address");
        else
            controlInfo->setDest(frame2->getReceiverAddress());
        if (getGateWayDataMap()==nullptr)
            throw cRuntimeError("error GateWayMap not found");

        for (auto it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
        {
            if (ctrl->getSrc()==it->second.ethAddress)
                controlInfo->setSrc(it->first.toMAC());
        }
        delete ctrl;
        L3Address dest;
        encapPkt->setControlInfo(controlInfo);
        if (routingModuleReactive->getDestAddress(encapPkt,dest))
        {
            std::vector<L3Address>add;
            if (routingModuleProactive && proactiveFeedback)
            {
                // int neig = routingModuleProactive))->getRoute(src,add);
                controlInfo->setPreviousFix(true); // This node is fix
            }
            else
                controlInfo->setPreviousFix(false); // This node is not fix
        }
        send(encapPkt,"routingOutReactive");
        delete pkt;
        return;
    }

    if(isGateWay && frame2)
    {
        if (frame2->getFinalAddress()==myAddress)
        {
            bool isUpper = (frame2->getSubType() == UPPERMESSAGE);
            int totalHops = frame2->getTotalHops();
            int totalFixHops = frame2->getTotalStaticHops();
            MACAddress origin = frame2->getAddress3();

            cPacket *msg = decapsulate(frame2);
            if (dynamic_cast<ETXBasePacket*>(msg))
            {
                if (ETXProcess)
                {
                    if (msg->getControlInfo())
                        delete msg->removeControlInfo();
                    send(msg,"ETXProcOut");
                }
                else
                    delete msg;
                return;
            }
            else if (dynamic_cast<LWMPLSPacket*> (msg))
            {
                LWMPLSPacket *lwmplspk = dynamic_cast<LWMPLSPacket*> (msg);
                encapPkt = decapsulateMpls(lwmplspk);
            }
            else if (dynamic_cast<LocatorPkt *>(msg) != nullptr && hasLocator)
                send(msg, "locatorOut");
            else
                encapPkt = msg;

            if (encapPkt && isUpper)
            {
                sendUp(encapPkt);
                std::deque<MACAddress> path;
                int patSize = 0;
                if (getOtpimunRoute)
                {
                    if (getOtpimunRoute->findRoute(120,origin,path))
                    {
                        getOtpimunRoute->findRoute(120,origin,path);
                        patSize = path.size();
                        emit(numHopsSignal,totalHops - patSize);
                    }
                }


                emit(numFixHopsSignal,totalFixHops/totalHops);
            }
        }
        else if (!frame2->getFinalAddress().isUnspecified())
        {
            if (frame2->getControlInfo())
                delete frame2->removeControlInfo();
            handleReroutingGateway(frame2);
        }
        else
        {
            if (frame2->getControlInfo())
                delete frame2->removeControlInfo();
            frame2->setTTL(frame2->getTTL()-1);
            actualizeReactive(frame2,false);
            processFrame(frame2);
        }
    }
    else
        delete pkt;
}

void Ieee80211Mesh::handleReroutingGateway(Ieee80211DataFrame *pkt)
{
    handleDataFrame(pkt);
}

bool Ieee80211Mesh::isAddressForUs(const MACAddress &add)
{
    if (routingModuleReactive)
        return  routingModuleReactive->addressIsForUs(L3Address(add));
    else if (routingModuleProactive)
        return routingModuleProactive->addressIsForUs(L3Address(add));
    else if (routingModuleHwmp)
        return routingModuleHwmp->addressIsForUs(L3Address(add));
    else if (add==myAddress)
        return true;
    else
        return false;
}

bool Ieee80211Mesh::getCostNode(const MACAddress &add, unsigned int &cost)
{
/*
    if (routingModuleProactive)
    {
         routingModuleProactive->getNextHop();
    }
    if (routingModuleReactive)
    {
        return  routingModuleReactive->addressIsForUs(add.getInt());
    }
    if (routingModuleHwmp)
    {
        return routingModuleHwmp->addressIsForUs(add.getInt());
    }

    else if (add==myAddress)
        return true;
    else
    */
        return false;
}

void Ieee80211Mesh::finish()
{
    recordScalar("bytes routing ", numRoutingBytes);
    recordScalar("bytesData Sent ",numDataBytes);
}


int Ieee80211Mesh::findSeqNum(const L3Address &addr, const uint64_t &sqnum)
{
    auto it = seqNumberInfo.find(addr);
    if (it == seqNumberInfo.end())
        return 0;
    if (it->second.back().getSeqNum() < sqnum)
        return 0;
    for (unsigned int i = 0; i < it->second.size(); i++)
    {
        if (it->second[i].getSeqNum() ==  sqnum)
            return it->second[i].getNumTimes();

    }
    return -1; // too old
}

bool Ieee80211Mesh::setSeqNum(const L3Address &addr, const uint64_t &sqnum, const int &numTimes)
{
    auto it = seqNumberInfo.find(addr);
    if (it == seqNumberInfo.end())
    {
        SeqNumberData sinfo(sqnum,numTimes);
        SeqNumberVector v;
        v.push_back(sinfo);
        seqNumberInfo[addr] = v;
        return true;
    }

    if (it->second.back().getSeqNum()<sqnum)
    {
        if (MaxSeqNum == 1)
        {
            it->second[0].setSeqNum(sqnum);
            return true;
        }
        SeqNumberData sinfo(sqnum,numTimes);
        SeqNumberVector v;
        it->second.push_back(sinfo);
        if ((int)it->second.size() > MaxSeqNum)
            it->second.pop_front();
        return true;
    }
    if (it->second.front().getSeqNum()>sqnum)
        return false;

    for (unsigned int i = 0; i < it->second.size(); i++)
    {
        if (it->second[i].getSeqNum() ==  sqnum)
        {
            it->second[i].setNumTimes(numTimes);
            return true;
        }
    }
    if (it->second.back().getSeqNum()<sqnum)
    {
        SeqNumberData sinfo(sqnum,numTimes);
        SeqNumberVector v;
        it->second.push_back(sinfo);
        std::stable_sort(it->second.begin(),it->second.end());
        if ((int)it->second.size() > MaxSeqNum)
            it->second.pop_front();
        return true;
    }
    return false; // too old
}

int Ieee80211Mesh::getNumVisit(const L3Address &addr, const std::vector<L3Address> &path)
{
    int numVisit = 0;
    for (unsigned int i = 0; i< path.size(); i++)
        if (addr == path[i])
            numVisit++;
    return numVisit;
}

int Ieee80211Mesh::getNumVisit(const std::vector<L3Address> &path)
{
    return getNumVisit(L3Address(myAddress), path);
}

bool Ieee80211Mesh::getNextInPath(const L3Address &addr, const std::vector<L3Address> &path, std::vector<L3Address> &next)
{
    next.clear();
    // search the address in the path
    std::vector<unsigned int> position;
    for (unsigned int i = 0; i< path.size(); i++)
    {
        if (addr == path[i])
            position.push_back(i);
    }
    if (position.empty())
        return false;
    if (position.size() == 1 && position[0] != path.size()-1)
    {
        next.push_back(path[position[0]+1]);
    }
    else if (position.size() == 1 && position[0] == path.size()-1) // last node, send the packet to the previous
    {
        next.push_back(path[position[0]-1]);
        return true;
    }
    else
    {
    // several instances
        for (unsigned int i = 0; i< position.size(); i++)
        {
            if (position[i]+1<path.size())
                next.push_back(path[position[i]+1]);
        }
    }
    // check if the next has been visited several times

    for (unsigned int i = 0; i < next.size(); i++)
    {
        std::vector<unsigned int> position2;
        for (unsigned int j = 0; j< path.size(); j++)
        {
            if (next[i] == path[j])
                position2.push_back(i);
        }
        if (position2.empty())
            throw cRuntimeError("!!!!!!");
        if (position2.size() > 1)
        {
            // check if the node has been visited previously
            if (position2[0]<position[0])
            {
                next.erase(next.begin()+i);
                i--;
                continue;
            }
        }
    }
    return true;
}

bool Ieee80211Mesh::getNextInPath(const std::vector<L3Address> &path, std::vector<L3Address> &next)
{
    return getNextInPath(L3Address(myAddress), path, next);
}

void Ieee80211Mesh::processDistributionPacket(Ieee80211MeshFrame *frame)
{
    double  delay = SIMTIME_DBL(simTime() - frame->getTimestamp());
    if ((frame->getTTL()<=0) || (delay > limitDelay))
    {
        delete frame;
        return;
    }
    uint64_t sqNum = frame->getSeqNumber();
    L3Address srcAddr = L3Address(frame->getAddress3());
    if (0 == findSeqNum(srcAddr, sqNum) && (frame->getSubType() == UPPERMESSAGE))
    {
        setSeqNum(srcAddr, sqNum, 1);
        OLSR * olsr = dynamic_cast<OLSR *> (routingModuleProactive);
        std::vector<L3Address> path;
        std::vector<L3Address> next;
        olsr->getDistributionPath(srcAddr, path);
        getNextInPath(path, next);
        if (!next.empty())
        {
            while (!next.empty())
            {
                Ieee80211MeshFrame *frameAux = frame->dup();
                frameAux->setReceiverAddress(next.back().toMAC());
                sendFrameDown(frameAux);
                next.pop_back();
            }
        }
        emit(numHopsSignal,frame->getTotalHops());
        emit(numFixHopsSignal,frame->getTotalStaticHops());

        cPacket *msg = decapsulate(frame);
        sendUp(msg);
    }
    delete frame;
}

}

}

