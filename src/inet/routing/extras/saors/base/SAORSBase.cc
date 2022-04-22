/*
 *  Copyright (C) 2012 Nikolaos Vastardis
 *  Copyright (C) 2012 University of Essex
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SAORSBase.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/common/ModuleAccess.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"

namespace inet {

namespace inetmanet {

using namespace SAORSns;


/********************************************************************************************
 * The Initialization function of the class.
 ********************************************************************************************/
void SAORSBase::initialize(int stage)
{

	cSimpleModule::initialize(stage);

	if (stage == 4)
	{
		//Initialize DYMO class
		DYMOFau::initialize(stage);

		//Set SAORS time parameters
        DTRT_DELETE_TIMEOUT = par("DTRT_DELETE_TIMEOUT");
        BEACON_TIMEOUT = par("BEACON_TIMEOUT");
        DLT_OUTSTANDINGRREQ_TIMEOUT = par("DLT_OUTSTANDINGRREQ_TIMEOUT");

        //Set up the coping mechanism
        NUM_COPIES = (uint8_t)par("NUM_COPIES");
        SEND_COPIES = (uint8_t)par("SEND_COPIES");
        SEND_COPIES_PC = (uint8_t)par("SEND_COPIES_PC");
        EPIDEMIC = par("EPIDEMIC").boolValue();
        if(SEND_COPIES>NUM_COPIES)
            SEND_COPIES=0;
        if(SEND_COPIES_PC>100 || SEND_COPIES_PC<0)
            SEND_COPIES_PC=0;

        //Register Received DT Messages signal
        RcvdDTMesgs = registerSignal("RcvdDTMsgs");

        //Create a timer for transmitting beacons
		beaconTimeout = new DYMO_Timer(this, "BeaconTimeout");

		//Create the rate limiting structure for RREQs
		rateLimiterRREQ = new SAORSBase_TokenBucket(RREQ_RATE_LIMIT, RREQ_BURST_LIMIT, simTime());

		//Create the routing table of SAORS
		dymo_routingTable = new SAORSBase_RoutingTable(this, IPv4Address(myAddr));

		//Create the queue for storing the delay tolerant messages
		dtqueuedDataPackets = new SAORSBase_DataQueue(this, BUFFER_SIZE_BYTES, NUM_COPIES, SEND_COPIES, SEND_COPIES_PC, EPIDEMIC);

		//Initialize the list for controlling the RREQs generation
		outstandingRREQList.delAll();

		//Watch the specified structures
		WATCH(myAddr);
		WATCH(ownSeqNum);
        WATCH_OBJ(outstandingRREQList);
        WATCH_PTR(beaconTimeout);
        WATCH_PTR(dymo_routingTable);
        WATCH_PTR(dtqueuedDataPackets);
        WATCH_PTR(beaconTimeout);

		//Start scheduling beacons
		beaconTimeout->start(BEACON_TIMEOUT);

		DYMOFau::outstandingRREQList.delAll();
		registerRoutingModule ();
		//setSendToICMP(true);
		linkLayerFeeback();

		//Statistics
		beaconsReceived=0;
		netDensity=0;
		DTMesgRcvd=0;

		//Register Received DT Messages signal
		RcvdDTMesgs = registerSignal("RcvdDTMsgs");

		EV << "Scheduling beacons" << endl;
        rescheduleTimer();
	}
}


/********************************************************************************************
 * The Finish function of the class.
 ********************************************************************************************/
void SAORSBase::finish() {
	recordScalar("totalPacketsSent", totalPacketsSent);
	recordScalar("totalBytesSent", totalBytesSent);

	recordScalar("DYMO_RREQSent", statsRREQSent);
	recordScalar("DYMO_RREPSent", statsRREPSent);
	recordScalar("DYMO_RERRSent", statsRERRSent);

	recordScalar("DYMO_RREQRcvd", statsRREQRcvd);
	recordScalar("DYMO_RREPRcvd", statsRREPRcvd);
	recordScalar("DYMO_RERRRcvd", statsRERRRcvd);

	recordScalar("DYMO_RREQFwd", statsRREQFwd);
	recordScalar("DYMO_RREPFwd", statsRREPFwd);
	recordScalar("DYMO_RERRFwd", statsRERRFwd);

	recordScalar("DYMO_DYMORcvd", statsDYMORcvd);

	recordScalar("NetworkDensity", netDensity);
	recordScalar("DTMesgRcvd", DTMesgRcvd);

	if(discoveryLatency > 0 && disSamples > 0)
		recordScalar("discovery latency", discoveryLatency/disSamples);
	if(dataLatency > 0 && dataSamples > 0)
		recordScalar("data latency", dataLatency/dataSamples);

	delete dymo_routingTable;
	dymo_routingTable = 0;

	outstandingRREQList.delAll();
	DYMOFau::outstandingRREQList.delAll();

	delete ownSeqNumLossTimeout;
	ownSeqNumLossTimeout = 0;
	delete ownSeqNumLossTimeoutMax;
	ownSeqNumLossTimeoutMax = 0;
	delete beaconTimeout;
	beaconTimeout = 0;

	//delete rateLimiterRREQ;
	rateLimiterRREQ = 0;

	//IP* ipLayer = queuedDataPackets->getIpLayer();
	delete queuedDataPackets;
	queuedDataPackets = 0;

	delete dtqueuedDataPackets;
	dtqueuedDataPackets = 0;
	//ipLayer->unregisterHook(0, this);
}


/********************************************************************************************
 * Class Constructor
 ********************************************************************************************/
SAORSBase::~SAORSBase()
{
	if (dymo_routingTable)
		delete dymo_routingTable;

	//Problem in rebuilding
	outstandingRREQList.delAll();

	if (dtqueuedDataPackets)
	    delete dtqueuedDataPackets;
}


/********************************************************************************************
 * Function called by whenever a message arrives at the module.
 ********************************************************************************************/
void SAORSBase::handleMessage(cMessage* apMsg)
{

	cMessage * msg_aux=NULL;
	UDPPacket* udpPacket = NULL;
	DT_MSG* dtmsg = NULL;

	if (apMsg->isSelfMessage()) {
		handleSelfMsg(apMsg);
	}
	else {

		if ( dynamic_cast<ControlManetRouting *>(apMsg) )
		{
			ControlManetRouting * control =  check_and_cast <ControlManetRouting *> (apMsg);
			if (control->getOptionCode()== MANET_ROUTE_NOROUTE)
			{
				IPv4Datagram * dgram = check_and_cast<IPv4Datagram*>(control->decapsulate()) ;
				processPacket(dgram);
			}
			else if (control->getOptionCode()== MANET_ROUTE_UPDATE)
			{
				updateRouteLifetimes(control->getSrcAddress().toIPv4().getInt());
				updateRouteLifetimes(control->getDestAddress().toIPv4().getInt());
			}
			delete apMsg;
			return;
		}
		else if (dynamic_cast<UDPPacket *>(apMsg))
		{

			udpPacket = check_and_cast<UDPPacket*>(apMsg);
			if (udpPacket->getDestinationPort()!= DYMO_PORT)
			{
				delete  apMsg;
				return;
			}
			msg_aux  = udpPacket->decapsulate();

			IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(udpPacket->removeControlInfo());
			if (isLocalAddress(L3Address(controlInfo->getSrcAddr())) || controlInfo->getSrcAddr().isUnspecified())
			{
				//Local address delete packet
				delete msg_aux;
				delete controlInfo;
				return;
			}
			msg_aux->setControlInfo(controlInfo);
		}
		else if (dynamic_cast<DT_MSG *>(apMsg)) {

			//Record the delay Tolerant Message received
			dtmsg = check_and_cast<DT_MSG*>(apMsg);
			if(dtqueuedDataPackets->queuePacket(dtmsg,32)==0) {
				//Record as vector
				DTMesgRcvd++;
			}
			return;
		}

		if (udpPacket)
		{
			delete udpPacket;
			udpPacket = NULL;
		}

		if (!dynamic_cast<DYMO_PacketBBMessage  *>(msg_aux))
		{
			delete msg_aux;
			return;
		}
		cPacket* apPkt = PK(msg_aux);
		handleLowerMsg(apPkt);
	}
}


