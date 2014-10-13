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

#include "inet/linklayer/xmac/AnyMacXmacLayer.h"

#include <cassert>

#include "inet/common/INETUtils.h"
#include "inet/common/INETMath.h"


#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/linklayer/xmac/AnyMacXmacLayer.h"


namespace inet {

Define_Module( AnyMacXmacLayer )

/**
 * Initialize method of BMacLayer. Init all parameters, schedule timers.
 */
void AnyMacXmacLayer::initialize(int stage)
{
    MACProtocolBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

		queueLength = hasPar("queueLength") ? par("queueLength").longValue() : 10;
		animation = hasPar("animation") ? par("animation").boolValue() : true;
		slotDuration = hasPar("slotDuration") ? par("slotDuration").doubleValue() : 1.0;
		bitrate = hasPar("bitrate") ? par("bitrate").longValue() : 15360;
		headerLength = hasPar("headerLength") ? par("headerLength").longValue() : 10;
		checkInterval = hasPar("checkInterval") ? par("checkInterval").doubleValue() : 0.1;
		txPower = hasPar("txPower") ? par("txPower").doubleValue() : 50.0;
		useMacAcks = hasPar("useMACAcks") ? par("useMACAcks").boolValue() : false;
		maxTxAttempts = hasPar("maxTxAttempts") ? par("maxTxAttempts").longValue() : 2;
		ADDR_SIZE = hasPar("addrSize")  ? par("addrSize").longValue() : 8;
		EV_DETAIL << "headerLength: " << headerLength << ", bitrate: " << bitrate << endl;
		xmac = par("XMAC");

		useAck2 = par("useAck2"); // if the preamble has destination address can send ACK2
		PreambleAddress = par("PreambleAddress"); // Include the destination address in the preamble

		stats = par("stats");
		nbTxDataPackets = 0;
		nbTxPreambles = 0;
		nbRxDataPackets = 0;
		nbRxPreambles = 0;
		nbMissedAcks = 0;
		nbRecvdAcks=0;
		nbDroppedDataPackets=0;
		nbTxAcks=0;

		txAttempts = 0;
		lastDataPktDestAddr = MACAddress::BROADCAST_ADDRESS;
		lastDataPktSrcAddr  = MACAddress::BROADCAST_ADDRESS;

		macState = INIT;
		prevMacState = INIT;

        initializeMACAddress();
        registerInterface();

        cModule *radioModule = getParentModule()->getSubmodule("radio");
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);

		// init the dropped packet info
		WATCH(macState);
    }

    else if(stage == INITSTAGE_LINK_LAYER) {

		wakeup = new cMessage("wakeup");
		wakeup->setKind(XMAC_WAKE_UP);

		data_timeout = new cMessage("data_timeout");
		data_timeout->setKind(XMAC_DATA_TIMEOUT);
		data_timeout->setSchedulingPriority(100);

		data_tx_over = new cMessage("data_tx_over");
		data_tx_over->setKind(XMAC_DATA_TX_OVER);

		stop_preambles = new cMessage("stop_preambles");
		stop_preambles->setKind(XMAC_STOP_PREAMBLES);

		send_preamble = new cMessage("send_preamble");
		send_preamble->setKind(XMAC_SEND_PREAMBLE);

		ack_tx_over = new cMessage("ack_tx_over");
		ack_tx_over->setKind(XMAC_ACK_TX_OVER);

		cca_timeout = new cMessage("cca_timeout");
		cca_timeout->setKind(XMAC_CCA_TIMEOUT);
		cca_timeout->setSchedulingPriority(100);

		cca2_timeout = new cMessage("cca2_timeout");
		cca2_timeout->setKind(XMAC_CCA2_TIMEOUT);
		cca2_timeout->setSchedulingPriority(100);

		send_ack = new cMessage("send_ack");
		send_ack->setKind(XMAC_SEND_ACK);

		start_xmac = new cMessage("start_bmac");
		start_xmac->setKind(XMAC_START_XMAC);

		ack_timeout = new cMessage("ack_timeout");
		ack_timeout->setKind(XMAC_ACK_TIMEOUT);

		resend_data = new cMessage("resend_data");
		resend_data->setKind(XMAC_RESEND_DATA);
		resend_data->setSchedulingPriority(100);

		send_ack2 = new cMessage("XMAC_SEND_ACK2");
		send_ack2->setKind(XMAC_SEND_ACK2);

		ack2_tx_over = new cMessage("ack2_tx_over");
		ack2_tx_over->setKind(XMAC_ACK2_TX_OVER);

		preamble_tx_over = new cMessage("preamble_tx_over");
		preamble_tx_over->setKind(XMAC_PREAMBLE_TX_OVER);


		scheduleAt(0.0, start_xmac);
    }
}

AnyMacXmacLayer::~AnyMacXmacLayer()
{
	cancelAndDelete(wakeup);
	cancelAndDelete(data_timeout);
	cancelAndDelete(data_tx_over);
	cancelAndDelete(stop_preambles);
	cancelAndDelete(send_preamble);
	cancelAndDelete(ack_tx_over);
	cancelAndDelete(cca_timeout);
	cancelAndDelete(send_ack);
	cancelAndDelete(start_xmac);
	cancelAndDelete(ack_timeout);
	cancelAndDelete(resend_data);

	cancelAndDelete(preamble_tx_over);
	cancelAndDelete(send_ack2);
	cancelAndDelete(ack2_tx_over);
	cancelAndDelete(cca2_timeout);

	MacQueue::iterator it;
	for(it = macQueue.begin(); it != macQueue.end(); ++it)
	{
		delete (*it);
	}
	macQueue.clear();
}

