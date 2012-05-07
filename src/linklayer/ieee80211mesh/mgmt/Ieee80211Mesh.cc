//
// Copyright (C) 2008 Alfonso Ariza
// Copyright (C) 2010 Alfonso Ariza
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
#include "Ieee80211Mesh.h"
#include "MeshControlInfo_m.h"
#include "lwmpls_data.h"
#include "ControlInfoBreakLink_m.h"
#include "ControlManetRouting_m.h"
#include "InterfaceTableAccess.h"
#include "locatorPkt_m.h"
#include <string.h>
#include "EtherFrame_m.h"


/* WMPLS */


#if !defined (UINT32_MAX)
#   define UINT32_MAX  4294967295UL
#endif

#ifdef CHEAT_IEEE80211MESH
Ieee80211Mesh::GateWayDataMap * Ieee80211Mesh::gateWayDataMap;
#endif

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
        for (GateWayDataMap::iterator it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
        {
            it->second.proactive=NULL;
            it->second.reactive=NULL;
            it->second.associatedAddress=NULL;
        }
        // getGateWayDataMap()->clear();
        delete gateWayDataMap;
        gateWayDataMap = NULL;
    }
}

Ieee80211Mesh::Ieee80211Mesh()
{
    // Mpls data
    mplsData = NULL;
    gateWayDataMap=NULL;
   // gateWayDataMap.clear();
    // subprocess
    ETXProcess = NULL;
    routingModuleProactive = NULL;
    routingModuleReactive = NULL;
    routingModuleHwmp = NULL;
    // packet timers
    WMPLSCHECKMAC = NULL;
    gateWayTimeOut = NULL;
    //
    macBaseGateId = -1;
    gateWayIndex = -1;
    isGateWay = false;
    hasLocator = false;
    hasRelayUnit = false;
    numRoutingBytes = 0;
}

void Ieee80211Mesh::initializeBase(int stage)
{
    if (stage==0)
    {
        PassiveQueueBase::initialize();

        dataQueue.setName("wlanDataQueue");
        mgmtQueue.setName("wlanMgmtQueue");
        dataQueueLenSignal = registerSignal("dataQueueLen");
        emit(dataQueueLenSignal, dataQueue.length());

        numDataFramesReceived = 0;
        numMgmtFramesReceived = 0;
        numMgmtFramesDropped = 0;
        WATCH(numDataFramesReceived);
        WATCH(numMgmtFramesReceived);
        WATCH(numMgmtFramesDropped);

        // configuration
        frameCapacity = par("frameCapacity");
        numMac = 0;
    }
    else if (stage==1)
    {
        // obtain our address from MAC
        cModule *mac = getParentModule()->getSubmodule("mac");
        if (!mac)
        {
            // search for vector of mac:
            do
            {
                mac = getParentModule()->getSubmodule("mac",numMac);
                if (mac)
                    numMac++;
            }
            while (mac);
            if (numMac == 0)
                error("MAC module not found; it is expected to be next to this submodule and called 'mac'");
            else
                mac = getParentModule()->getSubmodule("mac",0);
        }
        myAddress.setAddress(mac->par("address").stringValue());
    }
}