/********************************************************************************************
 * Processes the given IP packet, finding the route or discovering error.
 ********************************************************************************************/
void SAORSBase::processPacket (const IPv4Datagram* datagram)
{
	Enter_Method("procces ip Packet (%s)", datagram->getName());

	IPv4Address destAddr = datagram->getDestAddress();
	int TargetSeqNum=0;
	int TargetHopCount=0;

	//Look up routing table entry for packet destination
	SAORSBase_RoutingEntry* entry = dymo_routingTable->getForAddress(destAddr);
	if (entry) {
		//if a valid route exists, signal the queue to send all packets stored for this destination
		if (!entry->routeBroken) {
			//TODO: mark route as used when forwarding data packets? Draft says yes, but as
			//we are using Route Timeout to "detect" link breaks, this seems to be a bad idea

			//Update routes to destination
			//Send queued packets
			throw cRuntimeError("SAORS has a valid entry route but ip doesn't have a entry route");
			delete datagram;
			return;
		}
		TargetSeqNum=entry->routeSeqNum;
		TargetHopCount=entry->routeDist;
	}

	if (!datagram->getSrcAddress().isUnspecified() && !isIpLocalAddress(datagram->getSrcAddress()))
	{
		//It's not a packet of this node, send error to source
		sendRERR(destAddr.getInt(), TargetSeqNum);
		delete datagram;
		return;
	}

	//No route in the table found -> route discovery (if none is already underway)
	SAORSBase_OutstandingRREQ* outstandingRREQ = outstandingRREQList.getByDestAddr(destAddr.getInt(), 32);
	if (!outstandingRREQ) {
			sendRREQ(destAddr.getInt(), MIN_HOPLIMIT, TargetSeqNum, TargetHopCount, (entry?(findEncounterProb(entry)):0) );
			/** queue the RREQ in a list and schedule a timeout in order to re-send the RREQ when no RREP is received **/
			outstandingRREQ = new SAORSBase_OutstandingRREQ;
			outstandingRREQ->tries = 1;
			outstandingRREQ->wait_time = new DYMO_Timer(this, "RREQWaitTime");
			outstandingRREQ->RREPgather_time = new DYMO_Timer(this, "RREPGatherLimit");
			outstandingRREQ->wait_time->start(RREQ_WAIT_TIME);
			outstandingRREQ->destAddr = destAddr.getInt();
			outstandingRREQ->creationTime = simTime();
			outstandingRREQList.add(outstandingRREQ);
			rescheduleTimer();
	}
	//Else if(outstandingRREQList.getByDestAddr(destAddr.getInt(), 32) &&)
	queuedDataPackets->queuePacket(datagram);
}


/********************************************************************************************
 * Function that handles lower messages: sends messages sent to the node to higher layer,
 * forward messages to destination nodes...
 ********************************************************************************************/
void SAORSBase::handleLowerMsg(cPacket* apMsg) {
	//
	// Check the type of received message
	// 1) Beacon
	// 2) Routing Message: RREQ or RREP
	// 3) Error Message: RERR
	// 4) Unsupported Message: UERR
	// 5) Data Message
	//
	if(dynamic_cast<SAORS_BEACON*>(apMsg)) handleBeacon(dynamic_cast<SAORS_BEACON*>(apMsg));
	else if(dynamic_cast<DYMO_RM*>(apMsg)) handleLowerRM(dynamic_cast<DYMO_RM*>(apMsg));
	else if(dynamic_cast<DYMO_RERR*>(apMsg)) handleLowerRERR(dynamic_cast<DYMO_RERR*>(apMsg));
	else if(dynamic_cast<DYMO_UERR*>(apMsg))  handleLowerUERR(dynamic_cast<DYMO_UERR*>(apMsg));
	else if (apMsg->getKind() == UDP_I_ERROR) { EV << "discarded UDP error message" << endl; }
	else error("message is no SAORS Packet");
}


/********************************************************************************************
 * Check if the routing message received in for this node or another one, and choose the
 * appropriate action.
 ********************************************************************************************/
void SAORSBase::handleLowerRM(DYMO_RM *routingMsg) {
	//Message is a routing message
	EV << "received message is a routing message" << endl;
	statsDYMORcvd++;

	//Routing message  preprocessing and updating routes from routing blocks
	if(updateRoutes(routingMsg) == NULL) {
		EV << "dropping received message" << endl;
		delete routingMsg;
		return;
	}

	//
	// received message is a routing message.
	// check if the node is the destination
	// 1) YES - if the RM is a RREQ, then send a RREP to source
	// 2) NO - send message down to next node.
	//
	if(myAddr == routingMsg->getTargetNode().getAddress()) {
		handleLowerRMForMe(routingMsg);
		return;
	} else {
		handleLowerRMForRelay(routingMsg);
		return;
	}
}


/********************************************************************************************
 * Return destination interface for reply messages.
 ********************************************************************************************/
uint32_t SAORSBase::getNextHopAddress(DYMO_RM *routingMsg) {
	if (routingMsg->getAdditionalNodes().size() > 0) {
		return routingMsg->getAdditionalNodes().back().getAddress();
	} else {
		return routingMsg->getOrigNode().getAddress();
	}
}


/********************************************************************************************
 * Return destination interface for reply messages.
 ********************************************************************************************/
InterfaceEntry* SAORSBase::getNextHopInterface(cPacket* pkt) {

	if (!pkt) error("getNextHopInterface called with NULL packet");

	IPv4ControlInfo* controlInfo = check_and_cast<IPv4ControlInfo*>(pkt->removeControlInfo());
	if (!controlInfo) error("received packet did not have IPv4ControlInfo attached");

	int interfaceId = controlInfo->getInterfaceId();
	if (interfaceId == -1) error("received packet's UDPControlInfo did not have information on interfaceId");


	InterfaceEntry* srcIf = NULL;

	for (int i = 0; i <getNumWlanInterfaces(); i++)
	{
		InterfaceEntry *ie = getWlanInterfaceEntry(i);
		if (interfaceId ==ie->getInterfaceId())
		{
			srcIf = ie;
			break;
		}
	}

	if (!srcIf) error("parent module interface table did not contain interface on which packet arrived");

	if (controlInfo) delete controlInfo;
	return srcIf;
}


/********************************************************************************************
 * Handle messages that are destined for this node.
 ********************************************************************************************/