void AnyMacXmacLayer::finish()
{
    // record stats
    if (stats)
    {
    	recordScalar("nbTxDataPackets", nbTxDataPackets);
    	recordScalar("nbTxPreambles", nbTxPreambles);
    	recordScalar("nbRxDataPackets", nbRxDataPackets);
    	recordScalar("nbRxPreambles", nbRxPreambles);
    	recordScalar("nbMissedAcks", nbMissedAcks);
    	recordScalar("nbRecvdAcks", nbRecvdAcks);
    	recordScalar("nbTxAcks", nbTxAcks);
    	recordScalar("nbDroppedDataPackets", nbDroppedDataPackets);
    	//recordScalar("timeSleep", timeSleep);
    	//recordScalar("timeRX", timeRX);
    	//recordScalar("timeTX", timeTX);
    }
}


void AnyMacXmacLayer::initializeMACAddress()
{
    const char *addrstr = par("address");

    if (!strcmp(addrstr, "auto"))
    {
        // assign automatic address
        address = MACAddress::generateAutoAddress();

        // change module parameter from "auto" to concrete address
        par("address").setStringValue(address.str().c_str());
    }
    else
    {
        address.setAddress(addrstr);
    }
}

InterfaceEntry *AnyMacXmacLayer::createInterfaceEntry()
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
    e->setMtu(par("mtu").longValue());
    e->setMulticast(false);
    e->setBroadcast(true);

    return e;
}


/**
 * Check whether the queue is not full: if yes, print a warning and drop the
 * packet. Then initiate sending of the packet, if the node is sleeping. Do
 * nothing, if node is working.
 */
void AnyMacXmacLayer::handleUpperPacket(cPacket *msg)
{
	bool pktAdded = addToQueue(msg);
	if (!pktAdded)
		return;
	// force wakeup now
	if (wakeup->isScheduled() && (macState == SLEEP))
	{
		cancelEvent(wakeup);
		scheduleAt(simTime() + dblrand()*0.1f, wakeup);
	}
}

void AnyMacXmacLayer::sendPreamble(const std::vector<MACAddress> &addr)
{
    XMacPkt* preamble = new XMacPkt();
	preamble->setSrcAddr(address);
	preamble->setDestAddr(MACAddress::BROADCAST_ADDRESS);
	preamble->setKind(XMAC_PREAMBLE);
	preamble->setDestinationAddressArraySize(addr.size());
	preamble->setName("XMAC_PREAMBLE2");
	preamble->setPreambleCont(preambleCont);
	preambleCont++;

	for (unsigned int i = 0;i<addr.size();i++)
		preamble->setDestinationAddress(i,addr[i]);

	preamble->setBitLength(headerLength+(addr.size()*ADDR_SIZE));

	//attach signal and send down
	attachSignal(preamble);
	sendDown(preamble);
	nbTxPreambles++;
}


/**
 * Send one short preamble packet immediately.
 */
void AnyMacXmacLayer::sendMacAck()
{
    XMacPkt* ack = new XMacPkt();
	ack->setSrcAddr(address);
	ack->setDestAddr(lastDataPktSrcAddr);
	ack->setKind(XMAC_ACK);
	ack->setBitLength(headerLength);
	ack->setName("XMAC_ACK");

	//attach signal and send down
	attachSignal(ack);
	sendDown(ack);
	nbTxAcks++;
	//endSimulation();
}

/**
 * Send one short preamble packet immediately.
 */
void AnyMacXmacLayer::sendMacAck2(const MACAddress & addr)
{
    XMacPkt* ack = new XMacPkt();
	ack->setSrcAddr(address);
	ack->setDestAddr(addr);
	ack->setKind(XMAC_ACK2);
	ack->setBitLength(headerLength);
	ack->setName("XMAC_ACK2");

	//attach signal and send down
	attachSignal(ack);
	sendDown(ack);
	nbTxAcks++;
	//endSimulation();
}

/**
 * Handle own messages:
 * XMAC_WAKEUP: wake up the node, check the channel for some time.
 * XMAC_CHECK_CHANNEL: if the channel is free, check whether there is something
 * in the queue and switch the radio to TX. When switched to TX, the node will
 * start sending preambles for a full slot duration. If the channel is busy,
 * stay awake to receive message. Schedule a timeout to handle false alarms.
 * XMAC_SEND_PREAMBLES: sending of preambles over. Next time the data packet
 * will be send out (single one).
 * XMAC_TIMEOUT_DATA: timeout the node after a false busy channel alarm. Go
 * back to sleep.
 */