void Ieee80211Mesh::initialize(int stage)
{
    EV << "Init mesh proccess \n";
    initializeBase(stage);

    if (stage == 0)
    {
        limitDelay = par("maxDelay").doubleValue();
        useLwmpls = par("UseLwMpls");
        maxHopProactiveFeedback = par("maxHopProactiveFeedback");
        maxHopProactive = par("maxHopProactive");
        maxHopReactive = par("maxHopReactive");
        maxTTL = par("maxTTL");
        if (gate("upperLayerOut")->getPathEndGate()->isConnected() &&
                (strcmp(gate("upperLayerOut")->getPathEndGate()->getOwnerModule()->getName(),"relayUnit")==0 || par("forceRelayUnit").boolValue()))
        {
            hasRelayUnit = true;
        }
        if (gate("locatorOut")->getPathEndGate()->isConnected() &&
                       (strcmp(gate("locatorOut")->getPathEndGate()->getOwnerModule()->getName(),"locator")==0 || par("locatorActive").boolValue()))
            hasLocator = true;
    }
    else if (stage==1)
    {
        bool useReactive = par("useReactive");
        bool useProactive = par("useProactive");
        bool ETXEstimate = par("ETXEstimate");
        bool useHwmp = par("useHwmp");
        if (useHwmp)
        {
            useReactive = false;
            useProactive = false;
            useLwmpls = false;
        }
        //if (useReactive)
        //    useProactive = false;

        if (useReactive && useProactive)
        {
            proactiveFeedback  = par("ProactiveFeedback");
        }
        else
            proactiveFeedback = false;
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

        if (routingModuleProactive == NULL && routingModuleReactive ==NULL && routingModuleHwmp==NULL)
            error("Ieee80211Mesh doesn't have active routing protocol");

        mplsData->mplsMaxTime() = 35;
        activeMacBreak = false;
        if (activeMacBreak)
            WMPLSCHECKMAC = new cMessage();

        ETXProcess = NULL;

        if (ETXEstimate)
            startEtx();

    }
    else if (stage == 4)
    {
        // macBaseGateId = gateSize("macOut")==0 ? -1 : gate("macOut",0)->getId(); // FIXME macBaseGateId is unused, what is it?
        macBaseGateId = 0;
        EV << "macBaseGateId :" << macBaseGateId << "\n";
        ift = InterfaceTableAccess ().get();
        nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_LINK_BREAK);
        nb->subscribe(this, NF_LINK_REFRESH);
    }
    else if (stage==5)
    {
        if (routingModuleProactive)
            routingModuleProactive->setStaticNode(par("FixNode").boolValue());
        if (routingModuleReactive)
            routingModuleReactive->setStaticNode(par("FixNode").boolValue());
        if (routingModuleReactive && routingModuleProactive)
        {
            routingModuleReactive->setColaborativeProtocol(routingModuleProactive);
            routingModuleProactive->setColaborativeProtocol(routingModuleReactive);
        }
        if (par("IsGateWay"))
            startGateWay();
        //end Gateway and group address code
    }
}

void Ieee80211Mesh::startProactive()
{
    cModuleType *moduleType;
    cModule *module;
    //if (isEtx)
    //  moduleType = cModuleType::find("inet.networklayer.manetrouting.OLSR_ETX");
    //else
    moduleType = cModuleType::find("inet.networklayer.manetrouting.OLSR");
    module = moduleType->create("ManetRoutingProtocolProactive", this);
    routingModuleProactive = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleProactive->gate("to_ip")->connectTo(gate("routingInProactive"));
    gate("routingOutProactive")->connectTo(routingModuleProactive->gate("from_ip"));
    routingModuleProactive->buildInside();
    routingModuleProactive->scheduleStart(simTime());
}


void Ieee80211Mesh::startReactive()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find("inet.networklayer.manetrouting.DYMOUM");
    module = moduleType->create("ManetRoutingProtocolReactive", this);
    routingModuleReactive = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleReactive->gate("to_ip")->connectTo(gate("routingInReactive"));
    gate("routingOutReactive")->connectTo(routingModuleReactive->gate("from_ip"));
    routingModuleReactive->buildInside();
    routingModuleReactive->scheduleStart(simTime());
}

void Ieee80211Mesh::startHwmp()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find("inet.linklayer.ieee80211mesh.hwmp.HwmpProtocol");
    module = moduleType->create("HwmpProtocol", this);
    routingModuleHwmp = dynamic_cast <ManetRoutingBase*> (module);
    routingModuleHwmp->gate("to_ip")->connectTo(gate("routingInHwmp"));
    gate("routingOutHwmp")->connectTo(routingModuleHwmp->gate("from_ip"));
    routingModuleHwmp->buildInside();
    routingModuleHwmp->scheduleStart(simTime());
}

