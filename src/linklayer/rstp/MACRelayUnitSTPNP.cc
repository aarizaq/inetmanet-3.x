/*
 * Copyright (C) 2009 Juan-Carlos Maureira, INRIA
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include "MACRelayUnitSTPNP.h"
#include "EtherFrame_m.h"
#include "Ethernet.h"
#include "MACAddress.h"
#define DELTATIME 0.01
// TODO:
// 1. paint links when changing to forward state. now it only paints blocking links.
//    and when they turn into forwarding state they remain painted in red.
// 4. check the TC when topology changes to shorten the MAC aging timers
//

Define_Module( MACRelayUnitSTPNP );

const MACAddress MACRelayUnitSTPNP::STPMCAST_ADDRESS("01:80:C2:00:00:00");

MACRelayUnitSTPNP::MACRelayUnitSTPNP():MACRelayUnitNP()
{
    this->active = false;
    this->showInfo = false;
    this->hello_timer = NULL;
    this->topology_change_timeout = 0;
    this->message_age = 0;
}

MACRelayUnitSTPNP::~MACRelayUnitSTPNP()
{
    while (!port_status.empty())
    {
/*        if (this->port_status.begin()->second.forward_timer)
            cancelAndDelete(this->port_status.begin()->second.forward_timer);
        if (this->port_status.begin()->second.hold_timer)
            cancelAndDelete(this->port_status.begin()->second.hold_timer);
        if (this->port_status.begin()->second.edge_timer)
            cancelAndDelete(this->port_status.begin()->second.edge_timer);
        if (this->port_status.begin()->second.bpdu_timeout_timer)
            cancelAndDelete(this->port_status.begin()->second.bpdu_timeout_timer);
        this->port_status.begin()->second.BPDUQueue.clear();
        this->port_status.erase(this->port_status.begin());*/
        if (this->port_status.back().forward_timer)
            cancelAndDelete(this->port_status.back().forward_timer);
        if (this->port_status.back().hold_timer)
            cancelAndDelete(this->port_status.back().hold_timer);
        if (this->port_status.back().edge_timer)
            cancelAndDelete(this->port_status.back().edge_timer);
        if (this->port_status.back().bpdu_timeout_timer)
            cancelAndDelete(this->port_status.back().bpdu_timeout_timer);
        this->port_status.back().BPDUQueue.clear();
        this->port_status.pop_back();
    }
}

void MACRelayUnitSTPNP::setAllPortsStatus(PortStatus status)
{
    for (int i=0; i<this->gateSize("lowerLayerOut"); i++)
    {
        this->setPortStatus(i,status);
    }
}