void AnyMacXmacLayer::handleSelfMessage(cMessage *msg)
{
	if (msg->getKind() == XMAC_PREAMBLE && radio->getRadioMode() == IRadio::RADIO_MODE_RECEIVER)
	{
	    XMacPkt *macPkt = dynamic_cast<XMacPkt*>(msg);
		if (macPkt->getDestAddr() == MACAddress::BROADCAST_ADDRESS || macPkt->getDestAddr() == this->address)
		{
		    pendingPreambleIterator = pendingPreambles.find(macPkt->getSrcAddr());
		    if (pendingPreambleIterator == pendingPreambles.end())
			    pendingPreambles.insert(macPkt->getSrcAddr());
		}
	}
	switch (macState)
	{
	case INIT:
		if (msg->getKind() == XMAC_START_XMAC)
		{
			EV_DETAIL << "State INIT, message XMAC_START, new state SLEEP" << endl;
			changeDisplayColor(BLACK);
			radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
			macState = SLEEP;
			scheduleAt(simTime()+dblrand()*slotDuration, wakeup);
			return;
		}
		break;
	case SLEEP:
		if (msg->getKind() == XMAC_WAKE_UP)
		{
			EV_DETAIL << "State SLEEP, message XMAC_WAKEUP, new state CCA" << endl;
			scheduleAt(simTime() + checkInterval, cca_timeout);
			radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
			changeDisplayColor(GREEN);
			macState = CCA;
			pendingPreambles.clear();
			pendingPreambleIterator = pendingPreambles.end();
			prevMacState = SLEEP;
			destAddressNext = MACAddress::UNSPECIFIED_ADDRESS;
			destDestAddrAck2 = MACAddress::UNSPECIFIED_ADDRESS;
			return;
		}
		break;
	case CCA:
		if (msg->getKind() == XMAC_CCA_TIMEOUT)
		{
			// channel is clear
			// something waiting in eth queue?

			// if not, go back to sleep and wake up after a full period
			if (macQueue.empty())
			{
				EV_DETAIL << "State CCA, message CCA_TIMEOUT, new state SLEEP" << endl;
				if (prevMacState == WAIT_DATA)
				    scheduleAt(simTime() + slotDuration - checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				prevMacState = CCA;
				radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
				changeDisplayColor(BLACK);
				return;
			}
			if (prevMacState != WAIT_DATA)
			{
				EV_DETAIL << "State CCA, message CCA_TIMEOUT, new state"
						  " SEND_PREAMBLE" << endl;
				radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
				changeDisplayColor(YELLOW);
				macState = SEND_PREAMBLE;
				prevMacState = CCA;
				scheduleAt(simTime() + slotDuration, stop_preambles);
				return;
			}
			else
			{
				// continue a very small random period in CCA
				prevMacState = CCA;
				scheduleAt(simTime() + checkInterval*dblrand()*0.5f, cca_timeout);
				return;
			}
		}
		// during CCA, we received a preamble. Go to state WAIT_DATA and
		// schedule the timeout.
		if (msg->getKind() == XMAC_PREAMBLE)
		{
			nbRxPreambles++;
			XMacPkt *lmacPkt = dynamic_cast<XMacPkt*>(msg);
			if (lmacPkt)
			{
				// check if address in list
				bool inList = false;
				for (unsigned int i=0;i<lmacPkt->getDestinationAddressArraySize();i++)
					if (lmacPkt->getDestinationAddress(i)==address)
						inList = true;
				if (xmac)
				{
				    cancelEvent(cca_timeout);
				    if (inList)
				    {
			            macState = SEND_ACK2;
			            prevMacState = CCA;
			            radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
			            changeDisplayColor(YELLOW);
			            delete msg;
			            return;
				    }
				    else
				    {
                        scheduleAt(simTime() + slotDuration, wakeup);
                        macState = SLEEP;
                        prevMacState = CCA;
                        radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
                        changeDisplayColor(BLACK);
                        delete msg;
                        return;
				    }
				}
				else
				{
                    if (inList) {
                        macState = CCA2;
                        prevMacState = CCA;
                        double delay = slotDuration * 0.1 * dblrand();
                        scheduleAt(simTime() + delay, cca2_timeout);
                        destDestAddrAck2 = lmacPkt->getSrcAddr();
                        cancelEvent(cca_timeout);
                    } 
                    else 
                    {
                        macState = CCA_LOST;
                        prevMacState = CCA;
                    }
                }
			}
			else
			{
			    XMacPkt *macPkt = dynamic_cast<XMacPkt*>(msg);
				if (macPkt->getDestAddr() != MACAddress::BROADCAST_ADDRESS && macPkt->getDestAddr() != this->address)
				{
					// not for my  CCA_LOST
					EV_DETAIL << "State CCA, message XMAC_PREAMBLE received, new state"
											" CCA_LOST" << endl;
					macState = CCA_LOST;
					prevMacState = CCA;
				}
				else
				{
					EV_DETAIL << "State CCA, message XMAC_PREAMBLE received, new state"
						" WAIT_DATA" << endl;
					if (useAck2 && PreambleAddress && macPkt->getDestAddr() != MACAddress::BROADCAST_ADDRESS)
					{
						macState = CCA2;
						prevMacState = CCA;
						scheduleAt(simTime(), cca2_timeout);
						destDestAddrAck2 = macPkt->getSrcAddr();
						cancelEvent(cca_timeout);
					}
					else
						macState = WAIT_DATA;
					cancelEvent(cca_timeout);
					//if (prevMacState != WAIT_DATA)
				    scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
					prevMacState = CCA;
				}
			}
			delete msg;
			return;
		}
		// this case is very, very, very improbable, but let's do it.
		// if in CCA and the node receives directly the data packet, switch to
		// state WAIT_DATA and re-send the message
		if (msg->getKind() == XMAC_DATA)
		{
			nbRxDataPackets++;
			EV_DETAIL << "State CCA, message XMAC_DATA, new state WAIT_DATA"
				   << endl;
			macState = WAIT_DATA;
			prevMacState = CCA;
			cancelEvent(cca_timeout);
			scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
			scheduleAt(simTime(), msg);
			return;
		}
		//in case we get an ACK, we simply dicard it, because it means the end
		//of another communication
		if (msg->getKind() == XMAC_ACK)
		{
			EV_DETAIL << "State CCA, message XMAC_ACK, new state CCA" << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == XMAC_ACK2)
		{
			EV_DETAIL << "State CCA, message XMAC_ACK2, new state CCA_LOST" << endl;
			// XMAC_ACK2 works like RTS/CTS
			macState = CCA_LOST;
			prevMacState = CCA;
			delete msg;
			return;
		}
		break;
	case CCA_LOST:
		if (msg->getKind() == XMAC_CCA_TIMEOUT)
		{
			EV_DETAIL << "State CCA, message CCA_TIMEOUT, new state SLEEP" << endl;
			scheduleAt(simTime() + slotDuration, wakeup);
			macState = SLEEP;
			prevMacState = CCA_LOST;
			radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
			changeDisplayColor(BLACK);
			return;
		}
		// during CCA, we received a preamble. Go to state WAIT_DATA and
		// schedule the timeout.
		if (msg->getKind() == XMAC_PREAMBLE)
		{
			nbRxPreambles++;
			XMacPkt *macPkt = dynamic_cast<XMacPkt*>(msg);
			// check if address in list
			bool inList = false;
			for (unsigned int i=0;i<macPkt->getDestinationAddressArraySize();i++)
			    if (macPkt->getDestinationAddress(i)==address)
			        inList = true;
			if (inList)
			{
			    macState = CCA2;
			    prevMacState = CCA_LOST;
			    double delay = slotDuration*0.1*dblrand();
			    scheduleAt(simTime() + delay, cca2_timeout);
			    destDestAddrAck2 = macPkt->getSrcAddr();
			    cancelEvent(cca_timeout);
			}
			delete msg;
			return;
		}
		// this case is very, very, very improbable, but let's do it.
		// if in CCA and the node receives directly the data packet, switch to
		// state WAIT_DATA and re-send the message
		if (msg->getKind() == XMAC_DATA)
		{
			nbRxDataPackets++;
			EV_DETAIL << "State CCA_LOST, message XMAC_DATA, new state WAIT_DATA"
				   << endl;
			macState = WAIT_DATA;
			prevMacState = CCA_LOST;
			cancelEvent(cca_timeout);
			scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
			scheduleAt(simTime(), msg);
			return;
		}
		//in case we get an ACK, we simply dicard it, because it means the end
		//of another communication
		if (msg->getKind() == XMAC_ACK)
		{
			EV_DETAIL << "State CCA_LOST, message XMAC_ACK, new state CCA" << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == XMAC_ACK2)
		{
			EV_DETAIL << "State CCA_LOST, message XMAC_ACK2, new state CCA_LOST" << endl;
			// the XMAC_ACK2 works like RTS/CTS
			delete msg;
			return;
		}
		break;
	case CCA2:
		if (msg->getKind() == XMAC_CCA2_TIMEOUT)
		{
			macState = SEND_ACK2;
			prevMacState = CCA2;
			cancelEvent(data_timeout);
			radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
			changeDisplayColor(YELLOW);
			return;
		}
		// during CCA, we received a preamble. Go to state WAIT_DATA and
		// schedule the timeout.
		if (msg->getKind() == XMAC_ACK2)
		{
			XMacPkt *macPkt = dynamic_cast<XMacPkt*>(msg);
			if (macPkt->getDestAddr() == destDestAddrAck2) // other node has winned
			{
				scheduleAt(simTime() + slotDuration, wakeup);
				cancelEvent(cca2_timeout);
	            cancelEvent(data_timeout);
				radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
				changeDisplayColor(BLACK);
                macState = SLEEP;
                prevMacState = CCA2;
				delete msg;
				return;
			}
			else
			{
			    // continue waiting
                delete msg;
                return;
			}
		}
		if (msg->getKind() == XMAC_PREAMBLE)
		{
			EV_DETAIL << "State CCA2, message XMAC_PREAMBLE, new state CCA2" << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == XMAC_DATA)
		{
			nbRxDataPackets++;
			EV_DETAIL << "State CCA2, message XMAC_DATA, new state WAIT_DATA" << endl;
			cancelEvent(cca2_timeout);
			if (!data_timeout->isScheduled())
 	            scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
			scheduleAt(simTime(), msg);
            macState = WAIT_DATA;
            prevMacState = CCA2;
			return;
		}
		//in case we get an ACK, we simply dicard it, because it means the end
		//of another communication
		if (msg->getKind() == XMAC_ACK)
		{
			EV_DETAIL << "State CCA2, message XMAC_ACK, new state CCA2" << endl;
			delete msg;
			return;
		}

        if (msg->getKind() == XMAC_DATA_TIMEOUT)
        {
            EV_DETAIL << "State CCA2, message XMAC_DATA_TIMEOUT, new state"
                      " SLEEP" << endl;
            // if something in the queue, wakeup soon.
            if (macQueue.size() > 0)
                scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
            else
                scheduleAt(simTime() + slotDuration, wakeup);
            cancelEvent(cca2_timeout);
            cancelEvent(data_timeout);
            macState = SLEEP;
            prevMacState = CCA2;
            radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
            changeDisplayColor(BLACK);
            return;
        }
		break;
	case SEND_PREAMBLE:
		// check if anycast
	    if (msg->getKind() == XMAC_SEND_PREAMBLE)
	    {
	        EV_DETAIL << "State SEND_PREAMBLE, message XMAC_SEND_PREAMBLE, new"
	                " state SEND_PREAMBLE" << endl;
	        std::vector<MACAddress> addr;
	        XMacPkt * pkt = dynamic_cast<XMacPkt*>(macQueue.front());
	        for (unsigned int i = 0; i < pkt->getDestinationAddressArraySize();i++)
	            addr.push_back(pkt->getDestinationAddress(i));
	        sendPreamble(addr);
	        // scheduleAt(simTime() + 0.5f*checkInterval, send_preamble);
	        scheduleAt(simTime() + 0.5f*checkInterval, ack_timeout);
	        macState = SEND_PREAMBLE_TX_END;
	        prevMacState = SEND_PREAMBLE;
	        return;
	    }
	    // simply change the state to SEND_DATA
	    if (msg->getKind() == XMAC_STOP_PREAMBLES)
	    {
	        EV_DETAIL << "State SEND_PREAMBLE, message XMAC_STOP_PREAMBLES, new"
	                " state SEND_DATA" << endl;
	        macState = SEND_DATA;
	        prevMacState = SEND_PREAMBLE;
	        txAttempts = 1;
	        return;
	    }
	    break;
	case SEND_PREAMBLE_TX_END:
		if (msg->getKind() == XMAC_PREAMBLE_TX_OVER)
		{
			macState = WAIT_ACK2;
			prevMacState = SEND_PREAMBLE_TX_END;
			radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
			changeDisplayColor(GREEN);
			// scheduleAt(simTime()+checkInterval, ack_timeout);
			return;
		}
		break;
	case SEND_DATA:
		if ((msg->getKind() == XMAC_SEND_PREAMBLE)
			|| (msg->getKind() == XMAC_RESEND_DATA))
		{
			EV_DETAIL << "State SEND_DATA, message XMAC_SEND_PREAMBLE or"
					  " XMAC_RESEND_DATA, new state WAIT_TX_DATA_OVER" << endl;
			// send the data packet
			sendDataPacket();
			macState = WAIT_TX_DATA_OVER;
			prevMacState = SEND_DATA;
			return;
		}
		break;
	case SEND_DATA2:
		if ((msg->getKind() == XMAC_SEND_PREAMBLE) || (msg->getKind() == XMAC_RESEND_DATA))
		{
			EV_DETAIL << "State SEND_DATA, message XMAC_SEND_PREAMBLE or"
					  " XMAC_RESEND_DATA, new state WAIT_TX_DATA_OVER" << endl;
			// send the data packet
			sendDataPacket(destAddressNext);
			macState = WAIT_TX_DATA_OVER;
			prevMacState = SEND_DATA2;
			return;
		}
		break;

	case WAIT_TX_DATA_OVER:
		if (msg->getKind() == XMAC_DATA_TX_OVER)
		{
			if ((useMacAcks) && !lastDataPktDestAddr.isBroadcast())
			{
				EV_DETAIL << "State WAIT_TX_DATA_OVER, message XMAC_DATA_TX_OVER,"
						  " new state WAIT_ACK" << endl;
				macState = WAIT_ACK;
				prevMacState = WAIT_TX_DATA_OVER;
				radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
				changeDisplayColor(GREEN);
				scheduleAt(simTime()+checkInterval, ack_timeout);
			}
			else
			{
				EV_DETAIL << "State WAIT_TX_DATA_OVER, message XMAC_DATA_TX_OVER,"
						  " new state  SLEEP" << endl;
				delete macQueue.front();
				macQueue.pop_front();
				preambleCont = 0;
				// if something in the queue, wakeup soon.
				if (macQueue.size() > 0)
					scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				prevMacState = WAIT_TX_DATA_OVER;
				radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
				changeDisplayColor(BLACK);
			}
			return;
		}
		break;
	case WAIT_ACK:
		if (msg->getKind() == XMAC_ACK_TIMEOUT)
		{
			// No ACK received. try again or drop.
			if (txAttempts < maxTxAttempts)
			{
				EV_DETAIL << "State WAIT_ACK, message XMAC_ACK_TIMEOUT, new state"
						  " SEND_DATA" << endl;
				txAttempts++;
				macState = SEND_PREAMBLE;
				prevMacState = WAIT_ACK;
				scheduleAt(simTime() + slotDuration, stop_preambles);
				radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
				changeDisplayColor(YELLOW);
			}
			else
			{
				EV_DETAIL << "State WAIT_ACK, message XMAC_ACK_TIMEOUT, new state"
						  " SLEEP" << endl;
				delete macQueue.front();
				macQueue.pop_front();
				preambleCont = 0;
				// if something in the queue, wakeup soon.
				if (macQueue.size() > 0)
					scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				prevMacState = WAIT_ACK;
				radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
				changeDisplayColor(BLACK);
				nbMissedAcks++;
			}
			return;
		}
		//ignore and other packets
		if ((msg->getKind() == XMAC_DATA) || (msg->getKind() == XMAC_PREAMBLE) || (msg->getKind() == XMAC_ACK2))
		{
			EV_DETAIL << "State WAIT_ACK, message XMAC_DATA or XMAC_PREMABLE, new"
					  " state WAIT_ACK" << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == XMAC_ACK)
		{
			EV_DETAIL << "State WAIT_ACK, message XMAC_ACK" << endl;
			XMacPkt*  mac = static_cast<XMacPkt *>(msg);
			const MACAddress& src = mac->getSrcAddr();
			// the right ACK is received..
			EV_DETAIL << "We are waiting for ACK from : " << lastDataPktDestAddr
				   << ", and ACK came from : " << src << endl;
			if (src == lastDataPktDestAddr)
			{
				EV_DETAIL << "New state SLEEP" << endl;
				nbRecvdAcks++;
				lastDataPktDestAddr = MACAddress::BROADCAST_ADDRESS;
				cancelEvent(ack_timeout);
				delete macQueue.front();
				macQueue.pop_front();
				preambleCont = 0;
				// if something in the queue, wakeup soon.
				if (macQueue.size() > 0)
					scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
				else
					scheduleAt(simTime() + slotDuration, wakeup);
				macState = SLEEP;
				prevMacState = WAIT_ACK;
				radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
				changeDisplayColor(BLACK);
				lastDataPktDestAddr = MACAddress::BROADCAST_ADDRESS;
			}
			delete msg;
			return;
		}
		break;
	case WAIT_DATA:
		if(msg->getKind() == XMAC_PREAMBLE && dynamic_cast<XMacPkt*>(msg))
		{
			//nothing happens
			EV_DETAIL << "State WAIT_DATA, message XMAC_PREAMBLE, new state"
					  " WAIT_DATA" << endl;
			nbRxPreambles++;
			XMacPkt *lmacPkt = dynamic_cast<XMacPkt*>(msg);
			if (lmacPkt)
			{
				// check if address in list
				bool inList = false;
				for (unsigned int i=0;i<lmacPkt->getDestinationAddressArraySize();i++)
					if (lmacPkt->getDestinationAddress(i)==address)
						inList = true;
				if (inList)
				{
					double delay = slotDuration*0.1*dblrand();
					if (data_timeout->isScheduled() && data_timeout->getArrivalTime()-simTime()>delay)
					{
						macState = CCA2;
						prevMacState = WAIT_DATA;
						//cancelEvent(data_timeout);
						scheduleAt(simTime() + delay, cca2_timeout);
						destDestAddrAck2 = lmacPkt->getSrcAddr();
					}
				}
			}
			delete msg;
			return;
		}
		if(msg->getKind() == XMAC_PREAMBLE)
		{
			//nothing happens
			EV_DETAIL << "State WAIT_DATA, message XMAC_PREAMBLE, new state"
					  " WAIT_DATA" << endl;
			nbRxPreambles++;
			delete msg;
			return;
		}
		if(msg->getKind() == XMAC_ACK)
		{
			//nothing happens
			EV_DETAIL << "State WAIT_DATA, message XMAC_ACK, new state WAIT_DATA"
				   << endl;
			delete msg;
			return;
		}
		if(msg->getKind() == XMAC_ACK2)
		{
			//nothing happens
			EV_DETAIL << "State WAIT_DATA, message XMAC_ACK2, new state WAIT_DATA"
				   << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == XMAC_DATA)
		{
			nbRxDataPackets++;
			XMacPkt* mac  = static_cast<XMacPkt *>(msg);
			const MACAddress dest = mac->getDestAddr();
			const MACAddress src  = mac->getSrcAddr();
			pendingPreambleIterator = pendingPreambles.find(src);
			if (pendingPreambleIterator != pendingPreambles.end())
				pendingPreambles.erase(pendingPreambleIterator);

			if ((dest == address) || dest.isBroadcast())
			{
				sendUp(decapsMsg(mac));
			} else {
				delete msg;
				msg = NULL;
				mac = NULL;
			}

			cancelEvent(data_timeout);
			if ((useMacAcks) && (dest == address))
			{
				EV_DETAIL << "State WAIT_DATA, message XMAC_DATA, new state"
						  " SEND_ACK" << endl;
				macState = SEND_ACK;
				prevMacState = WAIT_DATA;
				lastDataPktSrcAddr = src;
				radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
				changeDisplayColor(YELLOW);
			}
			else
			{
				if (!dest.isBroadcast() && pendingPreambles.empty())
				{
					EV_DETAIL << "State WAIT_DATA, message XMAC_DATA, new state SLEEP" << endl;
					// if something in the queue, wakeup soon.
					if (macQueue.size() > 0)
						scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
					else
						scheduleAt(simTime() + slotDuration, wakeup);
					macState = SLEEP;
					prevMacState = WAIT_DATA;
					radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
					changeDisplayColor(BLACK);
				}
				else
				{
				// allow that other messages sent by flooding can arrive.
					EV_DETAIL << "State WAIT_DATA, message broadcast, new state CCA" << endl;
					scheduleAt(simTime() + checkInterval, cca_timeout);
					macState = CCA;
					prevMacState = WAIT_DATA;
					destAddressNext = MACAddress::UNSPECIFIED_ADDRESS;
					destDestAddrAck2 = MACAddress::UNSPECIFIED_ADDRESS;

				}
			}
			return;
		}
		if (msg->getKind() == XMAC_DATA_TIMEOUT)
		{
			EV_DETAIL << "State WAIT_DATA, message XMAC_DATA_TIMEOUT, new state"
					  " SLEEP" << endl;
			// if something in the queue, wakeup soon.
			if (macQueue.size() > 0)
				scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
			else
				scheduleAt(simTime() + slotDuration, wakeup);
			macState = SLEEP;
			prevMacState = WAIT_DATA;
			radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
			changeDisplayColor(BLACK);
			return;
		}
		break;
	case SEND_ACK:
		if (msg->getKind() == XMAC_SEND_ACK)
		{
			EV_DETAIL << "State SEND_ACK, message XMAC_SEND_ACK, new state"
					  " WAIT_ACK_TX" << endl;
			// send now the ack packet
			sendMacAck();
			macState = WAIT_ACK_TX;
			prevMacState = SEND_ACK;
			return;
		}
		break;
	case WAIT_ACK_TX:
		if (msg->getKind() == XMAC_ACK_TX_OVER)
		{
			EV_DETAIL << "State WAIT_ACK_TX, message XMAC_ACK_TX_OVER, new state"
					  " SLEEP" << endl;
			// ack sent, go to sleep now.
			// if something in the queue, wakeup soon.
			if (macQueue.size() > 0)
				scheduleAt(simTime() + dblrand()*checkInterval, wakeup);
			else
				scheduleAt(simTime() + slotDuration, wakeup);
			macState = SLEEP;
			prevMacState = WAIT_ACK_TX;
			radio->setRadioMode(IRadio::RADIO_MODE_SLEEP);
			changeDisplayColor(BLACK);
			lastDataPktSrcAddr = MACAddress::BROADCAST_ADDRESS;
			return;
		}
		break;
	case SEND_ACK2:
		if (msg->getKind() == XMAC_SEND_ACK2)
		{
			EV_DETAIL << "State SEND_ACK, message XMAC_SEND_ACK, new state"
					  " WAIT_ACK_TX2" << endl;
			// send now the ack packet
			sendMacAck2(destDestAddrAck2);
			macState = WAIT_ACK2_TX;
			prevMacState = SEND_ACK2;
			return;
		}
		break;
	case WAIT_ACK2_TX:
		if (msg->getKind() == XMAC_ACK2_TX_OVER)
		{
			macState = WAIT_DATA;
			prevMacState = WAIT_ACK2_TX;
			radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
			changeDisplayColor(GREEN);
			if (!data_timeout->isScheduled())
			    scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
			return;
		}
		break;
	case WAIT_ACK2:
		// simply change the state to SEND_DATA
		if (msg->getKind() == XMAC_ACK_TIMEOUT)
		{
			macState = SEND_PREAMBLE;
			prevMacState = WAIT_ACK2;
			radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
			changeDisplayColor(YELLOW);
			return;
		}

		if (msg->getKind() == XMAC_STOP_PREAMBLES)
		{
			EV_DETAIL << "State SEND_PREAMBLE, message XMAC_STOP_PREAMBLES, new"
					  " state SEND_DATA" << endl;
			radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
			changeDisplayColor(YELLOW);
			cancelEvent(ack_timeout);
			macState = SEND_DATA;
			prevMacState = WAIT_ACK2;
			txAttempts = 1;
			return;
		}

		//ignore and other packets

		if (msg->getKind() == XMAC_PREAMBLE)
		{
		    XMacPkt *macPkt = dynamic_cast<XMacPkt*>(msg);
		    if (preambleCont < macPkt->getPreambleCont())
		    {
		        // lost
	            cancelEvent(ack_timeout);
	            cancelEvent(stop_preambles);
	            if (macPkt->getDestAddr() != MACAddress::BROADCAST_ADDRESS && macPkt->getDestAddr() != this->address)
	            {
	                // not for my  CCA_LOST
	                EV_DETAIL << "State CCA, message XMAC_PREAMBLE received, new state"
	                        " CCA_LOST" << endl;
	                macState = CCA_LOST;
	                prevMacState = WAIT_ACK2;
	            }
	            else
	            {
	                if (macPkt->getDestAddr() == this->address)
	                {
	                    macState = SEND_ACK2;
	                    prevMacState = WAIT_ACK2;
	                    cancelEvent(data_timeout);
	                    radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
	                    changeDisplayColor(YELLOW);
	                }
	                else
	                {
	                    macState = WAIT_DATA;
	                    cancelEvent(cca_timeout);
	                    //if (prevMacState != WAIT_DATA)
	                    scheduleAt(simTime() + slotDuration + checkInterval, data_timeout);
	                    prevMacState = WAIT_ACK2;
	                }
	            }
		    }
		    delete msg;
		    return;
		}

		if ((msg->getKind() == XMAC_DATA)  || (msg->getKind() == XMAC_ACK))
		{
			EV_DETAIL << "State WAIT_ACK2, message XMAC_DATA or XMAC_PREMABLE, new"
					  " state WAIT_ACK2" << endl;
			delete msg;
			return;
		}
		if (msg->getKind() == XMAC_ACK2)
		{
			XMacPkt*  mac = dynamic_cast<XMacPkt *>(msg);
			const MACAddress dest = mac->getDestAddr();
			const MACAddress src = mac->getSrcAddr();
			// ack2 for other destination
			if (dest != address)
			{
				EV_DETAIL << "State WAIT_ACK2, message XMAC_ACK2, new state WAIT_ACK2" << endl;
				delete msg;
				return;
			}
			cancelEvent(ack_timeout);
			cancelEvent(stop_preambles);
			// the right ACK is received..
			destAddressNext = src;
			radio->setRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
			changeDisplayColor(YELLOW);
			EV_DETAIL << "State WAIT_ACK2, message XMAC_ACK2, new state SEND_DATA2" << endl;
			macState = SEND_DATA2;
			prevMacState = WAIT_ACK2;
			delete msg;
			return;
		}
		break;
	}
	opp_error("Undefined event of type %d in MAC stat %i", msg->getKind(), macState);
}


/**
 * Handle BMAC preambles and received data packets.
 */
void AnyMacXmacLayer::handleLowerPacket(cPacket *msg)
{
    if (msg->hasBitError()) {
        EV << "Received " << msg << " contains bit errors or collision, dropping it\n";
        delete msg;
        return;
    }
    else
        // simply pass the massage as self message, to be processed by the FSM.
        handleSelfMessage(msg);
}

void AnyMacXmacLayer::sendDataPacket(const MACAddress & add)
{
	nbTxDataPackets++;
	XMacPkt *pkt = macQueue.front()->dup();
	attachSignal(pkt);
	lastDataPktDestAddr = pkt->getDestAddr();
	pkt->setKind(XMAC_DATA);
	sendDown(pkt);
}

/**
 * Handle transmission over messages: either send another preambles or the data
 * packet itself.
 */
void AnyMacXmacLayer::receiveSignal(cComponent *source, simsignal_t signalID, long value)
{
    Enter_Method_Silent();
	// Transmission of one packet is over
    if (signalID == IRadio::radioModeChangedSignal)
    {
        IRadio::RadioMode radioMode = (IRadio::RadioMode)value;
        if (radioMode == IRadio::RADIO_MODE_TRANSMITTER)
        {
            if (macState == WAIT_TX_DATA_OVER)
            {
                scheduleAt(simTime(), data_tx_over);
            }
            if (macState == WAIT_ACK_TX)
            {
                scheduleAt(simTime(), ack_tx_over);
            }
            if (macState == WAIT_ACK2_TX)
            {
                scheduleAt(simTime(), ack2_tx_over);
            }
            if (macState == SEND_PREAMBLE_TX_END)
            {
                scheduleAt(simTime(), preamble_tx_over);
            }
        }
    }
    // Radio switching (to RX or TX) ir over, ignore switching to SLEEP.
    else if (signalID == IRadio::transmissionStateChangedSignal) {
    	// we just switched to TX after CCA, so simply send the first
    	// sendPremable self message
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE)
        {
            if (macState == SEND_PREAMBLE)
            {
                scheduleAt(simTime(), send_preamble);
            }
            if (macState == SEND_ACK2)
            {
                scheduleAt(simTime(), send_ack2);
            }
            if (macState == SEND_ACK)
            {
                scheduleAt(simTime(), send_ack);
            }
            // we were waiting for acks, but none came. we switched to TX and now
            // need to resend data
            if (macState == SEND_DATA)
            {
                scheduleAt(simTime(), resend_data);
            }
            if (macState == SEND_DATA2)
            {
                scheduleAt(simTime(), resend_data);
            }
        }
    }
}