void Ieee80211Mesh::startEtx()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find("inet.linklayer.ieee80211.mgmt.Ieee80211Etx");
    module = moduleType->create("ETXproc", this);
    ETXProcess = dynamic_cast <Ieee80211Etx*> (module);
    ETXProcess->gate("toMac")->connectTo(gate("ETXProcIn"));
    gate("ETXProcOut")->connectTo(ETXProcess->gate("fromMac"));
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
    if (gateWayDataMap == NULL)
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
        opp_error("Mesh gateway not initialized, Ethernet interface not found");
    GateWayData data;
    data.idAddress=myAddress;
    data.ethAddress=ethAddr;
    #ifdef CHEAT_IEEE80211MESH
    data.associatedAddress=&associatedAddress;
    data.proactive=routingModuleProactive;
    data.reactive=routingModuleReactive;
    #endif
    getGateWayDataMap()->insert(std::pair<Uint128,GateWayData>(myAddress.getInt(),data));
    if(routingModuleProactive)
        routingModuleProactive->addInAddressGroup(myAddress.getInt());
    if (routingModuleReactive)
        routingModuleReactive->addInAddressGroup(myAddress.getInt());
    gateWayIndex = 0;
    for(GateWayDataMap::iterator it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
    {
        if (it->first.getLo() == myAddress.getInt())
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
    if (strstr(gateName,"macIn")!=NULL)
    {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        if (msggate->isVector())
            msg->setKind(msggate->getIndex());
        else
            msg->setKind(-1);

        if (dynamic_cast<Ieee80211ActionHWMPFrame *>(msg))
        {
            if ((routingModuleHwmp != NULL) && (routingModuleHwmp->isOurType(PK(msg))))
                send(msg,"routingOutHwmp");
            else
                delete msg;
        }
        else
        {
            Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
            Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(msg);
            if (frame2)
                frame2->setTTL(frame2->getTTL()-1);
            actualizeReactive(frame,false);
            processFrame(frame);
        }
    }
    //else if (msg->arrivedOn("agentIn"))
    else if (strstr(gateName,"agentIn")!=NULL)
    {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cPolymorphic *ctrl = msg->removeControlInfo();
        delete msg;
        handleCommand(msgkind, ctrl);
    }
    //else if (msg->arrivedOn("routingIn"))
    else if (strstr(gateName,"routingIn")!=NULL)
    {
        handleRoutingMessage(PK(msg));
    }
    else if (strstr(gateName,"ETXProcIn")!=NULL)
    {
        handleEtxMessage(PK(msg));
    }
    //else if (strstr(gateName,"interGateWayConect")!=NULL)
    else if (strstr(gateName,"fromEthernet")!=NULL)
    {
        handleWateGayDataReceive(PK(msg));
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
        sendOrEnqueue(PK(msg));
    else
        opp_error("message timer error");
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
    if (dynamic_cast<Ieee80211ActionHWMPFrame *>(msg))
    {
        msg->setKind(ctrl->getInputPort());
        delete ctrl;
        sendOrEnqueue(msg);
    }
    else
    {
        Ieee80211DataFrame * frame = encapsulate(msg,ctrl->getDest());
        Ieee80211MeshFrame *frameMesh = check_and_cast<Ieee80211MeshFrame*>(frame);
        if (frameMesh->getSubType() == 0)
            frameMesh->setSubType(ROUTING);

        frame->setKind(ctrl->getInputPort());

        delete ctrl;
        sendOrEnqueue(frame);
    }
}

void Ieee80211Mesh::handleUpperMessage(cPacket *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    if (frame)
    {
        if (!isGateWay)
            sendOrEnqueue(frame);
        else
        {
            MACAddress gw;
            if (!frame->getAddress4().isBroadcast() && !frame->getAddress4().isUnspecified())
            {
                if (selectGateWay(frame->getAddress4().getInt(), gw))
                {
                    if (gw != myAddress)
                    {
                        frame->setReceiverAddress(gw);
                        sendOrEnqueue(frame);
                        return;
                    }
                }
            }
            handleReroutingGateway(frame);
        }
    }
}

void Ieee80211Mesh::handleCommand(int msgkind, cPolymorphic *ctrl)
{
    error("handleCommand(): no commands supported");
}

Ieee80211DataFrame *Ieee80211Mesh::encapsulate(cPacket *msg)
{
    Ieee80211MeshFrame *frame = new Ieee80211MeshFrame(msg->getName());
    frame->setSubType(UPPERMESSAGE);
    frame->setTTL(maxTTL);
    frame->setTimestamp(msg->getCreationTime());
    LWMPLSPacket *lwmplspk = NULL;
    LWmpls_Forwarding_Structure *forwarding_ptr=NULL;
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
        if (routingModuleReactive->findInAddressGroup(dest.getInt()))
            toGateWay = true;
    }
    else if (routingModuleProactive)
    {
        if (routingModuleProactive->findInAddressGroup(dest.getInt()))
             toGateWay = true;
    }
    else if (routingModuleHwmp)
    {
        if (routingModuleHwmp->findInAddressGroup(dest.getInt()))
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
                    opp_error("lwmpls data base error");
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
                opp_error("lwmpls data base error");
            }
        }
        forwarding_ptr->last_use=simTime();
    }
    else
    {
        std::vector<Uint128> add;
        int dist = 0;
        bool noRoute;

        if (routingModuleProactive)
        {
            if (toGateWay)
            {
                bool isToGt;
                Uint128 gateWayAddress;
                dist = routingModuleProactive->getRouteGroup(dest.getInt(),add,gateWayAddress,isToGt);
                noRoute = false;
            }
            else
            {
                dist = routingModuleProactive->getRoute(dest.getInt(),add);
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
                    Uint128 gateWayAddress;
                    bool isToGt;

                    if (!routingModuleReactive->getNextHopGroup(dest.getInt(),add[0],iface,gateWayAddress,isToGt)) //send the packet to the routingModuleReactive
                    {
                        ControlManetRouting *ctrlmanet = new ControlManetRouting();
                        ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
                        ctrlmanet->setDestAddress(dest.getInt());
                        ctrlmanet->setSrcAddress(myAddress.getInt());
                        frame->encapsulate(msg);
                        ctrlmanet->encapsulate(frame);
                        send(ctrlmanet,"routingOutReactive");
                        return NULL;
                    }
                    else
                    {
                        if (gateWayAddress.getLo() == dest.getInt())
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
                    if (!routingModuleReactive->getNextHop(dest.getInt(),add[0],iface,cost)) //send the packet to the routingModuleReactive
                    {
                        ControlManetRouting *ctrlmanet = new ControlManetRouting();
                        ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
                        ctrlmanet->setDestAddress(dest.getInt());
                        ctrlmanet->setSrcAddress(myAddress.getInt());
                        frame->encapsulate(msg);
                        ctrlmanet->encapsulate(frame);
                        send(ctrlmanet,"routingOutReactive");
                        return NULL;
                    }
                    else
                    {
                        if (add[0].getLo() == dest.getInt())
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
                      Uint128 gateWayAddress;
                      bool isToGt;

                      if (!routingModuleHwmp->getNextHopGroup(dest.getInt(),add[0],iface,gateWayAddress,isToGt)) //send the packet to the routingModuleReactive
                      {
                          frame->encapsulate(msg);
                          send(frame,"routingOutHwmp");
                          return NULL;
                      }
                      else
                      {
                          if (gateWayAddress.getLo() == dest.getInt())
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
                      if (!routingModuleHwmp->getNextHop(dest.getInt(),add[0],iface,cost)) //send the packet to the routingModuleReactive
                      {
                           frame->encapsulate(msg);
                           send(frame,"routingOutHwmp");
                           return NULL;
                      }
                      else
                      {
                           if (add[0].getLo() == dest.getInt())
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
                return NULL;
            }
        }
        next = MACAddress(add[0].getLo());
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
                next = MACAddress(add[0].getLo());
                lwmplspk->setVectorAddressArraySize(dist-1);
                //lwmplspk->setDist(dist-1);
                for (int i=0; i<dist-1; i++)
                    lwmplspk->setVectorAddress(i,MACAddress(add[i].getLo()));
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
                forwarding_ptr->path.push_back(add[i].getLo());
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
    if (frame==NULL)
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
        opp_error ("Ieee80211Mesh::encapsulate Bad Address");
    }
    if (frame->getReceiverAddress().isBroadcast())
        frame->setTTL(1);
    return frame;
}


void Ieee80211Mesh::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (details==NULL)
        return;

    if (category == NF_LINK_BREAK)
    {
        if (details==NULL)
            return;
        Ieee80211TwoAddressFrame *frame  = dynamic_cast<Ieee80211TwoAddressFrame *>(const_cast<cPolymorphic*> (details));
        if (frame)
        {
            MACAddress add = frame->getReceiverAddress();
            mplsBreakMacLink(add);
        }
    }
    else if (category == NF_LINK_REFRESH)
    {
        Ieee80211TwoAddressFrame *frame  = check_and_cast<Ieee80211TwoAddressFrame *>(details);
        if (frame)
            mplsData->lwmpls_refresh_mac (frame->getTransmitterAddress().getInt(), simTime());
    }
}

void Ieee80211Mesh::handleDataFrame(Ieee80211DataFrame *frame)
{
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
    if (frame2)
    {
        ttl = frame2->getTTL();
        finalAddress = frame2->getFinalAddress();
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
            GateWayDataMap::iterator it = getGateWayDataMap()->find((Uint128)lwmplspk->getDest().getInt());
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
        msg->setKind(0);
        if ((routingModuleProactive != NULL) && (routingModuleProactive->isOurType(msg)))
        {
            //sendDirect(msg,0, routingModule, "from_ip");
            send(msg,"routingOutProactive");
        }
        // else if (dynamic_cast<AODV_msg  *>(msg) || dynamic_cast<DYMO_element  *>(msg))
        else if ((routingModuleReactive != NULL) && routingModuleReactive->isOurType(msg))
        {
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
            MeshControlInfo *controlInfo = new MeshControlInfo;
            Ieee802Ctrl *ctrlAux = controlInfo;
            *ctrlAux=*ctrl;
            delete ctrl;
            Uint128 dest;
            msg->setControlInfo(controlInfo);
            if (routingModuleReactive->getDestAddress(msg, dest))
            {
                std::vector<Uint128>add;
                int dist = 0;
                if (routingModuleProactive && proactiveFeedback)
                {
                    // int neig = routingModuleProactive))->getRoute(src,add);
                    controlInfo->setPreviousFix(true); // This node is fix
                    dist = routingModuleProactive->getRoute(dest,add);
                }
                else
                    controlInfo->setPreviousFix(false); // This node is not fix
                if (maxHopProactive>0 && dist>maxHopProactive)
                    dist = 0;
                if (dist!=0 && proactiveFeedback)
                {
                    controlInfo->setVectorAddressArraySize(dist);
                    for (int i=0; i<dist; i++)
                        controlInfo->setVectorAddress(i,add[i]);
                }
            }
            send(msg,"routingOutReactive");
        }
        else if (isRouting)
        {
            delete msg;
        }
        else if (dynamic_cast<LocatorPkt *>(msg) != NULL && hasLocator)
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
                sendUp(msg);
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
    dropManagementFrame(frame);
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

void Ieee80211Mesh::handleBeaconFrame(Ieee80211BeaconFrame *frame)
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

// Cada ver que se envia un mensaje sirve para generar mensajes de permanencia. usa los propios hellos para garantizar que se env�an mensajes

void Ieee80211Mesh::sendOut(cMessage *msg)
{
    //InterfaceEntry *ie = ift->getInterfaceById(msg->getKind());
    msg->setKind(0);
    //send(msg, macBaseGateId + ie->getNetworkLayerGateIndex());
    Ieee80211MeshFrame *frameMesh = dynamic_cast<Ieee80211MeshFrame*>(msg);
    if (frameMesh && frameMesh->getSubType() == ROUTING)
        numRoutingBytes += frameMesh->getByteLength();
    else if (dynamic_cast<Ieee80211ActionHWMPFrame *>(msg))
    {
        Ieee80211ActionHWMPFrame *hwmpFrame = dynamic_cast<Ieee80211ActionHWMPFrame *>(msg);
        numRoutingBytes += hwmpFrame->getByteLength();
    }

    send(msg, "macOut",msg->getKind());
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
    if ((routingModuleProactive != NULL) && (routingModuleProactive->isOurType(msg)))
        return false;
    else if ((routingModuleReactive != NULL) && routingModuleReactive->isOurType(msg))
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
            if (routingModuleReactive->findInAddressGroup(frame->getAddress4().getInt()))
                toGateWay = true;
        }
        else if (routingModuleProactive)
        {
            if (routingModuleProactive->findInAddressGroup(frame->getAddress4().getInt()))
                 toGateWay = true;
        }
        else if (routingModuleHwmp)
        {
            if (routingModuleHwmp->findInAddressGroup(frame->getAddress4().getInt()))
                 toGateWay = true;
        }

        if (toGateWay)
            associatedAddress[frame2->getAddress3().getInt()]=simTime();
        if (toGateWay && !isAddressForUs(frame->getAddress4()))
        {
            frame2->setTransmitterAddress(myAddress);
            if (!frame2->getReceiverAddress().isBroadcast())
                frame2->setReceiverAddress(frame->getAddress4());
            sendOrEnqueue(frame2);
            return true;
        }
    }

    if (frame->getAddress4().isUnspecified())
        return false;
    if (isAddressForUs(frame->getAddress4()))
        return false;

    uint64_t dest = frame->getAddress4().getInt();
    uint64_t src = frame->getAddress3().getInt();
    uint64_t prev = frame->getTransmitterAddress().getInt();
    uint64_t next = mplsData->getForwardingMacKey(src,dest,prev);
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(frame);

    double  delay = SIMTIME_DBL(simTime() - frame->getTimestamp());
    if ((frame2 && frame2->getTTL()<=0) || (delay > limitDelay))
    {
        delete frame;
        return true;
    }

    if (next)
    {
        frame->setReceiverAddress(MACAddress(next));
    }
    else
    {
        std::vector<Uint128> add;
        int dist=0;
        int iface;

        bool toGateWay=false;
        if (routingModuleReactive)
        {
            if (routingModuleReactive->findInAddressGroup(dest))
                toGateWay = true;
        }
        else if (routingModuleProactive)
        {
            if (routingModuleProactive->findInAddressGroup(dest))
                 toGateWay = true;
        }
        else if (routingModuleHwmp)
        {
            if (routingModuleHwmp->findInAddressGroup(dest))
                 toGateWay = true;
        }
        Uint128 gateWayAddress;
        if (routingModuleProactive)
        {
             add.resize(1);
            if (toGateWay)
            {
                bool isToGw;
                if (routingModuleProactive->getNextHopGroup(dest,add[0],iface,gateWayAddress,isToGw))
                   dist = 1;
            }
            else
            {
                double cost;
                if (routingModuleProactive->getNextHop(dest,add[0],iface,cost))
                   dist = 1;
            }
        }

        if (dist==0 && routingModuleReactive)
        {
            add.resize(1);
            if (toGateWay)
            {
                bool isToGw;
                if (routingModuleReactive->getNextHopGroup(dest,add[0],iface,gateWayAddress,isToGw))
                   dist = 1;
            }
            else
            {
                double cost;
                if (routingModuleReactive->getNextHop(dest,add[0],iface,cost))
                   dist = 1;
            }
        }

        if (routingModuleHwmp) //send the packet to the routingModuleReactive
        {
            add.resize(1);
            if (toGateWay)
            {
                bool isToGw;
                if (routingModuleHwmp->getNextHopGroup(dest,add[0],iface,gateWayAddress,isToGw))
                   dist = 1;
            }
            else
            {
                 double cost;
                 if (routingModuleHwmp->getNextHop(dest,add[0],iface,cost)) //send the packet to the routingModuleReactive
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
                ctrlmanet->setDestAddress(dest);
                //  ctrlmanet->setSrcAddress(myAddress);
                ctrlmanet->setSrcAddress(src);
                ctrlmanet->encapsulate(frame);
                frame = NULL;
                send(ctrlmanet,"routingOutReactive");
            }
            if (routingModuleHwmp)
            {
                send(frame,"routingOutHwmp");
                frame = NULL;
            }
            else
            {
                delete frame;
                frame=NULL;
            }
        }
        else
        {
            frame->setReceiverAddress(MACAddress(add[0].getLo()));
        }

    }
    //send(msg, macBaseGateId + ie->getNetworkLayerGateIndex());
    if (frame)
        sendOrEnqueue(frame);
    return true;
}

bool Ieee80211Mesh::isUpperLayer(cMessage *msg)
{


        return true;
    return false;
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
    Uint128 dest,next;
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
            dest=frame->getAddress4().getInt();
        else
            return;
        if (!frame->getReceiverAddress().isUnspecified() && !frame->getReceiverAddress().isBroadcast())
            next=frame->getReceiverAddress().getInt();
        else
            return;

    }
    else
    {
        if (!frame->getAddress3().isUnspecified() && !frame->getAddress3().isBroadcast() )
            dest=frame->getAddress3().getInt();
        else
            return;
        if (!frame->getTransmitterAddress().isUnspecified() && !frame->getTransmitterAddress().isBroadcast())
            next=frame->getTransmitterAddress().getInt();
        else
            return;
        isReverse=true;

    }
    if (routingModuleHwmp)
        routingModuleHwmp->setRefreshRoute(dest,next,isReverse);
    if (routingModuleReactive)
        routingModuleReactive->setRefreshRoute(dest,next,isReverse);
    // actualize route to neighbor
    if (routingModuleHwmp)
        routingModuleHwmp->setRefreshRoute(next,next,isReverse);
    if (routingModuleReactive)
        routingModuleReactive->setRefreshRoute(next,next,isReverse);
}


void Ieee80211Mesh::sendOrEnqueue(cPacket *frame)
{
    Ieee80211MeshFrame * frameAux = dynamic_cast<Ieee80211MeshFrame*>(frame);
    Ieee80211DataOrMgmtFrame *frameDataorMgm = dynamic_cast <Ieee80211DataOrMgmtFrame*> (frame);
    if (frameAux && frameAux->getTTL()<=0)
    {
        delete frame;
        return;
    }
    if (frameDataorMgm)
        if (isSendToGateway(frameDataorMgm))
            return;
    actualizeReactive(frame,true);
    if (frameDataorMgm->getReceiverAddress().isBroadcast())
    {
        if (numMac>1 && this->ETXProcess)
        {
            for (unsigned int i = 1; i < numMac; i++)
            {
                cPacket *pkt = frame->dup();
                pkt->setKind(i);
                PassiveQueueBase::handleMessage(frame);
            }
        }
        frame->setKind(0);
        PassiveQueueBase::handleMessage(frame);
    }
    else
    {
        frame->setKind(getBestInterface(frameDataorMgm));
        PassiveQueueBase::handleMessage(frame);
    }
}

bool Ieee80211Mesh::isSendToGateway(Ieee80211DataOrMgmtFrame *frame)
{
    if (!isGateWay)
        return false;
    if (frame == NULL)
        return false;
    if (isGateWay)
    {
        if (frame->getControlInfo() == NULL || !dynamic_cast<MeshControlInfo*>(frame->getControlInfo()))
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
                    it = getGateWayDataMap()->find((Uint128) dynamic_cast<Ieee80211DataFrame*>(frame)->getAddress4().getInt());
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
                it = getGateWayDataMap()->find((Uint128) frame->getReceiverAddress().getInt());
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

    if (ETXProcess)
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
        if (cost.size()==1)
            return cost.begin()->second;
        double val = cost.begin()->first;
        int index = 0;
        std::multimap<double,int>::iterator it = cost.begin();
        while (val == it->first)
        {
            index++;
            it++;
        }
        if (index==0)
        {
            if (frame->getKind() != it->second)
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
                for (int j = 0;j < i;j++)
                    it++;
            } while (it->second == frame->getKind());
            return it->second;
        }
    }
    else
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
        if (frameAux &&  (frame->getControlInfo()==NULL || !dynamic_cast<MeshControlInfo*>(frame->getControlInfo())))
        {
            GateWayDataMap::iterator it;
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
                it = getGateWayDataMap()->find((Uint128)frameAux->getAddress4());
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
                it = getGateWayDataMap()->find((Uint128)frameAux->getReceiverAddress());
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
        if (frame)
            sendOrEnqueue(frame);
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
        for (GateWayDataMap::iterator it=this->getGateWayDataMap()->begin();it!=this->getGateWayDataMap()->end();it++)
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
        sendOrEnqueue(frame);
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
        getGateWayData()->insert(std::pair<Uint128,GateWayData>(pkt->getSource(),data));
    }
#endif
    for (unsigned int i=0;i<pkt->getVectorAddressArraySize();i++)
    {
        if(routingModuleProactive)
            routingModuleProactive->addInAddressGroup(pkt->getVectorAddress(i).getInt());
        if (routingModuleReactive)
            routingModuleReactive->addInAddressGroup(pkt->getVectorAddress(i).getInt());
    }
}