void MACRelayUnitSTPNP::setPortStatus(int port_idx, PortStatus status)
{
    //if (this->port_status.find(port_idx) != this->port_status.end())
    if (port_idx >=0 &&  port_idx < (int)this->port_status.size())
    {

        if (this->port_status[port_idx].role == status.role && this->port_status[port_idx].state == status.state)
        {
            // port is in the same status, no need to change port status
            return;
        }

        this->port_status[port_idx].role = status.role;
        this->port_status[port_idx].state = status.state;

        this->port_status[port_idx].BPDUQueue.clear(); // clean the BPDU queue since the port status have change

        if (status.state == LEARNING && status.role == ROOT_PORT)
        {
            this->port_status[port_idx].sync      = true;   // root port was proposed
            this->port_status[port_idx].proposing = false;  // root port is not proposing
            this->port_status[port_idx].proposed  = true;   // root port was proposed
            this->port_status[port_idx].agree     = false;  // wait for the allSynced to agree
            this->port_status[port_idx].agreed    = false;  // reset this flag
        }

        if ((status.state == LISTENING || status.state == LEARNING) && status.role == DESIGNATED_PORT)
        {
            // we are in position to propose
            this->port_status[port_idx].sync = true;      // we are in the sync process
            this->port_status[port_idx].proposing = true; // we start proposing until we block/edge the port
            this->port_status[port_idx].agreed = false;   // wait to be agreed with the proposal
            this->port_status[port_idx].agree = false;    // reset this flag
            this->port_status[port_idx].synced = false;   // wait until we agreed
            this->port_status[port_idx].proposed_pr = PriorityVector(this->priority_vector.root_id,this->priority_vector.root_path_cost,this->priority_vector.root_id,this->priority_vector.port_id);
            this->port_status[port_idx].observed_pr = PriorityVector();
        }

        if (status.state == FORWARDING && status.role == DESIGNATED_PORT)
        {
            // we are agreed to change to forward on this port
            this->port_status[port_idx].sync = false;      // sync finished
            this->port_status[port_idx].proposing = false; // reset this flag
            this->port_status[port_idx].proposed = false; // reset this flag
            this->port_status[port_idx].agreed = true;   // port is agreed
            this->port_status[port_idx].synced = true;   // port synced
        }

        if (status.role == EDGE_PORT || status.role == BACKUP_PORT)
        {
            // this port is synced
            this->port_status[port_idx].sync = true;      // sync process finished
            this->port_status[port_idx].synced = true;     // port synced
            this->port_status[port_idx].proposing = false; // reset this flag
        }

        if (status.role == ALTERNATE_PORT)
        {
            // this port is synced
            this->port_status[port_idx].sync = true;      // sync process finished
            this->port_status[port_idx].synced = true;    // port synced
            this->port_status[port_idx].agree = true;     // agree to set this port in alternate mode
            this->port_status[port_idx].proposing = false; // reset this flag
        }

        EV << "Port " << port_idx << " change status : " << this->port_status[port_idx] << endl;

        if (status.state == BLOCKING)
        {

            // color the internal port's link in red
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("ls",0,"red");
            // color the external port's link in red

            // TODO: make this code safe for unconnected gates
            cModule* mac = this->gate("lowerLayerOut",port_idx)->getPathEndGate()->getOwnerModule();
            cGate* outport = mac->gate("phys$o")->getNextGate();
            outport->getDisplayString().setTagArg("ls",0,"red");
            cGate* inport = mac->gate("phys$i")->getPreviousGate()->getPreviousGate();
            inport->getDisplayString().setTagArg("ls",0,"red");


            if (this->port_status[port_idx].getForwardTimer()->isScheduled())
            {
                EV << "  Canceling fwd timer" << endl;
                cancelEvent(this->port_status[port_idx].getForwardTimer());
            }
            if (this->port_status[port_idx].isPortEdgeTimerActive())
            {
                EV << "  Canceling port edge timer" << endl;
                cancelEvent(this->port_status[port_idx].getPortEdgeTimer());
                this->port_status[port_idx].clearPortEdgeTimer();
            }
        }
        else if (status.state == LISTENING)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("ls",0,"yellow");
            // schedule the port edge timer
            if (this->port_status[port_idx].getPortEdgeTimer()->isScheduled())
            {
                EV << "  Canceling port edge timer" << endl;
                cancelEvent(this->port_status[port_idx].getPortEdgeTimer());
            }
            this->scheduleAt(simTime()+this->edge_delay,this->port_status[port_idx].getPortEdgeTimer());
            // schedule the forward timer
            if (this->port_status[port_idx].getForwardTimer()->isScheduled())
            {
                EV << "  Canceling fwd timer" << endl;
                cancelEvent(this->port_status[port_idx].getForwardTimer());
            }
            EV << "  restarting the forward timer" << endl;
            this->scheduleAt(simTime()+this->forward_delay,this->port_status[port_idx].getForwardTimer());

        }
        else if (status.state == LEARNING)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("ls",0,"blue");

            // check the edge timer and if it is active, canceling it since
            if (this->port_status[port_idx].isPortEdgeTimerActive())
            {
                EV << "  Canceling port edge timer" << endl;
                cancelEvent(this->port_status[port_idx].getPortEdgeTimer());
                this->port_status[port_idx].clearPortEdgeTimer();
            }
            if (this->port_status[port_idx].getForwardTimer()->isScheduled())
            {
                EV << "  Canceling fwd timer" << endl;
                cancelEvent(this->port_status[port_idx].getForwardTimer());
            }
            EV << "  restarting the forward timer" << endl;
            this->scheduleAt(simTime()+this->forward_delay,this->port_status[port_idx].getForwardTimer());

            // flush entries for this port to start the learning process
            this->flushMACAddressesOnPort(port_idx);

        }
        else if (status.state == FORWARDING)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("ls",0,"green");
            if (this->port_status[port_idx].getForwardTimer()->isScheduled())
            {
                EV << "  Canceling fwd timer" << endl;
                cancelEvent(this->port_status[port_idx].getForwardTimer());
            }

            if (this->port_status[port_idx].isPortEdgeTimerActive())
            {
                EV << "  Canceling port edge timer" << endl;
                cancelEvent(this->port_status[port_idx].getPortEdgeTimer());
                this->port_status[port_idx].clearPortEdgeTimer();
            }
            this->port_status[port_idx].clearForwardTimer();
        }


        if (status.role == ROOT_PORT)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("t",0,"R");
        }
        else if (status.role == DESIGNATED_PORT)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("t",0,"D");
        }
        else if (status.role == ALTERNATE_PORT)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("t",0,"A");
        }
        else if (status.role == BACKUP_PORT)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("t",0,"B");
        }
        else if (status.role == NONDESIGNATED_PORT)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("t",0,"ND");
        }
        else if (status.role == EDGE_PORT)
        {
            this->gate("lowerLayerOut",port_idx)->getDisplayString().setTagArg("t",0,"E");
        }
    }
    else
    {
        error("trying to change the status to a port that is not registered into the portStatusList into this switch");
    }

    // check for allSynced flag
    this->allSynced = true;
    for (int i=0; i<this->gateSize("lowerLayerOut"); i++)
    {
        if (i!=this->getRootPort())
        {
            if (!this->port_status[i].synced)
            {
                // there are still ports to be synced.FORWARDING
                this->allSynced = false;
            }
        }
    }

    // check root port agreement for non-root bridges only
    if (this->getRootPort()>-1)
    {
        // check if all ports are synced when the root port is not yet agreed.
        if (this->allSynced)
        {
            if (!this->port_status[this->getRootPort()].agree)
            {
                // agree the root port and set the root port in forward mode immediately
                EV << "all ports synced, so, we agree with the root port selection" << endl;
                this->port_status[this->getRootPort()].agree = true;
                this->setPortStatus(this->getRootPort(),PortStatus(FORWARDING,ROOT_PORT));
                this->sendConfigurationBPDU(this->getRootPort());
            }
        }
    }

}

void MACRelayUnitSTPNP::setRootPort(int port)
{

    // preRootChange hook
    this->preRootChange();

    // start the sync process on all the ports
    this->allSynced = false;
    EV << "Old Root Election. PR " << this->priority_vector << endl;
    this->priority_vector = this->port_status[port].observed_pr;
    EV << "New Root Election: " << this->priority_vector << endl;

    // Setting all the port in designated status
    for (int i=0; i<this->gateSize("lowerLayerOut"); i++)
    {
        if (i!=port)
        {
            this->setPortStatus(i,PortStatus(LISTENING,DESIGNATED_PORT));
        }
    }
    // assing the root port role to the given port
    this->setPortStatus(port,PortStatus(LEARNING,ROOT_PORT));

    // start the BPDU timeout timer to know when we have lost the root port
    this->restartBPDUTimeoutTimer(port);
    // schedule the hello timer according the values received from the root bridge (RSTP)
    this->scheduleHelloTimer();

    // portRootChange hook
    this->postRootChange();

    // show pr info
    this->showPriorityVectorInfo();

    // start proposing our information to all the ports
    this->sendConfigurationBPDU();
}

void MACRelayUnitSTPNP::recordRootTimerDelays(BPDU* bpdu)
{
    // record the root bridge timers information and message age
    this->max_age_time = bpdu->getMaxAge();
    this->forward_delay = bpdu->getForwardDelay();
    this->hello_time = bpdu->getHelloTime();
    this->message_age = bpdu->getMessageAge();
}

void MACRelayUnitSTPNP::recordPriorityVector(BPDU* bpdu, int port_idx)
{
    this->port_status[port_idx].observed_pr = PriorityVector(bpdu->getRootBID(),bpdu->getRootPathCost(),bpdu->getSenderBID(),bpdu->getPortId());
}


int MACRelayUnitSTPNP::getRootPort()
{
    for (int i=0; i<this->gateSize("lowerLayerOut"); i++)
    {
        if (this->port_status[i].role == ROOT_PORT)
        {
            return i;
        }
    }
    return -1;
}