/**
 * Encapsulates the received network-layer packet into a MacPkt and set all
 * needed header fields.
 */

bool AnyMacXmacLayer::addToQueue(cMessage *msg)
{
    if (macQueue.size() >= queueLength) {
        // queue is full, message has to be deleted
        EV_DETAIL << "New packet arrived, but queue is FULL, so new packet is"
                  " deleted\n";
        emit(packetFromUpperDroppedSignal, msg);
        nbDroppedDataPackets++;
        return false;
    }

    XMacPkt *macPkt = encapsMsg((cPacket *)msg);
    macQueue.push_back(macPkt);
    EV_DETAIL << "Max queue length: " << queueLength << ", packet put in queue"
              "\n  queue size: " << macQueue.size() << " macState: "
              << macState << endl;
    return true;
}

cPacket* AnyMacXmacLayer::decapsMsg(XMacPkt * msg)
{
    cPacket *m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    // delete the macPkt
    delete msg;
    EV_DETAIL << " message decapsulated " << endl;
    return m;
}

XMacPkt *AnyMacXmacLayer::encapsMsg(cPacket *netwPkt)
{
    XMacPkt *pkt = new XMacPkt(netwPkt->getName(), netwPkt->getKind());
    pkt->setBitLength(headerLength);

    IMACProtocolControlInfo* cInfo = check_and_cast<IMACProtocolControlInfo *>(netwPkt->removeControlInfo());
    EV_DETAIL << "CInfo removed, mac addr=" << cInfo->getDestinationAddress() << endl;
    pkt->setDestAddr(cInfo->getDestinationAddress());
    pkt->setDestinationAddressArraySize(1);
    pkt->setDestinationAddress(0,cInfo->getDestinationAddress());

    delete cInfo;

    //set the src address to own mac address (nic module getId())
    pkt->setSrcAddr(address);

    //encapsulate the network packet
    pkt->encapsulate(netwPkt);
    EV_DETAIL << "pkt encapsulated\n";

    return pkt;
}