void SAORSBase::handleLowerRMForMe(DYMO_RM *routingMsg) {
	//Current node is the target
	if(dynamic_cast<SAORS_RREQ*>(routingMsg)) {
		//Received message is a RREQ -> send a RREP to source
		sendReply(routingMsg->getOrigNode().getAddress(), (routingMsg->getTargetNode().hasSeqNum() ? routingMsg->getTargetNode().getSeqNum() : 0));
		statsRREQRcvd++;
		delete routingMsg;
	}
	else if(dynamic_cast<SAORS_RREP*>(routingMsg)) {
		//Received message is a RREP
		statsRREPRcvd++;

		//Addition for SAORS: if the Searched Node field is empty, there is a connected path
		if( !(dynamic_cast<SAORS_RREP*>(routingMsg)->getSearchedNode().hasAddress()) ) {
			//signal the queue to dequeue waiting messages for this destination
			checkAndSendQueuedPkts(routingMsg->getOrigNode().getAddress(), (routingMsg->getOrigNode().hasPrefix() ? routingMsg->getOrigNode().getPrefix() : 32), getNextHopAddress(routingMsg));

			delete routingMsg;
		}
		else {
			//Check if is there is a record in the outstandingRREQList
			SAORSBase_OutstandingRREQ* o = outstandingRREQList.getByDestAddr(dynamic_cast<SAORS_RREP*>(routingMsg)->getSearchedNode().getAddress(), (dynamic_cast<SAORS_RREP*>(routingMsg)->getSearchedNode().hasPrefix() ? dynamic_cast<SAORS_RREP*>(routingMsg)->getSearchedNode().getPrefix() : 32) );

			if(o) {
				//Put RREP the carrier selection list
				o->CS_List.addRREPtoList(dynamic_cast<SAORS_RREP*>(routingMsg));

				//Stop timer from SAORSBase_OutstandingRREQList
				o->wait_time->cancel();

				//If RREP gather timer is not running, start it
				if( o->RREPgather_time->stopWhenExpired() )
					o->RREPgather_time->start(DLT_OUTSTANDINGRREQ_TIMEOUT);
			}
			else {
				EV << "SAORSBase_OutstandingRREQ not found, dropping message" << endl;
				delete routingMsg;
			}
		}
	}
	else error("received unknown SAORS message");
}


/********************************************************************************************
 * Handle messages that should be processed and transmitted to other destinations.
 ********************************************************************************************/
void SAORSBase::handleLowerRMForRelay(DYMO_RM *routingMsg) {

	//Current node is not the message destination -> find route to destination
	EV << "current node is not the message destination -> find route to destination " <<  routingMsg->getTargetNode().getAddress() << endl;

	unsigned int targetAddr = routingMsg->getTargetNode().getAddress();
	unsigned int targetSeqNum = 0;

	//Stores route entry of route to destination if a route exists, 0 otherwise
	SAORSBase_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(targetAddr));
	//Trying to find the real next hope address... the actual next hop ---PROBLEM
	if(entry && !entry->routeBroken) {
		while(entry->routeAddress.getInt() != entry->routeNextHopAddress.getInt()) {
			entry = dymo_routingTable->getForAddress(entry->routeNextHopAddress);
			if(!entry)
				break;
			else if(entry->routeBroken)
				break;
		}
	}
	if(entry) {
		targetSeqNum = entry->routeSeqNum;

		//TODO: mark route as used when forwarding DYMO packets?
		//entry->routeUsed.start(ROUTE_USED_TIMEOUT);
		//entry->routeDelete.cancel();

		if(entry->routeBroken) entry = 0;
	}

	//Received routing message is an RREP and no routing entry was found
	if(dynamic_cast<SAORS_RREP*>(routingMsg) && (!entry)) {
		/* do nothing, just drop the RREP */
		EV << "no route to destination of RREP was found. Sending RERR and dropping message." << endl;
		sendRERR(targetAddr, targetSeqNum);
		delete routingMsg;
		return;
	}

	//Check if received message is a RREQ and a routing entry was found
	if (dynamic_cast<SAORS_RREQ*>(routingMsg) && (entry) && (routingMsg->getTargetNode().hasSeqNum()) && (!seqNumIsFresher(routingMsg->getTargetNode().getSeqNum(), entry->routeSeqNum))) {
		//yes, we can. Do intermediate DYMO router RREP creation
		EV << "route to destination of RREQ was found. Sending intermediate SAORS router RREP" << endl;
		sendReplyAsIntermediateRouter(routingMsg->getOrigNode(), routingMsg->getTargetNode(), entry);
		statsRREQRcvd++;
		delete routingMsg;
		return;
	}

	//Addition for SAORS: if probability of encountering requested node is higher than stated in the RREQ
	SAORSBase_RoutingEntry* dtentry = dymo_routingTable->getDTByAddress(IPv4Address(targetAddr));
	if( dynamic_cast<SAORS_RREQ*>(routingMsg) && (!entry) && (dtentry) ) {
		//the requested node has been encountered in the past but no present contact
		//if the probability of encountering this node is higher than the sender
		//if( dynamic_cast<SAORS_RREQ*>(routingMsg)->getMinDeliveryProb() < (dtentry->deliveryProb*HANDOVER_THESHOLD) ) {
		if( sendEncounterProb( dynamic_cast<SAORS_RREQ*>(routingMsg), dtentry) ) {
			EV << "Possibility of encountering destination is higher in this node. Sending intermediate SAORS router RREP" << endl;
			sendReplyAsIntermediateRouter(routingMsg->getOrigNode(), routingMsg->getTargetNode(), dtentry);
			statsRREQRcvd++;
		}
	}

	//Check whether a RREQ was sent to discover route to destination
	EV << "received message is a RREQ or a RREP to be forwarded" << endl;
	EV << "trying to discover route to node " << targetAddr << endl;

	//Increment distance metric of existing AddressBlocks
	std::vector<DYMO_AddressBlock> additional_nodes = routingMsg->getAdditionalNodes();
	std::vector<DYMO_AddressBlock> additional_nodes_to_relay;
	if (routingMsg->getOrigNode().hasDist() && (routingMsg->getOrigNode().getDist() >= 0xFF - 1)) {
		EV << "passing on this message would overflow OrigNode.Dist -> dropping message" << endl;
		delete routingMsg;
		return;
	}
	routingMsg->getOrigNodeForUpdate().incrementDistIfAvailable();
	for (unsigned int i = 0; i < additional_nodes.size(); i++) {
		if (additional_nodes[i].hasDist() && (additional_nodes[i].getDist() >= 0xFF - 1)) {
			EV << "passing on additionalNode would overflow OrigNode.Dist -> dropping additionalNode" << endl;
			continue;
		}
		additional_nodes[i].incrementDistIfAvailable();
		additional_nodes_to_relay.push_back(additional_nodes[i]);
	}

	//Append additional routing information about this node
	DYMO_AddressBlock additional_node;
	additional_node.setDist(0);
	additional_node.setAddress(myAddr);
	if (RESPONSIBLE_ADDRESSES_PREFIX != -1) additional_node.setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
	incSeqNum();
	EV << "Setting sequence to additional node: " << ownSeqNum << endl;
	additional_node.setSeqNum(ownSeqNum);
	additional_nodes_to_relay.push_back(additional_node);

	routingMsg->setAdditionalNodes(additional_nodes_to_relay);
	routingMsg->setMsgHdrHopLimit(routingMsg->getMsgHdrHopLimit() - 1);

	//Check hop limit
	if (routingMsg->getMsgHdrHopLimit() < 1) {
		EV << "received message has reached hop limit -> delete message" << endl;
		delete routingMsg;
		return;
	}

	//Do not transmit DYMO messages when we lost our sequence number
	if (ownSeqNumLossTimeout->stopWhenExpired()) {
		EV << "node has lost sequence number -> not transmitting anything" << endl;
		delete routingMsg;
		return;
	}

	//Do rate limiting
	if ((dynamic_cast<SAORS_RREQ*>(routingMsg)) && (!rateLimiterRREQ->consumeTokens(1, simTime()))) {
		EV << "RREQ send rate exceeded maximum -> not transmitting RREQ" << endl;
		delete routingMsg;
		return;
	}

	if (dynamic_cast<SAORS_RREP*>(routingMsg))
		EV << "Next hop address is " << entry->routeNextHopAddress << endl;

	//Transmit message -- RREP via unicast, RREQ via DYMOcast
	sendDown(routingMsg, dynamic_cast<SAORS_RREP*>(routingMsg) ? (entry->routeNextHopAddress).getInt() : IPv4Address::LL_MANET_ROUTERS.getInt());

	//Keep statistics
	if (dynamic_cast<SAORS_RREP*>(routingMsg)) {
		statsRREPFwd++;
	} else {
		statsRREQFwd++;
	}
}