void MACRelayUnitSTPNP::scheduleHoldTimer(int port)
{
    STPHoldTimer* hold_timer = this->port_status[port].getHoldTimer();

    if (hold_timer->isScheduled())
    {
        EV << "canceling and rescheduling hold timer on port " << port << " " << endl;
        cancelEvent(hold_timer);
    }
    else
    {
        EV << "scheduling hold timer on port " << port << " " << endl;
    }
    this->port_status[port].packet_forwarded = 0;
    scheduleAt(simTime()+this->hold_time,hold_timer);
}

void MACRelayUnitSTPNP::scheduleHelloTimer()
{

    if (this->hello_timer==NULL)
    {
        this->hello_timer = new STPHelloTimer("STP Hello Timer");
    }

    if (this->hello_timer->isScheduled())
    {
        cancelEvent(this->hello_timer);
    }

    if (this->hello_time>0)
    {
        EV << "Scheduling Hello timer to " << simTime()+this->hello_time << endl;
        scheduleAt(simTime()+this->hello_time,this->hello_timer);
    }
    else
    {
        error("Hello timer could not be scheduled due to hello time is invalid");
    }
}


void MACRelayUnitSTPNP::restartBPDUTimeoutTimer(int port)
{

    EV << "Rescheduling BPDU Timeout Timer on port " << port << endl;

    STPBPDUTimeoutTimer* timer = this->port_status[port].getBPDUTimeoutTimer();

    if (timer->isScheduled())
    {
        cancelEvent(timer);
        scheduleAt(simTime()+this->bpdu_timeout,timer);
    }
    else
    {
        scheduleAt(simTime()+this->bpdu_timeout,timer);
    }
    this->port_status[port].alive = true;
}

void MACRelayUnitSTPNP::flushMACAddressesOnPort(int port_idx)
{
    EV << "Flushing MAC Address on port " << port_idx << endl;
    for (AddressTable::iterator iter = addresstable.begin(); iter != addresstable.end();)
    {
        AddressTable::iterator cur = iter++; // iter will get invalidated after erase()
        AddressEntry& entry = cur->second;
        if (entry.portno == port_idx )
        {
            EV << "Removing entry from Address Table: " <<
            cur->first << " --> port" << cur->second.portno << "\n";
            addresstable.erase(cur);
        }
    }
}

void MACRelayUnitSTPNP::moveMACAddresses(int from_port,int to_port)
{
    EV << "Moving MAC Address from port " << from_port <<  " to port " << to_port << endl;
    for (AddressTable::iterator iter = addresstable.begin(); iter != addresstable.end();)
    {
        AddressTable::iterator cur = iter++;
        MACAddress mac = cur->first;
        AddressEntry& entry = cur->second;
        if (entry.portno == from_port )
        {
            this->updateTableWithAddress(mac,to_port);
        }
    }
}

double MACRelayUnitSTPNP::readChannelBitRate(int index)
{
    cModule* mac = this->gate("lowerLayerOut",index)->getPathEndGate()->getOwnerModule();
    cGate* physOutGate = mac->gate("phys$o");
    cGate* physInGate = mac->gate("phys$i");

    cChannel *outTrChannel = physOutGate->findTransmissionChannel();
    cChannel *inTrChannel = physInGate->findIncomingTransmissionChannel();

    bool connected = physOutGate->getPathEndGate()->isConnected() && physInGate->getPathStartGate()->isConnected();

    if (connected && ((!outTrChannel) || (!inTrChannel)))
        throw cRuntimeError("Ethernet phys gate must be connected using a transmission channel");

    double txRate = outTrChannel ? outTrChannel->getNominalDatarate() : 0.0;
    double rxRate = inTrChannel ? inTrChannel->getNominalDatarate() : 0.0;
    bool dataratesDiffer;
    if (!connected)
    {
        dataratesDiffer = (outTrChannel != NULL) || (inTrChannel != NULL);
    }
    else
    {
        dataratesDiffer = (txRate != rxRate);
    }

    if (dataratesDiffer)
    {
        throw cRuntimeError("The input/output datarates differ (%g / %g bps)", rxRate, txRate);
    }

    return txRate;

}

void MACRelayUnitSTPNP::initialize()
{

    MACRelayUnitNP::initialize();

    EV << "STP Initialization" << endl;

    this->bridge_id.priority = par("priority");
    this->hello_time         = par("helloTime");
    this->max_age_time       = par("maxAge");
    this->forward_delay      = par("forwardDelay");
    this->power_on_time      = par("powerOn");
    this->hold_time          = par("holdTime");
    this->migrate_delay      = par("migrateDelay");
    this->edge_delay         = par("portEdgeDelay");
    this->packet_fwd_limit   = par("packetFwdLimit");
    this->bpdu_timeout       = par("bpduTimeout");

    this->showInfo           = par("showInfo");

    // capture the original addresses aging time to restore it when
    // the TCN will change this delay to get a faster renew of the address table.
    this->original_mac_aging_time = par("agingTime");

    const char* address_string = par("bridgeAddress");
    if (!strcmp(address_string,"auto"))
    {
        // assign automatic address
        this->bridge_id.address = MACAddress::generateAutoAddress();
        // change module parameter from "auto" to concrete address
        par("bridgeAddress").setStringValue(this->bridge_id.address.str().c_str());
    }
    else
    {
        this->bridge_id.address.setAddress(par("bridgeAddress"));
    }

    // initially, we state that we are the root bridge

    this->priority_vector = PriorityVector(this->bridge_id,0,this->bridge_id,0);

    EV << "Bridge ID :" << this->bridge_id << endl;
    EV << "Root Priority Vector :" << this->priority_vector << endl;

    scheduleAt(this->power_on_time, new STPStartProtocol("PoweringUp the Bridge"));

    // switch port registration
    this->port_status.clear();
    for (int i=0; i<this->gateSize("lowerLayerOut"); i++)
    {
        //this->port_status.insert(std::make_pair(i,PortStatus(i)));
        this->port_status.push_back(PortStatus(i));
        this->port_status[i].port_index = i;
        //WATCH(this->port_status[i]);
        double bitRate = readChannelBitRate(i);

        if (bitRate == ETHERNET_TXRATE)
            this->port_status[i].cost   = 2000000;
        else if (bitRate == FAST_ETHERNET_TXRATE)
            this->port_status[i].cost   = 200000;
        else if (bitRate == GIGABIT_ETHERNET_TXRATE)
            this->port_status[i].cost   =   20000;
        else if (bitRate == FAST_GIGABIT_ETHERNET_TXRATE)
            this->port_status[i].cost   =   2000;
        else if (bitRate == FOURTY_GIGABIT_ETHERNET_TXRATE)
            this->port_status[i].cost   = 500;
        else if (bitRate == HUNDRED_GIGABIT_ETHERNET_TXRATE)
            this->port_status[i].cost   = 200;
    }
    WATCH_VECTOR(port_status);
    WATCH(bridge_id);
    WATCH(priority_vector);
    WATCH(message_age);

}

