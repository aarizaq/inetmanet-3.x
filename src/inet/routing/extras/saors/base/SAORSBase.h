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

#ifndef _SAORSBASE_H_
#define _SAORSBASE_H_

#include "inet/common/INETDefs.h"
#include "inet/routing/extras/dymo_fau/DYMOFau.h"
#include "SAORS_RREQ_m.h"
#include "SAORS_RREP_m.h"
#include "SAORS_BEACON_m.h"
#include "DT_MSG_m.h"
#include "SAORSBase_RoutingTable.h"
#include "SAORSBase_OutstandingRREQList.h"
#include "SAORSBase_DataQueue.h"
#include "SAORSBase_TokenBucket.h"


#define DYMO_PORT 653

namespace inet {

namespace inetmanet {


namespace SAORSns {
	const int DYMO_RM_HEADER_LENGTH = 13; 			///< length (in bytes) of a DYMO RM header
	const int DYMO_RBLOCK_LENGTH = 10; 				///< length (in bytes) of one DYMO RBlock
	const int SAORS_BEACON_ENTRY_LENGTH = 6; 		///< length (in bytes) of one DYMO RBlock
	const int DYMO_RERR_HEADER_LENGTH = 4; 			///< length (in bytes) of a DYMO RERR header
	const int DYMO_UBLOCK_LENGTH = 8; 				///< length (in bytes) of one DYMO UBlock
	const int UDPPort = DYMO_PORT; 					///< UDP Port to listen on (TBD)
	const double MAXJITTER = 0.001; 				///< all messages sent to a lower layer are delayed by 0..MAXJITTER seconds (draft-ietf-manet-jitter-01)
	const double MINDELPROB = 0.2; 					///< The minimum carrier probability for carrier selection
	const double HANDOVER_THESHOLD = 1.1;           ///< Defines the difference between two metric that indicates a better carrier
}


/********************************************************************************************
 * class DYMO: Implements the network layer to route incoming messages
 ********************************************************************************************/
class SAORSBase : public DYMOFau {
	public:
		int numInitStages() const {return 5;}
		void initialize(int);
		void finish();
		~SAORSBase();

		/** @brief function called by whenever a message arrives at the module */
		void handleMessage(cMessage * msg);

		/** @brief returns the SAORS routing table of the module */
		virtual SAORSBase_RoutingTable* getDYMORoutingTable();

		/** @brief guesses which router the given address belongs to, might return 0 */
		virtual cModule* getRouterByAddress(IPv4Address address);

	protected:
		friend class SAORSBase_RoutingTable;
		//=======================================================================//
		//                            OPERATIONS                                 //
		//=======================================================================//
		/** @brief processes the packet */
		void processPacket (const IPv4Datagram* datagram);

		/** @brief handle self messages such as timer... */
		virtual void handleSelfMsg(cMessage*);

		/** @brief return destination interface for reply messages */
		virtual InterfaceEntry* getNextHopInterface(cPacket* pkt);

		/** @brief return destination address for reply messages */
		virtual uint32_t getNextHopAddress(DYMO_RM *routingMsg);

		/** @brief function sends messages to lower layer (transport layer) */
		virtual void sendDown(cPacket*, int);

		/** @brief updates the lifetime (validTimeout) of a route */
		virtual void updateRouteLifetimes(unsigned int destAddr);

		/** @brief generates and sends down a new rreq for given destination address */
		virtual void sendRREQ(unsigned int destAddr, int msgHdrHopLimit, unsigned int targetSeqNum, unsigned int targetHopCnt, double min_prob);

		/** @brief creates and sends a RREP to the given destination */
		virtual void sendReply(unsigned int destAddr, unsigned int tSeqNum);

		/** @brief creates and sends a RREP when this node is an intermediate router */
		virtual void sendReplyAsIntermediateRouter(const DYMO_AddressBlock& origNode, const DYMO_AddressBlock& targetNode, const SAORSBase_RoutingEntry* routeToTarget);

		/** @brief generates and sends a RERR message */
		virtual void sendRERR(unsigned int targetAddress, unsigned int targetSeqNum);

		/** @brief checks whether an AddressBlock's information is better than the current one or is stale/disregarded. Behaviour according to draft-ietf-manet-dymo-09, section 5.2.1 ("Judging New Routing Information's Usefulness") */
		virtual bool isRBlockBetter(SAORSBase_RoutingEntry * entry, DYMO_AddressBlock rblock, bool isRREQ);

		/** @brief function verifies the send tries for a given RREQ and send a new request if RREQ_TRIES is not reached, deletes control info from the queue if RREQ_TRIES is reached  and send an ICMP message to upper layer */
		virtual void handleRREQTimeout(SAORSBase_OutstandingRREQ& outstandingRREQ);