/********************************************************************************************
 * Handle the reception of received error messages. This could mean the deletion of the
 * related routing entries.
 ********************************************************************************************/
void SAORSBase::handleLowerRERR(DYMO_RERR *my_rerr) {
	//Message is a RERR
	statsDYMORcvd++;

	//Get RERR's IP.SourceAddress
	IPv4ControlInfo* controlInfo = check_and_cast<IPv4ControlInfo*>(my_rerr->getControlInfo());
	IPv4Address sourceAddr = controlInfo->getSrcAddr();

	//Get RERR's SourceInterface
	InterfaceEntry* sourceInterface = getNextHopInterface(my_rerr);

	EV << "Received RERR from " << sourceAddr << endl;

	//Iterate over all unreachableNode entries
	std::vector<DYMO_AddressBlock> unreachableNodes = my_rerr->getUnreachableNodes();
	std::vector<DYMO_AddressBlock> unreachableNodesToForward;
	for(unsigned int i = 0; i < unreachableNodes.size(); i++) {
		const DYMO_AddressBlock& unreachableNode = unreachableNodes[i];

		if (IPv4Address(unreachableNode.getAddress()).isMulticast()) continue;

		//Check whether this invalidates entries in our routing table
		std::vector<SAORSBase_RoutingEntry *> RouteVector = dymo_routingTable->getRoutingTable();
		for(unsigned int i = 0; i < RouteVector.size(); i++) {
			SAORSBase_RoutingEntry* entry = RouteVector[i];

			//Skip if route has no associated Forwarding Route
			if (entry->routeBroken) continue;

			//Skip if this route isn't to the unreachableNode Address mentioned in the RERR
			if (!entry->routeAddress.prefixMatches(IPv4Address(unreachableNode.getAddress()), entry->routePrefix)) continue;

			//Skip if route entry isn't via the RERR sender
			if (entry->routeNextHopAddress != sourceAddr) continue;
			if (entry->routeNextHopInterface != sourceInterface) continue;

			//Skip if route entry is fresher
			if (!((entry->routeSeqNum == 0) || (!unreachableNode.hasSeqNum()) || (!seqNumIsFresher(entry->routeSeqNum, unreachableNode.getSeqNum())))) continue;

			EV << "RERR invalidates route to " << entry->routeAddress << " via " << entry->routeNextHopAddress << endl;

			//Mark as broken and delete associated forwarding route
			entry->routeBroken = true;
			dymo_routingTable->maintainAssociatedRoutingTable();

			//Start delete timer
			//TODO: not specified in draft, but seems to make sense
			entry->routeDelete.start(ROUTE_DELETE_TIMEOUT);

			//Update unreachableNode.SeqNum
			//TODO: not specified in draft, but seems to make sense
			DYMO_AddressBlock unreachableNodeToForward;
			unreachableNodeToForward.setAddress(unreachableNode.getAddress());
			if (unreachableNode.hasSeqNum()) unreachableNodeToForward.setSeqNum(unreachableNode.getSeqNum());
			if (entry->routeSeqNum != 0) unreachableNodeToForward.setSeqNum(entry->routeSeqNum);

			//Forward this unreachableNode entry
			unreachableNodesToForward.push_back(unreachableNodeToForward);
		}
	}

	//Discard RERR if there are no entries to forward
	if (unreachableNodesToForward.size() <= 0) {
		statsRERRRcvd++;
		delete my_rerr;
		return;
	}

	//Discard RERR if ownSeqNum was lost
	if (ownSeqNumLossTimeout->stopWhenExpired()) {
		statsRERRRcvd++;
		delete my_rerr;
		return;
	}

	//Discard RERR if msgHdrHopLimit has reached 1
	if (my_rerr->getMsgHdrHopLimit() <= 1) {
		statsRERRRcvd++;
		delete my_rerr;
		return;
	}

	//Forward RERR with unreachableNodesToForward
	my_rerr->setUnreachableNodes(unreachableNodesToForward);
	my_rerr->setMsgHdrHopLimit(my_rerr->getMsgHdrHopLimit() - 1);

	EV << "send down RERR" << endl;
	sendDown(my_rerr, IPv4Address::LL_MANET_ROUTERS.getInt());

	statsRERRFwd++;
}


/********************************************************************************************
 * Handle the reception of unsupported error messages
 ********************************************************************************************/
void SAORSBase::handleLowerUERR(DYMO_UERR *my_uerr) {
	//Message is a UERR
	statsDYMORcvd++;

	EV << "Received unsupported UERR message" << endl;
	//To be finished
	delete my_uerr;
}


/********************************************************************************************
 * Handle self messages such as timer...
 ********************************************************************************************/
void SAORSBase::handleSelfMsg(cMessage* apMsg) {
	EV << "handle self message" << endl;
	if(apMsg == timerMsg) {
	    bool hasActive = false;

		//Something timed out. Let's find out what.

		//Read the Message name to learn were it came from
		std::string apMsgName(apMsg->getName());

		//Maybe it's a ownSeqNumLossTimeout
		if ( ownSeqNumLossTimeout->stopWhenExpired() || ownSeqNumLossTimeoutMax->stopWhenExpired() ) {
			ownSeqNumLossTimeout->cancel();
			ownSeqNumLossTimeoutMax->cancel();
			ownSeqNum = 1;
		}
		hasActive = ownSeqNumLossTimeout->isActive() || ownSeqNumLossTimeoutMax->isActive();

		//Maybe it's a outstanding RREQ
		SAORSBase_OutstandingRREQ* outstandingRREQ;
        while ((outstandingRREQ = outstandingRREQList.getExpired()) != NULL )
            handleRREQTimeout(*outstandingRREQ);

		//Maybe it's time to check the multiple replies gathered from outstanding RREQ
		SAORSBase_OutstandingRREQ* outstandingDTRREQ;
		while ((outstandingDTRREQ = outstandingRREQList.getRREPGTExpired()) != NULL )
		    carrierSelection(*outstandingDTRREQ);   //TODO: Send packets to intermediate node for DT routing

		if (!hasActive)
            hasActive = outstandingRREQList.hasActive();

		//Maybe it's time for sending a Beacon
		if(beaconTimeout->stopWhenExpired()) {

			//Record statistics
			int prevBeacons = simTime().dbl()/BEACON_TIMEOUT;
			netDensity = ( (prevBeacons-1)*netDensity + beaconsReceived )/prevBeacons;

			//Re-initialize statistics vector
			beaconsReceived=0;
			beaconTimeout->cancel();

			//Record present DT-Messages
			emit(RcvdDTMesgs, DTMesgRcvd);

			//Send out beacon
			sendBeacon();

			//Send bext beacon with a small jitter
			simtime_t jitter = dblrand() * MAXJITTER;
			beaconTimeout->start(BEACON_TIMEOUT+jitter);
		}

		if (!hasActive)
		    hasActive = beaconTimeout->isActive();

		//Maybe it's a SAORSBase_RoutingEntry
		for(unsigned int i = 0; i < dymo_routingTable->getNumRoutes(); ) {

			SAORSBase_RoutingEntry *entry = dymo_routingTable->getRoute(i);
            bool deleted = false;

            if (entry->routeAgeMax.stopWhenExpired())
            {
                dymo_routingTable->deleteRoute(entry);
                // if other timeouts also expired, they will have gotten their own DYMO_Timeout scheduled, so it's ok to stop here
                deleted = true;
            }
            else
            {
                bool routeNewStopped = entry->routeNew.stopWhenExpired();
                bool routeUsedStopped = entry->routeUsed.stopWhenExpired();

                if ((routeNewStopped || routeUsedStopped) && !(entry->routeUsed.isRunning() || entry->routeNew.isRunning()))
                    entry->routeDelete.start(ROUTE_DELETE_TIMEOUT);

                if (entry->routeDelete.stopWhenExpired())
                {
                    dymo_routingTable->deleteRoute(entry);
                    deleted = true;
                }
            }

            if (!deleted)
            {
                if (!hasActive)
                    hasActive = entry->hasActiveTimer();
                i++;
            }
		}

		//Maybe it's a SAORSBase_Routing delay tolerant Entry
		for(unsigned int i = 0; i < dymo_routingTable->getDTNumRoutes(); ) {

			SAORSBase_RoutingEntry *entry = dymo_routingTable->getDTRoute(i);
			bool deleted = false;

			if(entry->dtrtDelete.stopWhenExpired()) {
				dymo_routingTable->deleteDTRoute(entry);
				deleted = true;
			}

			if (!deleted)
            {
                if (!hasActive)
                    hasActive = entry->hasActiveTimer();
                i++;
            }
		}
		if (hasActive)
            rescheduleTimer();
	}

	else error("unknown message type");
}