bool Ieee80211Mesh::selectGateWay(const Uint128 &dest,MACAddress &gateway)
{
    if (!isGateWay)
        return false;
#ifdef CHEAT_IEEE80211MESH
    GateWayDataMap::iterator best=this->getGateWayDataMap()->end();
    double bestCost;
    double myCost=10E10;
    simtime_t timeAsoc=0;
    for (GateWayDataMap::iterator it=this->getGateWayDataMap()->begin();it!=this->getGateWayDataMap()->end();it++)
    {
        int iface;
        Uint128 next;
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
                AssociatedAddress::iterator itAssoc;
                itAssoc = best->second.associatedAddress->find(dest);
                if (itAssoc!=best->second.associatedAddress->end())
                    timeAsoc=itAssoc->second;
            }
        }
        else
        {
            int iface;
            Uint128 next;
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
                    AssociatedAddress::iterator itAssoc;
                    itAssoc = best->second.associatedAddress->find(dest);
                    if (itAssoc!=best->second.associatedAddress->end())
                        timeAsoc=itAssoc->second;
                    else
                        timeAsoc=0;
                }
                else if (cost==bestCost)
                {
                    AssociatedAddress::iterator itAssoc;
                    itAssoc = best->second.associatedAddress->find(dest);
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

    pkt->setKind(-1);
    if (dynamic_cast<Ieee80211ActionHWMPFrame *>(pkt))
    {
        if ((routingModuleHwmp != NULL) && routingModuleHwmp->isOurType(pkt))
            send(pkt,"routingOutHwmp");
        else
            delete pkt;
        return;
    }
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(pkt);
    cPacket *encapPkt=NULL;
    encapPkt = pkt->getEncapsulatedPacket();
    if ((routingModuleProactive != NULL) && (routingModuleProactive->isOurType(encapPkt)))
    {
        //sendDirect(msg,0, routingModule, "from_ip");
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(pkt->removeControlInfo());
        encapPkt = pkt->decapsulate();
        MeshControlInfo *controlInfo = new MeshControlInfo;
        Ieee802Ctrl *ctrlAux = controlInfo;
        *ctrlAux=*ctrl;
        if (frame2->getReceiverAddress().isUnspecified())
            opp_error("transmitter address is unspecified");
        else if (frame2->getReceiverAddress() != myAddress && !frame2->getReceiverAddress().isBroadcast())
            opp_error("bad address");
        else
            controlInfo->setDest(frame2->getReceiverAddress());
        if (getGateWayDataMap()==NULL)
            opp_error("error GateWayMap not found");
        for (GateWayDataMap::iterator it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
        {
            if (ctrl->getSrc()==it->second.ethAddress)
                controlInfo->setSrc(MACAddress(it->first.getLo()));
        }
        delete ctrl;
        encapPkt->setControlInfo(controlInfo);
        send(encapPkt,"routingOutProactive");
        delete pkt;
        return;
    }
    else if ((routingModuleReactive != NULL) && routingModuleReactive->isOurType(encapPkt))
    {
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl*>(pkt->removeControlInfo());
        encapPkt = pkt->decapsulate();
        MeshControlInfo *controlInfo = new MeshControlInfo;
        Ieee802Ctrl *ctrlAux = controlInfo;
        *ctrlAux=*ctrl;
        if (frame2->getReceiverAddress().isUnspecified())
            opp_error("transmitter address is unspecified");
        else if (frame2->getReceiverAddress() != myAddress && !frame2->getReceiverAddress().isBroadcast())
            opp_error("bad address");
        else
            controlInfo->setDest(frame2->getReceiverAddress());
        if (getGateWayDataMap()==NULL)
            opp_error("error GateWayMap not found");

        for (GateWayDataMap::iterator it=getGateWayDataMap()->begin();it!=getGateWayDataMap()->end();it++)
        {
            if (ctrl->getSrc()==it->second.ethAddress)
                controlInfo->setSrc(MACAddress(it->first.getLo()));
        }
        delete ctrl;
        Uint128 dest;
        encapPkt->setControlInfo(controlInfo);
        if (routingModuleReactive->getDestAddress(encapPkt,dest))
        {
            std::vector<Uint128>add;
            int dist = 0;
            if (routingModuleProactive && proactiveFeedback)
            {
                // int neig = routingModuleProactive))->getRoute(src,add);
                controlInfo->setPreviousFix(true); // This node is fix
                dist = routingModuleProactive->getRoute(dest,add);
            }
            else
                controlInfo->setPreviousFix(false); // This node is not fix
            if (maxHopProactive>0 && dist>maxHopProactive)
                dist = 0;
            if (dist!=0 && proactiveFeedback)
            {
                controlInfo->setVectorAddressArraySize(dist);
                for (int i=0; i<dist; i++)
                    controlInfo->setVectorAddress(i,add[i]);
            }
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
            else if (dynamic_cast<LocatorPkt *>(msg) != NULL && hasLocator)
                send(msg, "locatorOut");
            else
                encapPkt = msg;
            if (encapPkt && isUpper)
                sendUp(encapPkt);
        }
        else if (!frame2->getFinalAddress().isUnspecified())
        {
            handleReroutingGateway(frame2);
        }
        else
        {
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
        return  routingModuleReactive->addressIsForUs(add.getInt());
    else if (routingModuleProactive)
        return routingModuleProactive->addressIsForUs(add.getInt());
    else if (routingModuleHwmp)
        return routingModuleHwmp->addressIsForUs(add.getInt());
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
}
