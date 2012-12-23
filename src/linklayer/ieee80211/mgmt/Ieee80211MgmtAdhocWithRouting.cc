//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "Ieee80211MgmtAdhocWithRouting.h"
#include "Ieee802Ctrl_m.h"
#include "ControlManetRouting_m.h"


Define_Module(Ieee80211MgmtAdhocWithRouting);


void Ieee80211MgmtAdhocWithRouting::startRouting()
{
    cModuleType *moduleType;
    cModule *module;
    moduleType = cModuleType::find(par("routingProtocol").stringValue());
    if (moduleType == NULL)
        opp_error("Ieee80211MgmtAdhocWithRouting:: Routing protocol not found %s",par("routingProtocol").stringValue());
    module = moduleType->create("ManetRoutingProtocol", this);
    routingModule = dynamic_cast <ManetRoutingBase*> (module);
    routingModule->gate("to_ip")->connectTo(gate("routingIn"));
    gate("routingOut")->connectTo(routingModule->gate("from_ip"));
    routingModule->buildInside();
    routingModule->scheduleStart(simTime());
}


void Ieee80211MgmtAdhocWithRouting::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage == 4)
    {
        routingModule = NULL;
        maxTTL = par("maxTtl").longValue();
        if (strcmp(par("routingProtocol").stringValue(),"") != 0)
            startRouting();
    }
}

void Ieee80211MgmtAdhocWithRouting::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtAdhocWithRouting::handleUpperMessage(cPacket *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendOrEnqueue(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleCommand(int msgkind, cObject *ctrl)
{
    error("handleCommand(): no commands supported");
}


void Ieee80211MgmtAdhocWithRouting::receiveChangeNotification(int category, const cObject *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);
}

void Ieee80211MgmtAdhocWithRouting::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocWithRouting::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}


void Ieee80211MgmtAdhocWithRouting::actualizeReactive(cPacket *pkt,bool out)
{
    ManetAddress dest,next;
    if (routingModule == NULL || (routingModule != NULL && routingModule->isProactive()))
        return;

    Ieee80211DataFrame * frame = dynamic_cast<Ieee80211DataFrame*>(pkt);

    if (!frame )
        return;

    bool isReverse=false;
    if (out)
    {
        if (!frame->getAddress4().isUnspecified() && !frame->getAddress4().isBroadcast())
            dest= ManetAddress(frame->getAddress4());
        else
            return;
        if (!frame->getReceiverAddress().isUnspecified() && !frame->getReceiverAddress().isBroadcast())
            next = ManetAddress(frame->getReceiverAddress());
        else
            return;

    }
    else
    {
        if (!frame->getAddress3().isUnspecified() && !frame->getAddress3().isBroadcast() )
            dest = ManetAddress(frame->getAddress3());
        else
            return;
        if (!frame->getTransmitterAddress().isUnspecified() && !frame->getTransmitterAddress().isBroadcast())
            next = ManetAddress(frame->getTransmitterAddress());
        else
            return;
        isReverse=true;

    }
    routingModule->setRefreshRoute(dest,next,isReverse);
}


cPacket *Ieee80211MgmtAdhocWithRouting::decapsulate(Ieee80211DataFrame *frame)
{
    cPacket *payload = frame->decapsulate();

    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    ctrl->setSrc(frame->getTransmitterAddress());
    ctrl->setDest(frame->getReceiverAddress());
    Ieee80211DataFrameWithSNAP *frameWithSNAP = dynamic_cast<Ieee80211DataFrameWithSNAP *>(frame);
    if (frameWithSNAP)
        ctrl->setEtherType(frameWithSNAP->getEtherType());
    payload->setControlInfo(ctrl);

    delete frame;
    return payload;
}

void Ieee80211MgmtAdhocWithRouting::handleMessage(cMessage *msg)
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
        Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
        Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(msg);
        if (frame2)
            frame2->setTTL(frame2->getTTL()-1);
        actualizeReactive(frame,false);
        processFrame(frame);
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



void Ieee80211MgmtAdhocWithRouting::handleDataFrame(Ieee80211DataFrame *frame)
{
    // The message is forward
    if (forwardMessage(frame))
        return;

    MACAddress finalAddress;
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(frame);
    if (!frame2)
    {
        sendUp(decapsulate(frame));
        return;
    }

    bool upperPacket = (frame2 && (frame2->getSubType() == UPPERMESSAGE));
    bool isRouting = (frame2 && (frame2->getSubType() == ROUTING));

    cPacket *msg = decapsulate(frame);
    //cGate * msggate = msg->getArrivalGate();
    //int baseId = gateBaseId("macIn");
    //int index = baseId - msggate->getId();
    msg->setKind(0);
    if ((routingModule != NULL) && (routingModule->isOurType(msg)))
    {
        //sendDirect(msg,0, routingModule, "from_ip");
        send(msg,"routingOut");
    }
    else if (isRouting)
    {
        delete msg;
    }
    else if (upperPacket)// Normal frame test if upper layer frame in other case delete
    {
        sendUp(msg);
    }
    else
        delete msg;
    return;
}



bool Ieee80211MgmtAdhocWithRouting::forwardMessage (Ieee80211DataFrame *frame)
{

    cPacket *msg = frame->getEncapsulatedPacket();
    if ((routingModule != NULL) && (routingModule->isOurType(msg)))
        return false;
    else // Normal frame test if use the mac label address method
        return macLabelBasedSend(frame);

}