/********************************************************************************************
 * Function sends messages to lower layer (transport layer).
 ********************************************************************************************/
void SAORSBase::sendDown(cPacket* apMsg, int destAddr) {

	//All messages sent to a lower layer are delayed by 0..MAXJITTER seconds (draft-ietf-manet-jitter-01)
	simtime_t jitter = dblrand() * MAXJITTER;

	apMsg->setKind(UDP_C_DATA);

	//Set byte size of message
	const DYMO_RM* re = dynamic_cast<const DYMO_RM*>(apMsg);
	const DYMO_RERR* rerr = dynamic_cast<const DYMO_RERR*>(apMsg);
	if (re) {
		apMsg->setByteLength(DYMO_RM_HEADER_LENGTH + ((1 + re->getAdditionalNodes().size()) * DYMO_RBLOCK_LENGTH));

		//Add bytes if its a beacon
		if(const SAORS_BEACON* bcn = dynamic_cast<const SAORS_BEACON*>(re)) {
			int length=apMsg->getByteLength();
			apMsg->setByteLength(length + SAORS_BEACON_ENTRY_LENGTH * bcn->getBeaconEntries().size());
		}
	}
	else if (rerr) {
		apMsg->setByteLength(DYMO_RERR_HEADER_LENGTH + (rerr->getUnreachableNodes().size() * DYMO_UBLOCK_LENGTH));
	}
	else {
		error("Tried to send unsupported message type");
	}
	//Keep statistics
	totalPacketsSent++;
	totalBytesSent+=apMsg->getByteLength();
	 if (IPv4Address::LL_MANET_ROUTERS.getInt()==(unsigned int)destAddr)
    {
        destAddr = IPv4Address::ALLONES_ADDRESS.getInt();
        sendToIp(apMsg, UDPPort, L3Address(IPv4Address(destAddr)), UDPPort, 1, SIMTIME_DBL(jitter));
    }
    else
    {
        sendToIp(apMsg, UDPPort, L3Address(IPv4Address(destAddr)), UDPPort, 1, 0.0);
    }
}


/********************************************************************************************
 * Generates and sends down a new rreq for given destination address.
 ********************************************************************************************/
void SAORSBase::sendRREQ(unsigned int destAddr, int msgHdrHopLimit, unsigned int targetSeqNum, unsigned int targetDist, double min_prob) {
	//Generate a new RREQ with the given pararmeter
	EV << "send a RREQ to discover route to destination node " << destAddr << endl;

	SAORS_RREQ *my_rreq = new SAORS_RREQ("RREQ");
	my_rreq->setMsgHdrHopLimit(msgHdrHopLimit);
	my_rreq->getTargetNodeForUpdate().setAddress(destAddr);
	if (targetSeqNum != 0) my_rreq->getTargetNodeForUpdate().setSeqNum(targetSeqNum);
	if (targetDist != 0) my_rreq->getTargetNodeForUpdate().setDist(targetDist);

	my_rreq->getOrigNodeForUpdate().setDist(0);
	my_rreq->getOrigNodeForUpdate().setAddress(myAddr);
	if (RESPONSIBLE_ADDRESSES_PREFIX != -1) my_rreq->getOrigNodeForUpdate().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
	incSeqNum();
	my_rreq->getOrigNodeForUpdate().setSeqNum(ownSeqNum);
	my_rreq->setMinDeliveryProb(min_prob);

	//Do not transmit DYMO messages when we lost our sequence number
	if (ownSeqNumLossTimeout->stopWhenExpired()) {
		EV << "node has lost sequence number -> not transmitting RREQ" << endl;
		delete my_rreq;
		return;
	}

	//Do rate limiting
	if (!rateLimiterRREQ->consumeTokens(1, simTime())) {
		EV << "RREQ send rate exceeded maximum -> not transmitting RREQ" << endl;
		delete my_rreq;
		//nb->fireChangeNotification()
		return;
	}

	sendDown(my_rreq, IPv4Address::LL_MANET_ROUTERS.getInt());
	statsRREQSent++;
}


/********************************************************************************************
 * Creates and sends a RREP to the given destination
 ********************************************************************************************/
void SAORSBase::sendReply(unsigned int destAddr, unsigned int tSeqNum) {
	//Create a new RREP and send it to given destination
	EV << "send a reply to destination node " << destAddr << endl;

	DYMO_RM* rrep = new SAORS_RREP("RREP");
	SAORSBase_RoutingEntry *entry = dymo_routingTable->getForAddress(IPv4Address(destAddr));
	if (!entry) error("Tried sending RREP via a route that just vanished");

	rrep->setMsgHdrHopLimit(MAX_HOPLIMIT);
	rrep->getTargetNodeForUpdate().setAddress(destAddr);
	rrep->getTargetNodeForUpdate().setSeqNum(entry->routeSeqNum);
	rrep->getTargetNodeForUpdate().setDist(entry->routeDist);

	//Check if ownSeqNum should be incremented
	if ((tSeqNum == 0) || (seqNumIsFresher(ownSeqNum, tSeqNum))) incSeqNum();

	rrep->getOrigNodeForUpdate().setAddress(myAddr);
	if (RESPONSIBLE_ADDRESSES_PREFIX != -1) rrep->getOrigNodeForUpdate().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
	rrep->getOrigNodeForUpdate().setSeqNum(ownSeqNum);
	rrep->getOrigNodeForUpdate().setDist(0);

	//!!! Do not set Searched node parameters !!!

	//Set probability of encounter as certain
	dynamic_cast<SAORS_RREP*>(rrep)->setDeliveryProb(1);

	//Do not transmit DYMO messages when we lost our sequence number
	if (ownSeqNumLossTimeout->stopWhenExpired()) {
		EV << "node has lost sequence number -> not transmitting anything" << endl;
		return;
	}

	sendDown(rrep, (entry->routeNextHopAddress).getInt());

	statsRREPSent++;
}


/********************************************************************************************
 * Creates and sends a RREP when this node is an intermediate router.
 ********************************************************************************************/