void MACRelayUnitSTPNP::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage())
    {
        if (dynamic_cast<STPTimer*>(msg))
        {
            this->handleTimer(msg);
        }
        else
        {
            this->processFrame(msg);
        }
        return;
    }
    else
    {
        if (this->active)
        {
            if (dynamic_cast<EtherFrame*>(msg))
            {
                cPacket* frame = ((EtherFrame*)msg)->getEncapsulatedPacket();
                if (dynamic_cast<BPDU*>(frame))
                {
                    cPacket* frame = ((EtherFrame*)msg)->decapsulate();
                    BPDU* bpdu = dynamic_cast<BPDU*>(frame);
                    bpdu->setArrival(this,msg->getArrivalGateId());
                    this->handleBPDU(bpdu);
                }
                else
                {
                    // Etherframe incoming, handle it
                    EtherFrame* frame = dynamic_cast<EtherFrame*>(msg);
                    this->handleIncomingFrame(frame);
                    return;
                }
            }
            else
            {
                EV << "Non Ethernet frame arrived. Discarding it" << endl;
            }
        }
        else
        {
            EV << "Bridge not active. discarding packet" << endl;
        }
    }
    delete (msg);
}

void MACRelayUnitSTPNP::handleIncomingFrame(EtherFrame *msg)
{
    int inputPort = msg->getArrivalGate()->getIndex();
    if (this->port_status[inputPort].state != BLOCKING)
    {
        MACAddress dest(msg->getDest());
        int outputport = getPortForAddress(dest);
        if (outputport>=0 && this->port_status[outputport].state!=FORWARDING)
        {
            if (this->port_status[outputport].state==LEARNING)
            {
                EV << "Port in LEARNING state. updating address table" << endl;
                this->updateTableWithAddress(msg->getDest(),outputport);
            }
            EV << "Frame arrived on port " << msg->getArrivalGate()->getIndex() << " Addressed to a port not in forward mode. discarding it" << endl;
            delete(msg);
            return;
        }

        if (outputport>-1)
        {
            this->port_status[outputport].packet_forwarded++;
            if (this->port_status[outputport].packet_forwarded > this->packet_fwd_limit)
            {
                // possible loop.. forcing a bpdu transmission
                EV << "packet forward limit reached on port " << outputport << " forcing a bpdu transmission " << endl;
                this->sendConfigurationBPDU(outputport);
                this->port_status[outputport].packet_forwarded = 0;
            }
            //EV << "Port " << outputport << " forwarded frames " << this->port_status[outputport].packet_forwarded << endl;
        }
        // handle the frame with the legacy method to process it

        if (this->port_status[inputPort].state == FORWARDING)
            MACRelayUnitNP::handleIncomingFrame(msg);
        else if (this->port_status[inputPort].state == LEARNING)
        {
            AddressTable::iterator iter = addresstable.find(msg->getSrc());
            if (iter == addresstable.end() ||
                    !(this->port_status[iter->second.portno].state == FORWARDING && iter->second.insertionTime + DELTATIME > simTime()))
                updateTableWithAddress(msg->getSrc(), inputPort);
             delete msg;
             return;
        }
    }
    else
    {
        // discarding frame since the arrival port is blocked
        EV << "Port " << msg->getArrivalGate()->getIndex() << " incoming frame in blocked port. discarding" << endl;
        delete(msg);
    }
}

void MACRelayUnitSTPNP::handleTimer(cMessage* msg)
{
    if (dynamic_cast<STPStartProtocol*>(msg))
    {
        this->handleSTPStartTimer(check_and_cast<STPStartProtocol*>(msg));
    }
    else if (dynamic_cast<STPHelloTimer*>(msg))
    {
        this->handleSTPHelloTimer(check_and_cast<STPHelloTimer*>(msg));
    }
    else if (dynamic_cast<STPForwardTimer*>(msg))
    {
        this->handleSTPForwardTimer(check_and_cast<STPForwardTimer*>(msg));
    }
    else if (dynamic_cast<STPHoldTimer*>(msg))
    {
        this->handleSTPHoldTimer(check_and_cast<STPHoldTimer*>(msg));
    }
    else if (dynamic_cast<STPBPDUTimeoutTimer*>(msg))
    {
        this->handleSTPBPDUTimeoutTimer(check_and_cast<STPBPDUTimeoutTimer*>(msg));
    }
    else if (dynamic_cast<STPPortEdgeTimer*>(msg))
    {
        this->handleSTPPortEdgeTimer(check_and_cast<STPPortEdgeTimer*>(msg));
    }
    else
    {
        EV << "unknown type of timer arrived. ignoring it" <<endl;
    }
}

void MACRelayUnitSTPNP::handleSTPStartTimer(STPStartProtocol* t)
{
    this->active = true;

    EV << "Starting (or Restarting) RSTP Protocol." << endl;

    this->message_age = 0;
    this->priority_vector = PriorityVector(this->bridge_id,0,this->bridge_id,0);

    EV << "I'm the ROOT Bridge: Priority Vector: " << this->priority_vector << endl;

    this->setAllPortsStatus(PortStatus(LISTENING,DESIGNATED_PORT));
    this->sendConfigurationBPDU();
    this->scheduleHelloTimer();
    delete(t);
}

void MACRelayUnitSTPNP::handleSTPHelloTimer(STPHelloTimer* t)
{
    EV << "Hello Timer arrived" << endl;
    this->sendConfigurationBPDU();
    // schedule the timer again to the hello time
    this->scheduleHelloTimer();
}