bool Ieee80211MgmtAdhocWithRouting::macLabelBasedSend(Ieee80211DataFrame *frame)
{

    if (!frame)
        return false;

     if (frame->getAddress4().isUnspecified())
        return false;

    if (frame->getAddress4() == this->myAddress)
        return false;


    if (frame->getReceiverAddress().isBroadcast())
    {

        Ieee80211MeshFrame *frameMesh = check_and_cast<Ieee80211MeshFrame*>(frame);
        if (!frameMesh)
            return false;
        if (frame->getAddress3().isUnspecified())
            opp_error("frame Address3 Unspecified");

        SeqNumberInfo::iterator it = mySeqNumberInfo.find(frame->getAddress3().getInt());
        if (it == mySeqNumberInfo.end())
        {
            mySeqNumberInfo[frame->getAddress3().getInt()].push_back(frameMesh->getSequenceNumber());
        }
        else
        {
            for (unsigned int i = 0 ; i< it->second.size(); i++)
            {
                if (it->second[i] == frameMesh->getSequenceNumber())
                {
                    delete frame;
                    return true;
                }
            }
            it->second.push_back(frameMesh->getSequenceNumber());
            if (it->second.size()>20)
                it->second.pop_front();
        }
        frame->setKind(0);
        sendOrEnqueue(frame->dup());
        sendUp(decapsulate(frame));
        return true;
    }

    ManetAddress dest = ManetAddress(frame->getAddress4());
    ManetAddress src = ManetAddress(frame->getAddress3());
    Ieee80211MeshFrame *frame2  = dynamic_cast<Ieee80211MeshFrame *>(frame);

    if ((frame2 && frame2->getTTL()<=0))
    {
        delete frame;
        return true;
    }

    std::vector<ManetAddress> add;
    int iface;
    double cost;
    ManetAddress next;

    if (!routingModule)
    {
        delete frame;
        return true;
    }


    if (routingModule->getNextHop(dest,next,iface,cost))
    {
        frame->setReceiverAddress(next.getMAC());
    }
    else
    {
        // Destination unreachable
        if (!routingModule->isProactive())
        {
            ControlManetRouting *ctrlmanet = new ControlManetRouting();
            ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
            ctrlmanet->setDestAddress(dest);
            //  ctrlmanet->setSrcAddress(myAddress);
            ctrlmanet->setSrcAddress(src);
            ctrlmanet->encapsulate(frame);
            frame = NULL;
            send(ctrlmanet,"routingOut");
        }
    }
    //send(msg, macBaseGateId + ie->getNetworkLayerGateIndex());
    if (frame)
    {
        frame->setKind(0);
        sendOrEnqueue(frame);
    }
    return true;
}



void Ieee80211MgmtAdhocWithRouting::handleRoutingMessage(cPacket *msg)
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





Ieee80211DataFrame *Ieee80211MgmtAdhocWithRouting::encapsulate(cPacket *msg)
{
    if (!routingModule)
    {
        Ieee80211DataFrameWithSNAP *frame = new Ieee80211DataFrameWithSNAP(msg->getName());
        // copy receiver address from the control info (sender address will be set in MAC)
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
        frame->setReceiverAddress(ctrl->getDest());
        frame->setEtherType(ctrl->getEtherType());
        frame->setAddress3(myAddress);
        delete ctrl;
        frame->encapsulate(msg);
        return frame;
    }

    Ieee80211MeshFrame *frame = new Ieee80211MeshFrame(msg->getName());
    frame->setSubType(UPPERMESSAGE);
    frame->setTTL(maxTTL);
    frame->setTimestamp(msg->getCreationTime());

    MACAddress next;
    MACAddress dest;

    // copy receiver address from the control info (sender address will be set in MAC)
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    dest = ctrl->getDest();
    next = ctrl->getDest();
    delete ctrl;
    frame->setAddress3(myAddress);
    frame->setFinalAddress(dest);
    frame->setAddress4(dest);
    frame->encapsulate(msg);

    if (dest.isBroadcast())
    {
        frame->setReceiverAddress(dest);
        frame->setTTL(1);
        mySeqNumber++;
        frame->setSequenceNumber(mySeqNumber);
        return frame;
    }

    //
    // Search in the data base
    //
    ManetAddress nextHop;
    int iface;
    double cost;
    if (!routingModule->getNextHop(ManetAddress(dest),nextHop,iface,cost)) //send the packet to the routingMo
    {
        if (!routingModule->isProactive())
        {
            ControlManetRouting *ctrlmanet = new ControlManetRouting();
            ctrlmanet->setOptionCode(MANET_ROUTE_NOROUTE);
            ctrlmanet->setDestAddress(ManetAddress(dest));
            ctrlmanet->setSrcAddress(ManetAddress(myAddress));
            ctrlmanet->encapsulate(frame);
            send(ctrlmanet,"routingOut");
            return NULL;
        }
        else
        {
            delete frame;
            return NULL;
        }
    }
    next = nextHop.getMAC();
    frame->setReceiverAddress(next);

    if (frame->getReceiverAddress().isUnspecified())
        ASSERT(!frame->getReceiverAddress().isUnspecified());
    return frame;
}

Ieee80211DataFrame *Ieee80211MgmtAdhocWithRouting::encapsulate(cPacket *msg,MACAddress dest)
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