void SAORSBase::sendReplyAsIntermediateRouter(const DYMO_AddressBlock& origNode, const DYMO_AddressBlock& targetNode, const SAORSBase_RoutingEntry* routeToTargetNode) {

	bool dt_routing=false;

	//Create a new RREP and send it to given destination
	EV << "sending a reply to OrigNode " << origNode.getAddress() << endl;

	SAORSBase_RoutingEntry* routeToOrigNode = dymo_routingTable->getForAddress(IPv4Address(origNode.getAddress()));
	SAORSBase_RoutingEntry* dtrouteToTargetNode = dymo_routingTable->getDTByAddress(IPv4Address(targetNode.getAddress()));

	if(!routeToOrigNode) error("no route to OrigNode found");

	//Addition for SAORS: If entry only exists in DT routing table set probability
	double encounter_prob=1;
	SAORSBase_RoutingEntry* test = dymo_routingTable->getForAddress(IPv4Address(targetNode.getAddress()));
	//Trying to find the real next hope address... the actual next hop ---PROBLEM
	if(test && !test->routeBroken) {
		while(test->routeAddress.getInt() != test->routeNextHopAddress.getInt()) {
			test = dymo_routingTable->getForAddress(test->routeNextHopAddress);
			if(!test)
				break;
			else if(test->routeBroken)
				break;
		}
	}

	//If the is only a delay-tolerant route to the host, set the probability and flag
	if ( dtrouteToTargetNode && ( (!test) || test->routeBroken ) ) {
		encounter_prob = findEncounterProb(dtrouteToTargetNode);
		dt_routing=true;
	}

	//Increment ownSeqNum.
	//TODO: The draft is unclear about when to increment ownSeqNum for intermediate SAORS router RREP creation
	incSeqNum();

	//Create rrepToOrigNode
	SAORS_RREP* rrepToOrigNode = new SAORS_RREP("RREP");
	rrepToOrigNode->setMsgHdrHopLimit(MAX_HOPLIMIT);
	rrepToOrigNode->getTargetNodeForUpdate().setAddress(origNode.getAddress());
	rrepToOrigNode->getTargetNodeForUpdate().setSeqNum(origNode.getSeqNum());
	if (origNode.hasDist()) rrepToOrigNode->getTargetNodeForUpdate().setDist(origNode.getDist() + 1);

	//If the DT-Routing will NOT be used, then the node sent request is in the proximity of this host
	if(!dt_routing) {
		//Set the RREQ destination as the original node //additionalNode
		rrepToOrigNode->getOrigNodeForUpdate().setAddress(routeToTargetNode->routeAddress.getInt());
		if (RESPONSIBLE_ADDRESSES_PREFIX != -1) rrepToOrigNode->getOrigNodeForUpdate().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
		if (test->routeSeqNum != 0)rrepToOrigNode->getOrigNodeForUpdate().setSeqNum(test->routeSeqNum);
		if (test->routeDist != 0) rrepToOrigNode->getOrigNodeForUpdate().setDist(test->routeDist);
		EV << "Target Sequence number: " << test->routeSeqNum << " and distance: " << test->routeDist;

		//Set me as the additional node to create the correct path //rrepToOrigNode->getOrigNode()
		DYMO_AddressBlock additionalNode;
		additionalNode.setAddress(myAddr);
		additionalNode.setSeqNum(ownSeqNum);
		EV << " Own Sequence number: " << ownSeqNum << endl;
		additionalNode.setDist(0);
		rrepToOrigNode->getAdditionalNodesForUpdate().push_back(additionalNode);
	}
	//Addition for SAORS: If entry only exists in DT routing table set Searched Node, therefore it is not in the proximity of this host
	else {
		//Set me as original address
		rrepToOrigNode->getOrigNodeForUpdate().setAddress(myAddr);
		if (RESPONSIBLE_ADDRESSES_PREFIX != -1) rrepToOrigNode->getOrigNodeForUpdate().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
		rrepToOrigNode->getOrigNodeForUpdate().setSeqNum(ownSeqNum);
		rrepToOrigNode->getOrigNodeForUpdate().setDist(0);

		//Set searched node
		rrepToOrigNode->getSearchedNodeForUpdate().setAddress(targetNode.getAddress());
		if (targetNode.hasPrefix()) rrepToOrigNode->getSearchedNodeForUpdate().setPrefix(targetNode.getPrefix());
		//The target node has to have a sequence number, so if non available set the minimum
		if (targetNode.hasSeqNum()) rrepToOrigNode->getSearchedNodeForUpdate().setSeqNum(targetNode.getSeqNum());
		else rrepToOrigNode->getSearchedNodeForUpdate().setSeqNum(1);
		rrepToOrigNode->getSearchedNodeForUpdate().setDist(0);
	}

	//Setting encounter probability in RREP towards original node
	rrepToOrigNode->setDeliveryProb(encounter_prob);

	//Create rrepToTargetNode
	SAORS_RREP* rrepToTargetNode = new SAORS_RREP("RREP");
	rrepToTargetNode->setMsgHdrHopLimit(MAX_HOPLIMIT);
	rrepToTargetNode->getTargetNodeForUpdate().setAddress(targetNode.getAddress());
	if (targetNode.hasSeqNum()) rrepToTargetNode->getTargetNodeForUpdate().setSeqNum(targetNode.getSeqNum());
	if (targetNode.hasDist()) rrepToTargetNode->getTargetNodeForUpdate().setDist(targetNode.getDist());

	rrepToTargetNode->getOrigNodeForUpdate().setAddress(myAddr);
	if (RESPONSIBLE_ADDRESSES_PREFIX != -1) rrepToTargetNode->getOrigNodeForUpdate().setPrefix(RESPONSIBLE_ADDRESSES_PREFIX);
	rrepToTargetNode->getOrigNodeForUpdate().setSeqNum(ownSeqNum);
	rrepToTargetNode->getOrigNodeForUpdate().setDist(0);

	DYMO_AddressBlock additionalNode2;
	additionalNode2.setAddress(origNode.getAddress());
	additionalNode2.setSeqNum(origNode.getSeqNum());
	if (origNode.hasDist()) additionalNode2.setDist(origNode.getDist() + 1);
	rrepToTargetNode->getAdditionalNodesForUpdate().push_back(additionalNode2);

	//Setting encounter probability in RREP towards target node
	rrepToTargetNode->setDeliveryProb(encounter_prob);

	//Do not transmit DYMO messages when we lost our sequence number
	if (ownSeqNumLossTimeout->stopWhenExpired()) {
		EV << "node has lost sequence number -> not transmitting anything" << endl;
		return;
	}

	sendDown(rrepToOrigNode, ( routeToOrigNode->routeNextHopAddress).getInt() );
	if(!dt_routing) sendDown( rrepToTargetNode, ( routeToTargetNode->routeNextHopAddress).getInt() );
	else delete rrepToTargetNode;

	statsRREPSent++;
}


/********************************************************************************************
 * Generates and sends a RERR message.
 ********************************************************************************************/
void SAORSBase::sendRERR(unsigned int targetAddr, unsigned int targetSeqNum) {

	DYMO_RERR *rerr = new DYMO_RERR("RERR");
	std::vector<DYMO_AddressBlock> unode_vec;

	//Add target node as first unreachableNode
	DYMO_AddressBlock unode;
	unode.setAddress(targetAddr);
	if (targetSeqNum != 0) unode.setSeqNum(targetSeqNum);
	unode_vec.push_back(unode);

	//Set hop limit
	rerr->setMsgHdrHopLimit(MAX_HOPLIMIT);

	//add additional unreachableNode entries for all route entries that use the same routeNextHopAddress and routeNextHopInterface
	SAORSBase_RoutingEntry* brokenEntry = dymo_routingTable->getForAddress(IPv4Address(targetAddr));
	//Trying to find the real next hope address... the actual next hop ---PROBLEM
	if(brokenEntry && !brokenEntry->routeBroken) {
		while(brokenEntry->routeAddress.getInt() != brokenEntry->routeNextHopAddress.getInt()) {
			brokenEntry = dymo_routingTable->getForAddress( brokenEntry->routeNextHopAddress );
			if(!brokenEntry)
				break;
			else if(brokenEntry->routeBroken)
				break;
		}
	}

	if (brokenEntry) {
		//Sanity check
		if (!brokenEntry->routeBroken) throw std::runtime_error("sendRERR called for targetAddr that has a perfectly fine routing table entry");

		//Add route entries with same routeNextHopAddress as broken route
		std::vector<SAORSBase_RoutingEntry *> RouteVector = dymo_routingTable->getRoutingTable();
		for(unsigned int i = 0; i < RouteVector.size(); i++) {
			SAORSBase_RoutingEntry* entry = RouteVector[i];
			if ((entry->routeNextHopAddress != brokenEntry->routeNextHopAddress) || (entry->routeNextHopInterface != brokenEntry->routeNextHopInterface)) continue;

			EV << "Including in RERR route to " << entry->routeAddress << " via " << entry->routeNextHopAddress << endl;

			DYMO_AddressBlock unode;
			unode.setAddress(entry->routeAddress.getInt());
			if (entry->routeSeqNum != 0) unode.setSeqNum(entry->routeSeqNum);
			unode_vec.push_back(unode);
		}
	}

	//Wrap up and send
	rerr->setUnreachableNodes(unode_vec);
	sendDown(rerr, IPv4Address::LL_MANET_ROUTERS.getInt());

	//Keep statistics
	statsRERRSent++;
}