void MACRelayUnitSTPNP::handleSTPForwardTimer(STPForwardTimer* t)
{
    int port = t->getPort();
    EV << "Forward timer arrived for port " << port << endl;

    if (this->port_status[port].state == LISTENING)
    {
        EV << "Port transition from LISTENING to LEARNING" << endl;
        this->setPortStatus(port, PortStatus(LEARNING,this->port_status[port].role));
    }
    if (this->port_status[port].state == LEARNING)
    {
        EV << "Port transition from LEARNING to FORWARDING" << endl;
        this->setPortStatus(port, PortStatus(FORWARDING,this->port_status[port].role));
    }

    // check topology change
    if (this->priority_vector.root_id != this->bridge_id)
    {
        // if i'm not the root.
        if (port!=this->getRootPort())
        {
            // send the TCN  via the root port informing "port" have change state
            this->sendTopologyChangeNotificationBPDU(port);
        }
        else
        {
            // root port have change state, we dont need to inform it.
        }
    }
    else
    {
        // i'm the root, set the topology change timeout
        this->topology_change_timeout = simTime() + this->max_age_time + this->forward_delay;
        EV << "setting the topology change timeout to " << this->topology_change_timeout << endl;
    }

}

void MACRelayUnitSTPNP::handleSTPHoldTimer(STPHoldTimer* t)
{
    int port = t->getPort();
    EV << "Hold timer arrived for port " << port << endl;
    if (!this->port_status[port].BPDUQueue.isEmpty())
    {
        BPDU* bpdu = (BPDU*)this->port_status[port].BPDUQueue.pop();
        this->sendBPDU(bpdu,port);
    }
}

void MACRelayUnitSTPNP::handleSTPBPDUTimeoutTimer(STPBPDUTimeoutTimer* t)
{
    EV << "BPDU TTL timeout arrived" << endl;

    // port where the timer was triggered
    int port = t->getPort();

    // TODO: notify with a bubble to improve the observation

    if (port == this->getRootPort())
    {
        // BPDU was lost in the root port. starting the root port recovery

        // preRootPortLost Hook
        this->preRootPortLost();

        EV << "Root port Lost! Starting recovery" << endl;
        // FastRecovery procedure when there is a backup root path
        int root_candidate = -1;
        for (int i=0; i<this->gateSize("lowerLayerOut"); i++)
        {
            if (this->port_status[i].role == BACKUP_PORT && this->port_status[i].observed_pr.bridge_id == this->priority_vector.bridge_id)
            {
                root_candidate = i;
                break;
            }
        }

        if (root_candidate>-1)
        {
            // there is a candidate to replace the lost root port
            // setting the old root port in designated mode
            int lost_root_port = this->getRootPort();
            EV << "replace lost root " << lost_root_port << " port by port " << root_candidate << " port status: " <<  this->port_status[root_candidate]<< endl;
            this->setPortStatus(root_candidate,PortStatus(LEARNING,ROOT_PORT));
            this->setPortStatus(lost_root_port,PortStatus(LISTENING,DESIGNATED_PORT));
            this->priority_vector = this->port_status[root_candidate].observed_pr;

            EV << "New Root Election: " << this->priority_vector << endl;
            // moving mac address from old root port to the new one
            this->moveMACAddresses(lost_root_port,root_candidate);
            // schedule the hello timer according the values received from the root bridge (RSTP)
            this->scheduleHelloTimer();
            // start updating our information to all the ports

            // postRootPortLost Hook
            this->postRootPortLost();

            this->sendConfigurationBPDU();

        }
        else
        {
            // i'm the root switch.
            this->handleTimer(new STPStartProtocol("Restart the RSTP Protocol"));
            // postRootPortLost Hook
            this->postRootPortLost();
        }
    }
    else
    {
        EV << "Port " << port << " losses the BPDU keep alive. port is not alive" << endl;
        this->port_status[port].alive = false;
        this->flushMACAddressesOnPort(port);
    }
}

void MACRelayUnitSTPNP::handleSTPPortEdgeTimer(STPPortEdgeTimer* t)
{
    ;
    int port = t->getPort();
    EV << "Port Edge timer arrived for port. passing it to forward state immediately " << port << endl;
    this->setPortStatus(port,PortStatus(FORWARDING,EDGE_PORT));
    this->port_status[port].clearPortEdgeTimer();
}

void MACRelayUnitSTPNP::handleTopologyChangeFlag(bool flag)
{
    if (flag)
    {
        this->agingTime = this->max_age_time + this->forward_delay;
        this->par("agingTime") = this->agingTime.dbl();
        EV << "BPDU is topology change flagged, reducing the mac aging time = " << this->agingTime << endl;
    }
    else
    {
        this->agingTime = this->original_mac_aging_time;
        this->par("agingTime") = this->agingTime.dbl();
        EV << "BPDU is NOT topology change flagged, mac aging time = " << this->agingTime << endl;
    }
}