void AnyMacXmacLayer::sendDataPacket()
{
    nbTxDataPackets++;
    XMacPkt *pkt = macQueue.front()->dup();
    attachSignal(pkt);
    lastDataPktDestAddr = pkt->getDestAddr();
    pkt->setKind(XMAC_DATA);
    sendDown(pkt);
}


void AnyMacXmacLayer::attachSignal(XMacPkt *macPkt)
{
    //calc signal duration
    simtime_t duration = macPkt->getBitLength() / bitrate;
    //create and initialize control info with new signal
    macPkt->setDuration(duration);
}
/**
 * Attaches a "control info" (MacToNetw) structure (object) to the message pMsg.
 */
cObject* AnyMacXmacLayer::setUpControlInfo(cMessage * const pMsg, const MACAddress& pSrcAddr)
{
    SimpleLinkLayerControlInfo *const cCtrlInfo = new SimpleLinkLayerControlInfo();
    cCtrlInfo->setSrc(pSrcAddr);
    cCtrlInfo->setInterfaceId(interfaceEntry->getInterfaceId());
    pMsg->setControlInfo(cCtrlInfo);
    return cCtrlInfo;
}


/**
 * Change the color of the node for animation purposes.
 */

void AnyMacXmacLayer::changeDisplayColor(XMAC_COLORS color)
{
	if (!animation)
		return;
	cDisplayString& dispStr = findContainingNode(this)->getDisplayString();
	//b=40,40,rect,black,black,2"
	if (color == GREEN)
		dispStr.setTagArg("b", 3, "green");
		//dispStr.parse("b=40,40,rect,green,green,2");
	if (color == BLUE)
		dispStr.setTagArg("b", 3, "blue");
				//dispStr.parse("b=40,40,rect,blue,blue,2");
	if (color == RED)
		dispStr.setTagArg("b", 3, "red");
				//dispStr.parse("b=40,40,rect,red,red,2");
	if (color == BLACK)
		dispStr.setTagArg("b", 3, "black");
				//dispStr.parse("b=40,40,rect,black,black,2");
	if (color == YELLOW)
		dispStr.setTagArg("b", 3, "yellow");
				//dispStr.parse("b=40,40,rect,yellow,yellow,2");
}

} // namespace inet