/********************************************************************************************
 * Updates the lifetime (validTimeout) of a route.
 ********************************************************************************************/
void SAORSBase::updateRouteLifetimes(unsigned int targetAddr) {
	SAORSBase_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(targetAddr));
	if (!entry) return;

	//TODO: not specified in draft, but seems to make sense
	if (entry->routeBroken) return;

	entry->routeUsed.start(ROUTE_USED_TIMEOUT);
	entry->routeDelete.cancel();
	rescheduleTimer();

	dymo_routingTable->maintainAssociatedRoutingTable();
	EV << "lifetimes of route to destination node " << targetAddr << " are up to date "  << endl;

	checkAndSendQueuedPkts(entry->routeAddress.getInt(), entry->routePrefix, (entry->routeNextHopAddress).getInt());
}


/********************************************************************************************
 * Checks whether an AddressBlock's information is better than the current one or is
 * stale/disregarded. Behaviour according to draft-ietf-manet-dymo-09, section 5.2.1
 * ("Judging New Routing Information's Usefulness").
 ********************************************************************************************/
bool SAORSBase::isRBlockBetter(SAORSBase_RoutingEntry * entry, DYMO_AddressBlock ab, bool isRREQ) {
	//TODO: check handling of unknown SeqNum values

	//Stale?
	if (seqNumIsFresher(entry->routeSeqNum, ab.getSeqNum())) return false;

	//Loop-possible or inferior?
	if (ab.getSeqNum() == (int)entry->routeSeqNum) {

		int nodeDist = ab.hasDist() ? (ab.getDist() + 1) : 0; //incremented by one, because draft -10 says to first increment, then compare
		int routeDist = entry->routeDist;

		//Loop-possible?
		if (nodeDist == 0) return false;
		if (routeDist == 0) return false;
		if (nodeDist > routeDist + 1) return false;
		EV << "Route Distances are " << routeDist << " against the new " << nodeDist << endl;

		//Inferior?
		if ( (nodeDist > routeDist) && (!entry->routeBroken) ) return false;
		if ( (nodeDist == routeDist) && (!entry->routeBroken) && (isRREQ) ) return false;
	}

	//Superior
	return true;
}


/********************************************************************************************
 * Function verifies the send tries for a given RREQ and send a new request if RREQ_TRIES is
 * not reached, deletes control info from the queue if RREQ_TRIES is reached  and send an
 * ICMP message to upper layer.
 ********************************************************************************************/
void SAORSBase::handleRREQTimeout(SAORSBase_OutstandingRREQ& outstandingRREQ) {
	EV << "Handling RREQ Timeouts for RREQ to " << outstandingRREQ.destAddr << endl;

	if(outstandingRREQ.tries < RREQ_TRIES) {
		SAORSBase_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(outstandingRREQ.destAddr));
		if(entry && (!entry->routeBroken)) {
			//An entry was found in the routing table -> get control data from the table, encapsulate message
			EV << "RREQ timed out and we DO have a route" << endl;

			checkAndSendQueuedPkts(entry->routeAddress.getInt(), entry->routePrefix, entry->routeNextHopAddress.getInt());

			return;
		}
		else {
			EV << "RREQ timed out and we do not have a route yet" << endl;
			//Number of tries is less than RREQ_TRIES -> backoff and send the rreq
			outstandingRREQ.tries = outstandingRREQ.tries + 1;
			outstandingRREQ.wait_time->start(computeBackoff(outstandingRREQ.wait_time->getInterval()));

			//Update seqNum
			incSeqNum();

			unsigned int targetSeqNum = 0;
			//If a targetSeqNum is known, include it in all but the last RREQ attempt
			if (entry && (outstandingRREQ.tries < RREQ_TRIES)) targetSeqNum = entry->routeSeqNum;

			//Expanding ring search
			int msgHdrHopLimit = MIN_HOPLIMIT + (MAX_HOPLIMIT - MIN_HOPLIMIT) * (outstandingRREQ.tries - 1) / (RREQ_TRIES - 1);

			sendRREQ(outstandingRREQ.destAddr, msgHdrHopLimit, targetSeqNum, (entry?(entry->routeDist):0), (entry?(findEncounterProb(entry)):0) );

			return;
		}
	} else {
		// RREQ_TRIES is reached //

		std::list<IPv4Datagram*> datagrams;
		//drop packets bound for the expired RREQ's destination
		dymo_routingTable->maintainAssociatedRoutingTable();
		queuedDataPackets->dropPacketsTo(IPv4Address(outstandingRREQ.destAddr), 32, &datagrams);
		while (!datagrams.empty())
		{
			IPv4Datagram* dgram = datagrams.front();

			//Count this as a DT Message
			DTMesgRcvd++;

			dtqueuedDataPackets->queueIPPacket(dgram,32,IPv4Address(myAddr));

			datagrams.pop_front();
			//sendICMP(dgram); Don't send error to ICMP
		}

		//clean up outstandingRREQList
		outstandingRREQList.del(&outstandingRREQ);

		return;
	}

	return;
}


/********************************************************************************************
 * Updates routing entries from AddressBlock, returns whether AddressBlock should be kept.
 ********************************************************************************************/
bool SAORSBase::updateRoutesFromAddressBlock(const DYMO_AddressBlock& ab, bool isRREQ, uint32_t nextHopAddress, InterfaceEntry* nextHopInterface) {
	SAORSBase_RoutingEntry* entry = dymo_routingTable->getForAddress(IPv4Address(ab.getAddress()));

	if (entry && !isRBlockBetter(entry, ab, isRREQ)) return false;

	if (!entry) {
		EV << "adding routing entry for " << IPv4Address(ab.getAddress()) << endl;
		entry = new SAORSBase_RoutingEntry(dynamic_cast<SAORSBase*>(this));
		dymo_routingTable->addRoute(entry);
	} else {
		EV << "updating routing entry for " << IPv4Address(ab.getAddress()) << endl;
	}

	entry->routeAddress = IPv4Address(ab.getAddress());
	entry->routeSeqNum = ab.getSeqNum();
	entry->routeDist = ab.hasDist() ? (ab.getDist() + 1) : 0;  //incremented by one, because draft -10 says to first increment, then compare
	entry->routeNextHopAddress = IPv4Address(nextHopAddress);
	entry->routeNextHopInterface = nextHopInterface;
	entry->routePrefix = ab.hasPrefix() ? ab.getPrefix() : 32;
	entry->routeBroken = false;
	entry->routeAgeMin.start(ROUTE_AGE_MIN_TIMEOUT);
	entry->routeAgeMax.start(ROUTE_AGE_MAX_TIMEOUT);
	entry->routeNew.start(ROUTE_NEW_TIMEOUT);
	entry->routeUsed.cancel();
	entry->routeDelete.cancel();
	entry->dtrtDelete.cancel();

	rescheduleTimer();

	dymo_routingTable->maintainAssociatedRoutingTable();

	checkAndSendQueuedPkts(entry->routeAddress.getInt(), entry->routePrefix, nextHopAddress);

	return true;
}