// process incoming BPDU's
void MACRelayUnitSTPNP::handleBPDU(BPDU* bpdu)
{
    int arrival_port = bpdu->getArrivalGate()->getIndex();
    EV << bpdu->getName() << " arrived on port " << arrival_port << endl;

    if (this->isBPDUAged(bpdu))
    {
        EV << "Incoming BPDU age " << bpdu->getMessageAge() << " exceed the max age time " << this->max_age_time <<  ", blocking port" << endl;
        this->setPortStatus(arrival_port,PortStatus(BLOCKING,ALTERNATE_PORT));
        delete(bpdu);

        // check if we have lost the root port
        if (this->priority_vector.root_id != this->bridge_id && this->getRootPort()==-1)
        {
            // we loose the root port. restarting the protocol
            this->handleTimer(new STPStartProtocol("Restart the RSTP Protocol"));
        }

        return;
    }

    // if port is in listening mode, set it in learning mode, since there is a bridge connected
    if (this->port_status[arrival_port].state == LISTENING)
    {
        EV << "changing port " << arrival_port << " to learning state since there is a bridge connected" << endl;
        this->setPortStatus(arrival_port,PortStatus(LEARNING,this->port_status[arrival_port].role));
    }

    // restart the bpdu timeout timer
    this->restartBPDUTimeoutTimer(arrival_port);

    // check if topology change flag is set to reduce the aging mac timer
    this->handleTopologyChangeFlag(bpdu->getTopologyChangeFlag());

    // Handle the BPDU according its type.

    if (bpdu->getType() == CONF_BPDU)
    {
        this->handleConfigurationBPDU(check_and_cast<CBPDU*>(bpdu));
    }
    else if (bpdu->getType() == TCN_BPDU)
    {
        this->handleTopologyChangeNotificationBPDU(check_and_cast<TCNBPDU*>(bpdu));
    }
    else
    {
        EV << "Unknown BPDU, ignoring it" << endl;
    }
}
void MACRelayUnitSTPNP::handleConfigurationBPDU(CBPDU* bpdu)
{

    int arrival_port = bpdu->getArrivalGate()->getIndex();
    PriorityVector arrived_pr = PriorityVector(bpdu->getRootBID(),bpdu->getRootPathCost(),bpdu->getSenderBID(),bpdu->getPortId());

    EV << "Arrived PR: " << arrived_pr << endl;
    if (this->port_status[arrival_port].proposing)
    {
        EV << "Proposing PR: " << this->port_status[arrival_port].proposed_pr << endl;
    }
    if (this->port_status[arrival_port].proposed)
    {
        EV << "Proposed PR: " << this->port_status[arrival_port].observed_pr << endl;
    }
    EV << "ROOT PR: " << this->priority_vector << endl;

    // RSTP: check for port agreement allowing port fast transition
    if (bpdu->getAgreement())
    {
        if (this->port_status[arrival_port].proposing && arrived_pr == this->port_status[arrival_port].proposed_pr)
        {
            // arrived bpdu is exactly the same that we have proposed. bpdu should be an agreement and the port should be proposing

            EV << "BPDU agrees the proposal to fast port transition. set the port in FORWARDING state" << endl;
            this->setPortStatus(arrival_port,PortStatus(FORWARDING,this->port_status[arrival_port].role));
        }
        else
        {
            EV << "BPDU informs that is agree and we are not proposing. so, we discard the BPDU and we propose our Root information" << endl;
            this->port_status[arrival_port].proposing = true;
            this->sendConfigurationBPDU(arrival_port);
        }
        delete(bpdu);
        return;
    }

    this->recordPriorityVector(bpdu,arrival_port);

    if (arrived_pr > this->priority_vector)
    {
        EV << "received BPDU has superior priority vector." << endl;
        if (this->priority_vector.root_id == this->bridge_id)
        {
            EV << "This switch is no longer the root bridge" << endl;
        }
        // set the root port and the other port status (initially designated)
        this->recordRootTimerDelays(bpdu);
        this->setRootPort(arrival_port);

        EV << "current BPDU Age counter " << this->message_age << endl;

    }
    else if (arrived_pr == this->priority_vector)
    {
        EV << "Root Bridge informed is the same. current root:" <<  this->priority_vector << endl;
        if (arrival_port < this->getRootPort())
        {
            EV << "new bpdu informs a new path to the root tie-breaking by the local port id" << endl;

            if (this->priority_vector.root_id == this->bridge_id)
            {
                EV << "This switch is no longer the root bridge" << endl;
            }
            // set the root port and the other port status (initially designated)
            this->recordRootTimerDelays(bpdu);
            this->setRootPort(arrival_port);

        }
        else if (this->getRootPort()>-1 && arrival_port > this->getRootPort())
        {
            // same root bridge bpdu arrived on a lower priority port. blocking the port and leaving it as backup
            // NOTE: this port should be in Alternate mode according the RSTP protocol. but this case happened
            // when both bridges are connected by two parallel links. So it makes more sense to assigned this port
            // as backup in order to allow a faster recovery when the root port is lost.
            EV << "same root bridge BPDU arrived on a lower priority port. blocking the port and leave it in backup role" << endl;
            this->setPortStatus(arrival_port,PortStatus(BLOCKING,BACKUP_PORT));
        }
        else
        {
            if (this->getRootPort()>-1)
            {
                EV << "Keeping the root election." << endl;
                if (!bpdu->getAckFlag())
                {
                    EV << "updating message age" << endl;
                    this->message_age = bpdu->getMessageAge();
                }
            }
            else
            {
                // root port blocked by a redundant path before to get blocked. reactivating the root port
                EV << "root port blocked by a redundant path before to get blocked. reactivating the root port" << endl;
                this->recordRootTimerDelays(bpdu);
                this->setRootPort(arrival_port);
            }
        }
    }
    else
    {
        EV << "received BPDU does have inferior priority vector." << endl;

        if (arrived_pr.root_id == this->priority_vector.root_id)
        {
            EV << "BPDU informing the same root bridge we have selected" << endl;

            if (arrived_pr.root_path_cost > this->priority_vector.root_path_cost)
            {
                if (arrived_pr.bridge_id != this->bridge_id)
                {

                    //bool bp = arrived_pr.bridge_id < this->bridge_id ;

                    //EV << "arrived bid " << arrived_pr.bridge_id << " this bid " << this->bridge_id << " arrived < this ? " << bp << endl;

                    if (arrived_pr.bridge_id < this->bridge_id)
                    {

                        EV << "BPDU informs a lower priority switch connected to this network segment. port status " << this->port_status[arrival_port] << endl;

                    }
                    else
                    {
                        if (bpdu->getPortRole()==ROOT_PORT)
                        {
                            EV << "Inferior BPDU arrived from a root port. RSTP special case: Accepting it" << endl;
                            if (this->port_status[arrival_port].role==DESIGNATED_PORT)
                            {
                                EV << "Proposing a path to the root bridge" << endl;
                                this->port_status[arrival_port].proposing = true;

                                this->sendConfigurationBPDU(arrival_port);
                            }
                            else
                            {
                                EV << "lower priority BPDU arrived in a non designated port. just ignoring it" << endl;
                            }
                        }
                        else
                        {
                            EV << "BPDU informs that this port is an higher cost alternate path to the root. this port is BLOCKED/ALTERNATE_PORT" << endl;
                            if (this->port_status[arrival_port].proposing)
                            {
                                EV << "proposing our information" << endl;
                                this->sendConfigurationBPDU(arrival_port);
                            }
                            else
                            {
                                this->setPortStatus(arrival_port,PortStatus(BLOCKING,ALTERNATE_PORT));
                            }
                        }
                    }
                }
                else
                {
                    // BPDU comes from my self with a higher cost
                    if (arrival_port > arrived_pr.port_id)
                    {
                        EV << "BPDU informs a backup designated path to the same network segment. this port is BLOCKED/BACKUP_PORT" << endl;
                        this->setPortStatus(arrival_port,PortStatus(BLOCKING,BACKUP_PORT));
                    }
                    else
                    {
                        EV << "BPDU from my self arrived with better priority arrived. discard it" << endl;
                    }
                }
            }
            else if (arrived_pr.root_path_cost < this->priority_vector.root_path_cost)
            {
                EV << "BPDU informs an alternate better path to the root. This is the new ROOT port" << endl;
                if (this->priority_vector.root_id == this->bridge_id)
                {
                    EV << "This switch is no longer the root bridge" << endl;
                }
                // set the root port and the other port status (initially designated)
                this->recordRootTimerDelays(bpdu);
                this->setRootPort(arrival_port);
            }
            else
            {
                // root paths cost and arrived bpdu informs similar costs. analyzing the bridge_ids
                if (arrived_pr.bridge_id < this->priority_vector.bridge_id && arrived_pr.bridge_id != this->bridge_id)
                {
                    EV << "BPDU informs an same cost alternate path to the root bridge via a lower priority bridge. Port is BLOCKED/ALTERNATE_PORT" << endl;
                    if (this->port_status[arrival_port].proposing)
                    {
                        EV << "proposing our information" << endl;
                        this->sendConfigurationBPDU(arrival_port);
                    }
                    else
                    {
                        this->setPortStatus(arrival_port,PortStatus(BLOCKING,ALTERNATE_PORT));
                    }

                }
                else
                {
                    if (arrived_pr.bridge_id != this->bridge_id)
                    {
                        // BPDU comes from another bridge
                        if (arrived_pr.bridge_id == this->priority_vector.bridge_id)
                        {
                            EV << "BPDU informs a root backup path to the same network segment. this port is BLOCKED/BACKUP_PORT" << endl;
                            this->setPortStatus(arrival_port,PortStatus(BLOCKING,BACKUP_PORT));
                        }
                        else
                        {
                            // TODO: untie from sender port id
                            EV << "unhandled case." << endl;
                        }
                    }
                    else
                    {
                        // BPDU arrived from my self
                        if (arrival_port != this->getRootPort())
                        {
                            // but from a different port than root port
                            if (arrival_port > this->getRootPort())
                            {
                                EV << "BPDU informs a backup path to the same network segment. this port is BLOCKED/BACKUP_PORT" << endl;
                                this->setPortStatus(arrival_port,PortStatus(BLOCKING,BACKUP_PORT));

                            }
                            else if (arrival_port < this->getRootPort())
                            {
                                EV << "BPDU informs that this port is the best port connected to a network segment. this port is DESIGNATED" << endl;
                                this->setPortStatus(arrival_port,PortStatus(this->port_status[arrival_port].state,DESIGNATED_PORT));
                                EV << "Proposing a path to the root bridge" << endl;
                                this->port_status[arrival_port].proposing = true;
                                this->sendConfigurationBPDU(arrival_port);
                            }
                            else
                            {
                                // this case does not exists, since priority vectors are equal, case already handled
                                error("BPDU equal to the root priority vector, but identified as a low priority one. this situation should never happened");
                            }
                        }
                        else
                        {
                            // on the root port. discarding it
                            EV << "BPDU from my self arrived on my root port. discarding it" << endl;
                        }
                    }
                }
            }
        }
        else
        {
            EV << "BPDU informing a lower priority bridge as root bridge." << endl;
            if (this->port_status[arrival_port].role !=ROOT_PORT)
            {
                EV << "This port becomes DESIGNATED." << endl << "Proposing a path to our root bridge" << endl;
                this->setPortStatus(arrival_port,PortStatus(LEARNING,DESIGNATED_PORT));
                this->port_status[arrival_port].proposing = true;
                this->port_status[arrival_port].proposed = false;
                this->sendConfigurationBPDU(arrival_port);
            }
            else
            {
                EV << "This port becomes DESIGNATED." << endl << "We lost the root port. reinitializing protocol." << endl;
                this->handleTimer(new STPStartProtocol("Restart the RSTP Protocol"));
            }
        }

    }

    delete(bpdu);
}
void MACRelayUnitSTPNP::handleTopologyChangeNotificationBPDU(TCNBPDU* bpdu)
{
    int port = bpdu->getArrivalGate()->getIndex();

    if (this->priority_vector.root_id == this->bridge_id)
    {

        delete(bpdu);
        // TCN received at the root bridge, ack it and configure the topology change flag timeout
        // According the STP, we flag the BPDU with the Topology Change flag by a period of this->max_age_time + this->forward_delay
        this->topology_change_timeout = simTime()+this->max_age_time + this->forward_delay;
        EV << "setting the topology change timeout to " << this->topology_change_timeout << " and send the ACK on the same port (" << port << ")" << endl;
        this->sendTopologyChangeAckBPDU(port);

    }
    else
    {

        if (this->getRootPort() == port)
        {
            EV << "TCN arrived on the root port. discarding it" << endl;
            delete(bpdu);
        }
        else
        {

            if (this->port_status[port].state != BLOCKING)
            {
                EV << "TCN arrived on a non-root/non-blocked port. " << endl;
                EV << "ACK the TCN and forwarding it via the root port" << endl;

                // ack the TCN
                this->sendTopologyChangeAckBPDU(port);
                // forward the TCN via the root port
                bpdu->setMessageAge(0);
                bpdu->setSenderBID(this->bridge_id);
                bpdu->setPortId(this->getRootPort());
                this->sendBPDU(bpdu,this->getRootPort());
            }
            else
            {
                EV << "TCN arrived on a non-root/blocked port. discarding it" << endl;
                delete(bpdu);
            }
        }
    }

}

