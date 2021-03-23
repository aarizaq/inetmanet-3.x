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

#ifndef SAORSBASE_DATAQUEUE_H
#define SAORSBASE_DATAQUEUE_H

#include <stdio.h>
#include <string.h>
#include <list>
#include <map>
#include <algorithm>
#include <omnetpp.h>
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/networklayer/ipv4/IPv4.h"
#include "DT_MSG_m.h"

namespace inet {

namespace inetmanet {


enum Results {DROP_P, ACCEPT_P, SPREAD_P};
typedef std::map<IPv4Address, std::list<std::string> > carriersMap;
typedef std::map<IPv4Address, std::list<std::string> >::iterator carriersMapIter;


/********************************************************************************************
 * class SAORSBase_QueuedData: A single item of the SAORSBase_Dataqueue class list
 ********************************************************************************************/
class SAORSBase_QueuedData {
	public:
		SAORSBase_QueuedData(IPv4Address destAddr, DT_MSG* dgram) : destAddr(destAddr) {datagram_list.push_back(dgram);}

		IPv4Address destAddr;
		std::list<DT_MSG*> datagram_list;

	public:
		friend std::ostream& operator<<(std::ostream& os, const SAORSBase_QueuedData& o);
		bool operator==(const SAORSBase_QueuedData& o) {
        if(this->destAddr==o.destAddr)
                  return true;
            else
                  return false;
        }
};


/********************************************************************************************
 * class SAORSBase_DataQueue: Stores datagrams awaiting route discovery
 ********************************************************************************************/
class SAORSBase_DataQueue : public cObject {
	public:
		SAORSBase_DataQueue(cSimpleModule *owner, int BUFFER_SIZE_BYTES, uint8_t NUM_COPIES, uint8_t SEND_COPIES, uint8_t SEND_COPIES_PC, bool EPIDEMIC);
		~SAORSBase_DataQueue();

		/** @brief inherited from cObject */
		virtual const char* getFullName() const override;

		/** @brief inherited from cObject */
		virtual std::string info() const;

		/** @brief inherited from cObject */
		virtual std::string str() const override;

		/** @brief checks if list is empty */
		bool isempty();

		/** @brief prints list information */
		void print_info();

		/** @brief checks if packets for this address exists in the queue */
		bool pktsexist(IPv4Address destAddr, int prefix);

		/** @brief returns the destination of the requested position of the queue */
		IPv4Address getDestination(uint k);

		/** @brief returns the size of the DT data Queue */
		uint getSize() {return dataQueue.size();}

		/** @brief overloading the output stream operator */
		friend std::ostream& operator<<(std::ostream& os, const SAORSBase_DataQueue& o);

		//-------------------------------------------------------------//
        //          -- Data Queue Manipulation Operations --           //
		//-------------------------------------------------------------//
		/** @brief places the given DT Messages in the data queue after checks are made */
		int queuePacket(DT_MSG* dtmsg, int prefix);

		/** @brief encapsulates the given IP packet into a DT Message and puts it in the queue */
		void queueIPPacket(const IPv4Datagram* datagram, int prefix, IPv4Address myAddress);

		/** @brief uses reinjectDatagramsTo to send queued DT Messages to the network */
		int dequeuePacketsTo(IPv4Address destAddr, int prefix, IPv4Address intermAddr=IPv4Address(), int previousPkts=0);

		/** @brief uses reinjectDatagramsTo to send copies of the queued DT Messages to the network */
		int spreadPacketsTo(IPv4Address destAddr, int prefix, IPv4Address intermAddr=IPv4Address(), int previousPkts=0);

		/** @brief uses reinjectDatagramsTo to delete or return queued DT Messages */
		void dropPacketsTo(IPv4Address destAddr, int prefix,std::list<DT_MSG*>* datagrams=NULL);
		//-------------------------------------------------------------//

	protected:
		cSimpleModule *moduleOwner;                 /**< The owner class */
		std::list<SAORSBase_QueuedData> dataQueue;  /**< Queued data packets */
		carriersMap copiedCarriers;                 /**< Which DT Messages have been sent to whom */
		carriersMap pastCarriers;                   /**< The past carriers for each message */
		long dataQueueByteSize;                     /**< Total size of all messages currently in dataQueue */

		int BUFFER_SIZE_BYTES;  /**< NED configuration parameter: maximum total size of queued packets, -1 for no limit */
		uint8_t NUM_COPIES;     /**< NED configuration parameter: the initial amount of copies that can be made from a DT Packet */
		uint8_t SEND_COPIES;    /**< NED configuration parameter: the number of copies to be sent to the other nodes */
		uint8_t SEND_COPIES_PC; /**< NED configuration parameter: the percentage of the copies left to be sent to the other nodes */
		bool EPIDEMIC;          /**< NED configuration parameter: defines whether the copy spreading will take place epidemically */

		/** @brief re-injects the DT Messages towards the given destination to the network or the given list structure */
		int reinjectDatagramsTo(IPv4Address destAddr, int prefix, IPv4Address intermAddr, Results verdict,std::list<DT_MSG*> *datagrams=NULL, int previousPkts=0);

		/** @brief decapsulates the given DT Message to IP and send it to localhost */
		void sendtoHL(DT_MSG* dtmsg);

		/** @brief generates and sends a copy of the given message according to the imposed restrictions. */
		bool sendCopy(DT_MSG* dtmsg, IPv4Address destAddr, double delay_var);

		//-------------------------------------------------------------//
		//  -- Map functions manipulating the copies of the packets -- //
		//  --  spread around, and the addresses of the carriers    -- //
		//-------------------------------------------------------------//
		/** @brief checks if a carrier has received the packet specified, in the past */
		bool hasCarrierBeenCopied(IPv4Address carrierAddress, std::string MsgName);

		/** @brief checks if a node was a past carrier of a message about to be spread */
		bool wasPastCarrier(IPv4Address carrierAddress, std::string MsgName);

		/** @brief indicates that the carrier mentioned received the packet specified */
		bool updateCopiedCarriers(IPv4Address carrierAddress, std::string MsgName);

		/** @brief indicates that the carrier mentioned has sent us the packet specified */
        bool updatePastCarriers(IPv4Address carrierAddress, std::string MsgName);
		//-------------------------------------------------------------//
};

} // namespace inetmanet

} // namespace inet
#endif /* SAORSBASE_DATAQUEUE_H */