		/** @brief updates routing entries from AddressBlock, returns whether AddressBlock should be kept */
		virtual bool updateRoutesFromAddressBlock(const DYMO_AddressBlock& ab, bool isRREQ, uint32_t nextHopAddress, InterfaceEntry* nextHopInterface);

		/** @brief function updates the routing entries from received AdditionalNodes. @see draft 4.2.1 */
		virtual DYMO_RM* updateRoutes(DYMO_RM * pkt);

		/** @brief dequeue packets to destinationAddress */
		virtual void checkAndSendQueuedPkts(unsigned int destinationAddress, int prefix, unsigned int nextHopAddress);

		/** @brief selects the best reply sent to the outstandingDTRREQ list and forwards the SAORS packets */
		virtual void carrierSelection(SAORSBase_OutstandingRREQ& outstandingRREQ);

		/** @brief called for packets whose delivery fails at the link layer */
		virtual void processLinkBreak(const cObject *details);

		/** @brief called by processLinkBreak to inform on the packets failed to be delivered */
		virtual void packetFailed(IPv4Datagram *dgram);

		//=============================================//
        //          Lower Messages Handlers            //
        //=============================================//
        /** @brief Function handles lower messages: sends messages sent to the node to higher layer, forward messages to destination nodes... */
        virtual void handleLowerMsg(cPacket*);

        /** @brief Check if the routing message received in for this node or another one, and choose the appropriate action */
        virtual void handleLowerRM(DYMO_RM*);

        /** @brief handle messages that are destined for this node  */
        virtual void handleLowerRMForMe(DYMO_RM *routingMsg);

        /** @brief handle messages that should be processed and transmitted to other destinations */
        virtual void handleLowerRMForRelay(DYMO_RM *routingMsg);

        /** @bried handle the reception of received error messages */
        virtual void handleLowerRERR(DYMO_RERR*);

        /** @brief handle the reception of unsupported error messages */
        virtual void handleLowerUERR(DYMO_UERR*);
        //=============================================//

        //=============================================//
        //         SAORS Extension Functions           //
        //=============================================//
        /** @brief generates and sends a beacon to the surrounding nodes */
        virtual void sendBeacon() = 0;

        /** @brief performs the update of the routing entries, according to the beacon received */
        virtual void handleBeacon(SAORS_BEACON* my_beacon) = 0;

        /** @brief returns the probability of encountering the node indicating by the SAORS Routing Entry */
        virtual double findEncounterProb(const SAORSBase_RoutingEntry* routeToNode) = 0;

        /** @brief returns whether the RREP has a higher or lower  encounter probability */
        virtual bool compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep) = 0;

        /** @brief determines whether the node will reply as an intermediate router, to inform of it's encounter probability*/
        virtual bool sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry) = 0;
        //=============================================//

		//=======================================================================//
		//                          MEMBER VARIABLES                             //
		//=======================================================================//
		SAORSBase_RoutingTable* dymo_routingTable;          ///< Pointer to the routing table */

		SAORSBase_DataQueue* dtqueuedDataPackets;           ///< Vector contains the queued data packets, waiting for a route

		SAORSBase_OutstandingRREQList outstandingRREQList;  ///< Vector contains the RREQs sent, waiting for a reply

		DYMO_Timer* beaconTimeout;                          ///< Runs after the Beacon timeout expired and sends a new beacon

		SAORSBase_TokenBucket* rateLimiterRREQ;             ///< Performs rate limiting in RREQ transmissions

		/** @brief SAORS routing table entry limits */
		int DTRT_DELETE_TIMEOUT;
		int BEACON_TIMEOUT;
		int DLT_OUTSTANDINGRREQ_TIMEOUT;
		uint8_t NUM_COPIES;     /**< NED configuration parameter: the initial amount of copies that can be made from a DT Packet */
        uint8_t SEND_COPIES;    /**< NED configuration parameter: the number of copies to be sent to the other nodes */
        uint8_t SEND_COPIES_PC; /**< NED configuration parameter: the percentage of the copies left to be sent to the other nodes */
        bool EPIDEMIC;          /**< NED configuration parameter: defines whether the copy spreading will take place epidemically */

		/** @brief Statistics */
		int beaconsReceived;
		int DTMesgRcvd;
		double netDensity;

		/** @brief records the received DT Messages of the node during the simulation */
        simsignal_t RcvdDTMesgs;
};

} // namespace inetmanet

} // namespace inet


#endif /* _SAORSBASE_H_ */