void MACRelayUnitSTPNP::sendConfigurationBPDU(int port_idx)
{

    if (port_idx>-1)
    {
        // get a new RST BPDU prepared to be sent by port_idx
        BPDU* bpdu = this->getNewRSTBPDU(port_idx);
        // send the BPDU
        this->sendBPDU(bpdu,port_idx);
    }
    else
    {
        // send the BPDU over all the ports (bpdu broadcast
        for (int port=0; port<numPorts; port++)
        {
            if (this->port_status[port].role == DESIGNATED_PORT || this->port_status[port].role == ROOT_PORT)
            {
                BPDU* bpdu = this->getNewRSTBPDU(port);
                // send the BPDU
                this->sendBPDU(bpdu,port);
            }
        }
    }
}

void MACRelayUnitSTPNP::sendTopologyChangeNotificationBPDU(int port_idx)
{
    if (this->getRootPort()>-1)
    {
        // get a new BPDU
        BPDU* bpdu = this->getNewBPDU(TCN_BPDU);
        // prepare the configuration BPDU
        bpdu->setPortId(port_idx);
        bpdu->setMessageAge(0);
        // send the BPDU
        this->sendBPDU(bpdu,this->getRootPort());
    }
    else
    {
        EV << "No root port defined. TCN BPDU transmission canceled" << endl;
    }
}