/********************************************************************************************
 * Function updates the routing entries from received AdditionalNodes. @see draft 4.2.1.
 ********************************************************************************************/
DYMO_RM* SAORSBase::updateRoutes(DYMO_RM * pkt) {
	EV << "starting update routes from routing blocks in the received message" << endl;
	std::vector<DYMO_AddressBlock> additional_nodes = pkt->getAdditionalNodes();
	std::vector<DYMO_AddressBlock> new_additional_nodes;

	bool isRREQ = (dynamic_cast<SAORS_RREQ*>(pkt) != 0);
	uint32_t nextHopAddress = getNextHopAddress(pkt);
	InterfaceEntry* nextHopInterface = getNextHopInterface(pkt);

	if(pkt->getOrigNode().getAddress()==myAddr) return NULL;
	EV << "Updating entry for " << pkt->getOrigNode().getAddress() << " with sequence number " <<  pkt->getOrigNode().getSeqNum() << endl;
	bool origNodeEntryWasSuperior = updateRoutesFromAddressBlock(pkt->getOrigNode(), isRREQ, nextHopAddress, nextHopInterface);

	for(unsigned int i = 0; i < additional_nodes.size(); i++) {

		//TODO: not specified in draft, but seems to make sense
		if(additional_nodes[i].getAddress()==myAddr) return NULL;
		EV << "Updating entry for " << additional_nodes[i].getAddress() << " with sequence number " <<  additional_nodes[i].getSeqNum() << endl;
		if (updateRoutesFromAddressBlock(additional_nodes[i], isRREQ, nextHopAddress, nextHopInterface)) {
			/** read routing block is valid -> save block to the routing message **/
			new_additional_nodes.push_back(additional_nodes[i]);
		} else {
			EV << "AdditionalNode AddressBlock has no valid information  -> dropping block from routing message" << endl;
		}
	}

	if (!origNodeEntryWasSuperior) {
		EV << "OrigNode AddressBlock had no valid information -> deleting received routing message" << endl;
		return NULL;
	}

	pkt->setAdditionalNodes(new_additional_nodes);
	return pkt;
}


/********************************************************************************************
 * Dequeue packets to destinationAddress.
 ********************************************************************************************/
void SAORSBase::checkAndSendQueuedPkts(unsigned int destinationAddress, int prefix, unsigned int /*nextHopAddress*/) {
	dymo_routingTable->maintainAssociatedRoutingTable();
	queuedDataPackets->dequeuePacketsTo(IPv4Address(destinationAddress), prefix);

	//clean up outstandingRREQList: remove those with matching destAddr
	SAORSBase_OutstandingRREQ* o = outstandingRREQList.getByDestAddr(destinationAddress, prefix);

	if (o) outstandingRREQList.del(o);
}


/********************************************************************************************
 * Returns the SAORS routing table of the module
 ********************************************************************************************/
SAORSBase_RoutingTable* SAORSBase::getDYMORoutingTable() {
	return dymo_routingTable;
}


/********************************************************************************************
 * Guesses which router the given address belongs to, might return 0
********************************************************************************************/
cModule* SAORSBase::getRouterByAddress(IPv4Address address) {
	return dynamic_cast<cModule*>(getSimulation()->getModule(address.getInt() - AUTOASSIGN_ADDRESS_BASE.getInt()));
}


/********************************************************************************************
 * Called for packets whose delivery fails at the link layer.
 ********************************************************************************************/
void SAORSBase::packetFailed(IPv4Datagram *dgram)
{

	//We don't care about link failures for broadcast or non-data packets
	if (dgram->getDestAddress() == IPv4Address::ALLONES_ADDRESS || dgram->getDestAddress() == IPv4Address::LL_MANET_ROUTERS)
	{
		return;
	}
	EV << "LINK FAILURE for dest=" << dgram->getSrcAddress();
	SAORSBase_RoutingEntry *entry = dymo_routingTable->getByAddress(dgram->getDestAddress());
	if (entry)
	{
		IPv4Address nextHop = entry->routeNextHopAddress;
		for(unsigned int i = 0; i < dymo_routingTable->getNumRoutes(); i++) {
			SAORSBase_RoutingEntry *entry = dymo_routingTable->getRoute(i);
			if (entry->routeNextHopAddress==nextHop)
			{
				entry->routeBroken = true;
				//sendRERR(entry->routeAddress.getInt(),entry->routeSeqNum);
			}
		}
	}
	dymo_routingTable->maintainAssociatedRoutingTable();
}


/********************************************************************************************
 * Called by processLinkBreak to inform on the packets failed to be delivered.
 ********************************************************************************************/
void SAORSBase::processLinkBreak (const cObject *details)
{
	IPv4Datagram	*dgram=NULL;
	if (dynamic_cast<IPv4Datagram *>(const_cast<cObject*> (details)))
		dgram = check_and_cast<IPv4Datagram *>(const_cast<cObject*>(details));
	else
		return;
    packetFailed(dgram);
}


/********************************************************************************************
 * Selects the best reply sent to the outstandingDTRREQ list and forwards the SAORS packets.
 ********************************************************************************************/
void SAORSBase::carrierSelection(SAORSBase_OutstandingRREQ& outstandingRREQ) {
	std::list<IPv4Datagram*> datagrams;
	dymo_routingTable->maintainAssociatedRoutingTable();

	//Move kept Datagrams to the "datagram" variable, to send the to the best carrier
	queuedDataPackets->dropPacketsTo(IPv4Address(outstandingRREQ.destAddr), 32, &datagrams);

	EV << "Selecting best carrier: ";

	while (!datagrams.empty())
	{
		IPv4Datagram* dgram = datagrams.front();

		//Count this as a DT Message
		DTMesgRcvd++;

		dtqueuedDataPackets->queueIPPacket(dgram,32,IPv4Address(myAddr));

		datagrams.pop_front();
	}
	//Acquire RREP from best carrier
	SAORS_RREP* best_rrep = outstandingRREQ.CS_List.findBestCarrier();

	//If no reply found to be adequate
	if(!best_rrep) {
		EV << "ERROR: No best carrier found... Dropping transmission to intermediate node" << endl;
		return;
	}

	EV << "Node with address " << best_rrep->getOrigNode().getAddress() << endl;

	//Find the routing table entry
	SAORSBase_RoutingEntry* entry=dymo_routingTable->getByAddress( IPv4Address(best_rrep->getOrigNode().getAddress()) );
	SAORSBase_RoutingEntry* dtentry=dymo_routingTable->getDTByAddress( IPv4Address(outstandingRREQ.destAddr) );

	if( (!entry) && (!dtentry) )
		error("Tried sending DT messages to intermediate carrier but proper routes not found in the Routing tables");
	else if ( (entry) && (!dtentry) )
		dtentry = entry;

	//Check if probability of encountering the destination is greater in the intermediate node
	if ( compareEncounterProb(dtentry, best_rrep) ) {
		EV << "Dequeuing packet to better carrier: " <<  dtentry->routeAddress << endl;
		int numOfPkts = dtqueuedDataPackets->dequeuePacketsTo( dtentry->routeAddress,  dtentry->routePrefix, entry->routeAddress );

		//Count this as a DT Message
		DTMesgRcvd -= numOfPkts;
	}

	//Delete outstandingRREQ list
	outstandingRREQList.del(&outstandingRREQ);

	return;
}

} // namespace inetmanet

} // namespace inet