void MACRelayUnitSTPNP::sendTopologyChangeAckBPDU(int port_idx)
{

    // get a new BPDU
    BPDU* bpdu = this->getNewBPDU(CONF_BPDU);
    // prepare the configuration BPDU and flag it as ACK
    bpdu->setName("CBPDU+ACK");
    bpdu->setAckFlag(true);
    bpdu->setMessageAge(0);
    bpdu->setPortId(port_idx);
    // send the BPDU
    this->sendBPDU(bpdu,port_idx);
}

// send BPDU's

void MACRelayUnitSTPNP::sendBPDU(BPDU* bpdu,int port)
{

    if (!(this->port_status[port].isHoldTimerActive()))
    {
        // bpdu is meant to be send via the port id
        EV << "Sending " << bpdu->getName() << " via port " << port << endl;

        EtherFrameWithLLC* frame = new EtherFrameWithLLC(bpdu->getName());
        frame->setDsap(SAP_STP);
        frame->setSsap(SAP_STP);
        frame->setDest(MACRelayUnitSTPNP::STPMCAST_ADDRESS);
        frame->setSrc(this->bridge_id.address);

        BPDU* b = bpdu->dup();

        b->setPortRole(this->port_status[port].role);
        b->setForwarding(this->port_status[port].state == FORWARDING ? true : false);
        b->setLearning(this->port_status[port].state == LEARNING ? true : false);

        frame->encapsulate(b);
        if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"

        send(frame, "lowerLayerOut", port);
        this->scheduleHoldTimer(port);

        // record the proposed bpdu on this port
        this->port_status[port].proposed_pr = PriorityVector(b->getRootBID(),b->getRootPathCost(),b->getSenderBID(),b->getPortId());
        EV << "recording proposed bpdu: " << this->port_status[port].proposed_pr << endl;

    }
    else
    {
        EV << "Hold timer active on port " << port << " canceling transmission and enqueuing the BPDU" << endl;
        this->port_status[port].BPDUQueue.insert(bpdu);
        return;
    }
    delete bpdu;
}

BPDU* MACRelayUnitSTPNP::getNewBPDU(BPDUType type)
{
    // prepare a new bpdu

    BPDU* bpdu = NULL;
    switch (type)
    {
    case CONF_BPDU:
        bpdu = new CBPDU("CBPDU");
        break;
    case TCN_BPDU:
        bpdu = new TCNBPDU("TCN BPDU");
        break;
    }

    bpdu->setRootBID(this->priority_vector.root_id);
    //TODO: set the cost according the link speed
    bpdu->setRootPathCost(this->priority_vector.root_path_cost+1);
    bpdu->setSenderBID(this->bridge_id);
    bpdu->setMaxAge(this->max_age_time);
    bpdu->setHelloTime(this->hello_time);
    bpdu->setForwardDelay(this->forward_delay);

    // check if we need to flag the bpdu with the Topology Change flag (only for root bridge)
    if (this->priority_vector.root_id == this->bridge_id)
    {
        if (simTime() < this->topology_change_timeout)
        {
            bpdu->setTopologyChangeFlag(true);

            EV << "flagging BPDU with TC flag" << endl;
        }
        else
        {
            EV << "TC Flag is off" << endl;
        }
    }

    return bpdu;
}

BPDU* MACRelayUnitSTPNP::getNewRSTBPDU(int port)
{
    BPDU* bpdu = this->getNewBPDU(CONF_BPDU);
    bpdu->setPortId(port);

    if (this->port_status[port].role == ROOT_PORT)
    {
        // adjust message age
        bpdu->setMessageAge(0);
    }
    else
    {
        bpdu->setMessageAge(this->message_age+1);
    }

    // RSTP: set proposal flag we are proposing a transition on that port
    if (this->port_status[port].proposing)
    {
        bpdu->setProposal(true);
        bpdu->setAgreement(false);
        this->port_status[port].proposed_pr = PriorityVector(bpdu->getRootBID(),bpdu->getRootPathCost(),bpdu->getSenderBID(),bpdu->getPortId());
        EV << "proposed_pr: " << this->port_status[port].proposed_pr << endl;
    }

    // RSTP: set agreement flag when we are agree (all port synced) with the root bridge info
    if (this->port_status[port].agree)
    {
        bpdu->setProposal(false);
        bpdu->setAgreement(true);
        // use the observed_pr on the port to agree the proposal.
        // root bridge id comes already set from the getNewBPDU method
        bpdu->setSenderBID(this->port_status[port].observed_pr.bridge_id);
        bpdu->setPortId(this->port_status[port].observed_pr.port_id);
        bpdu->setRootPathCost(this->port_status[port].observed_pr.root_path_cost);
        bpdu->setMessageAge(0);
    }
    return bpdu;
}


bool MACRelayUnitSTPNP::isBPDUAged(BPDU* bpdu)
{
    // check the message age limit
    if (bpdu->getMessageAge() > this->max_age_time)
    {
        return true;
    }
    return false;
}

void MACRelayUnitSTPNP::broadcastFrame(EtherFrame *frame, int inputport)
{
    for (int i=0; i<numPorts; ++i)
        if (i!=inputport && this->port_status[i].state==FORWARDING)
            send((EtherFrame*)frame->dup(), "lowerLayerOut", i);
    delete frame;
}

void MACRelayUnitSTPNP::showPriorityVectorInfo()
{

    if (this->showInfo)
    {
        // the bridge/switch module must be the parent.
        cModule* bridge_module = this->getParentModule();

        if (bridge_module!=NULL)
        {
            // provisory: show only the path's cost

            std::stringstream tmp;
            tmp << this->priority_vector.root_path_cost;
            bridge_module->getDisplayString().setTagArg("t",0,tmp.str().c_str());
        }
    }

}
