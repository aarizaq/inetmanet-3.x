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

#include <omnetpp.h>
#include "SAORSBase_DataQueue.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"

namespace inet {

namespace inetmanet {


/**
 * Assisting function for creating DT_MSGs
 */
/********************************************************************/
cPacket* decapsulateIP(IPv4Datagram *datagram);
DT_MSG* createDTMSG(IPv4Datagram *dgram, IPv4Address myAddress, uint8_t copies);
/********************************************************************/


std::ostream& operator<<(std::ostream& os, const SAORSBase_QueuedData& o)
{
	os << "[ ";
	os << "destAddr: " << o.destAddr << ", Size: " << o.datagram_list.size();
	os << " ]";

	return os;
}


/*****************************************************************************
 * Class Constructor
 *****************************************************************************/
SAORSBase_DataQueue::SAORSBase_DataQueue(cSimpleModule *owner, int BUFFER_SIZE_BYTES, uint8_t NUM_COPIES, uint8_t SEND_COPIES, uint8_t SEND_COPIES_PC, bool EPIDEMIC) :
dataQueueByteSize(0), BUFFER_SIZE_BYTES(BUFFER_SIZE_BYTES), NUM_COPIES(NUM_COPIES), SEND_COPIES(SEND_COPIES), SEND_COPIES_PC(SEND_COPIES_PC), EPIDEMIC(EPIDEMIC)
{
	moduleOwner = owner;
}


/*****************************************************************************
 * Class Destructor
 *****************************************************************************/
SAORSBase_DataQueue::~SAORSBase_DataQueue() {
	dropPacketsTo(IPv4Address::ALLONES_ADDRESS, 0);
}


/*****************************************************************************
 * Inherited from cObject -  Returns the class name.
 *****************************************************************************/
const char* SAORSBase_DataQueue::getFullName() const {
	return "SAORSBase_DataQueue";
}


/*****************************************************************************
 * Inherited from cObject -  Return in a string the class information.
 *****************************************************************************/
std::string SAORSBase_DataQueue::info() const {
	std::ostringstream ss;

	int total = dataQueue.size();
	ss << total << " queued datagrams: ";

	ss << "{" << std::endl;
	for (std::list<SAORSBase_QueuedData>::const_iterator iter = dataQueue.begin(); iter != dataQueue.end(); iter++) {
		SAORSBase_QueuedData e = *iter;
		ss << "  " << e << std::endl;
	}
	ss << "}";

	return ss.str();
}



/*****************************************************************************
 * Inherited from cObject -  The same and the info function.
 *****************************************************************************/
std::string SAORSBase_DataQueue::str() const {
	return info();
}



/*****************************************************************************
 * Checks if packets for this address exists in the queue.
 *****************************************************************************/
bool SAORSBase_DataQueue::pktsexist(IPv4Address destAddr, int prefix) {
	bool reply=false;

	for (std::list<SAORSBase_QueuedData>::iterator iter = dataQueue.begin(); iter != dataQueue.end(); iter++) {

		if(iter->destAddr.prefixMatches(destAddr, prefix) && !iter->datagram_list.empty()) {
			reply=true;
			break;
		}
	}

	return reply;
}


/*****************************************************************************
 * Returns the destination of the requested position of the queue.
 *****************************************************************************/
IPv4Address SAORSBase_DataQueue::getDestination(uint k) {
	//Take the first object
	std::list<SAORSBase_QueuedData>::iterator iter = dataQueue.begin();

	//More iterator to the requested position
	std::advance(iter, k);

	//Return the destinaton address
	return iter->destAddr;
}


/*****************************************************************************
 * Encapsulates the given IP packet into a DT Message and puts it in the queue.
 *****************************************************************************/
void SAORSBase_DataQueue::queueIPPacket(const IPv4Datagram* datagram, int prefix, IPv4Address myAddress) {

	IPv4Address destAddr = datagram->getDestAddress();
	std::list<SAORSBase_QueuedData>::iterator iter;

	// Create DT_MSG
	DT_MSG *dtmsg =  NULL;
	IPv4Datagram* dgram = datagram->dup();
	dtmsg = createDTMSG(dgram, myAddress, NUM_COPIES);
	delete datagram;

	queuePacket(dtmsg, prefix);
}


/*****************************************************************************
 * Places the given DT Messages in the data queue after checks are made.
 *****************************************************************************/
int SAORSBase_DataQueue::queuePacket(DT_MSG* dtmsg, int prefix) {

    bool exist=false;
    DT_MSG* tmp=NULL;
	std::list<SAORSBase_QueuedData>::iterator iter;
	std::list<DT_MSG*>::iterator msg_iter;


	//Extract the delay tolerant message info
	IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(dtmsg->getControlInfo());
	IPv4Address destAddr = dtmsg->getDstAddress();

	EV << "Queuing delayed data packet to " << destAddr << endl;

	// Check address in control info and DT-MSG field agree then it's for us
	if( destAddr ==  controlInfo->getDestAddr() ) {
		sendtoHL(dtmsg);
		return 1;
	}

	//Update the past carriers list if the packet arrived from another node
    if(dtmsg->isCopy()==true)
        updatePastCarriers(controlInfo->getSrcAddr(), std::string(dtmsg->getFullName()));

	//Check if this IP Address already exists
	iter = std::find(dataQueue.begin(),dataQueue.end(), SAORSBase_QueuedData(destAddr,NULL));
	if (iter != dataQueue.end()) {
		SAORSBase_QueuedData qd = *iter;
        //Check if the packets already exists in the list
        for (msg_iter=iter->datagram_list.begin(); msg_iter!=iter->datagram_list.end(); msg_iter++) {
            tmp = * msg_iter;

            //Delete the copy message if found (the one with zero copies left!)
            if(strcmp(tmp->getFullName(),dtmsg->getFullName())==0) {
                //Combine the number of copies from both messages if this is not an epidemic scheme
                if(!EPIDEMIC) {
                    tmp->setCopiesLeft(dtmsg->getCopiesLeft()+tmp->getCopiesLeft());
                }
                delete dtmsg;
                return -1;
            }
        }

        //Else put it there and proceed normally
        iter->datagram_list.push_back(const_cast<DT_MSG *> (dtmsg));
        exist=true;
	}

	//If IP address does not exists, put it in the list
	if(exist==false) {
		dataQueue.push_back(SAORSBase_QueuedData(destAddr, const_cast<DT_MSG *> (dtmsg)));

		//Set iterator position
		iter = dataQueue.end(); iter--;
	}

	//Measuring List size
	dataQueueByteSize += dtmsg->getByteLength();

	// if buffer is full, force dequeuing of old packets for the specific IP
	while (((BUFFER_SIZE_BYTES != -1) && (dataQueueByteSize > BUFFER_SIZE_BYTES))) {
		dataQueueByteSize -= iter->datagram_list.front()->getByteLength();

		//Deleting packet
		DT_MSG* tempdtmsg = iter->datagram_list.front();
		iter->datagram_list.pop_front();
		delete tempdtmsg;

		EV << "Forced dropping of delayed data packet to " << destAddr << endl;
	}

	return(0);
}


/*****************************************************************************
 * Decapsulates the given DT Message to IP and send it to localhost.
 *****************************************************************************/
void SAORSBase_DataQueue::sendtoHL(DT_MSG* dtmsg) {

	IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(dtmsg->removeControlInfo());
	controlInfo->setProtocol(dtmsg->getTransportProtocol());

	// TODO Fix the source address information
	//controlInfo->setSrcAddr(dtmsg->getSrcAddress());
	controlInfo->setSrcAddr(IPv4Address::LOOPBACK_ADDRESS);
	cPacket *packet = dtmsg->decapsulate();

	// attach new control info
	packet->setControlInfo(controlInfo);

	moduleOwner->send(packet, "ipOut");

	delete dtmsg;
	return;
}


/*****************************************************************************
 * Re-injects the DT Messages towards the given destination to the network or
 * the given list structure.
 *****************************************************************************/
int SAORSBase_DataQueue::reinjectDatagramsTo(IPv4Address destAddr, int prefix, IPv4Address intermAddr, Results verdict, std::list<DT_MSG*> *datagrams, int previousPkts) {
	bool tryAgain = true;
	IPv4Address tempAddr;
	int reinjectedPkts = 0;
	double delay = 0;
#define ARP_DELAY 0.001
	while (tryAgain) {
		tryAgain = false;
		for (std::list<SAORSBase_QueuedData>::iterator iter = dataQueue.begin(); iter != dataQueue.end(); iter++) {
			while(iter->destAddr.prefixMatches(destAddr, prefix) && !iter->datagram_list.empty()) {

				//Take the first packet from the list
				dataQueueByteSize -= iter->datagram_list.front()->getByteLength();
				DT_MSG* dtmsg = iter->datagram_list.front();
				iter->datagram_list.pop_front();

                //If Datagram list is empty delete node
                if(iter->datagram_list.empty() && verdict!=SPREAD_P) {
                    dataQueue.erase(iter);
                    iter=dataQueue.begin();

                    //Delete last IP datagram
                    if(verdict==DROP_P && datagrams == NULL) {
                        delete dtmsg;
                        tryAgain = true;
                        break;
                    }
                }

				if (verdict==ACCEPT_P)
				{
				    // If intermAddr is zero, next hop is to final destination
                    if(intermAddr.isUnspecified()) tempAddr = destAddr;
                    else tempAddr = intermAddr;

                    EV << "Dequeuing delayed data packet for " << iter->destAddr << " to " << tempAddr << endl;

					//Find out the node carried this message in the past
                    bool pastCarrier = wasPastCarrier(tempAddr, std::string(dtmsg->getFullName()));

					//If the message is a copy and the candidate node was a past carrier
					if(pastCarrier && dtmsg->isCopy()==true) {
					    //Push the initial message into the collected datagrams list
					    datagrams->push_back( dtmsg );
					}
					//Else send it
					else {
					    // Set Destination Address
                        IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(dtmsg->removeControlInfo());

                        controlInfo->setSrcAddr(IPv4Address::UNSPECIFIED_ADDRESS);
                        controlInfo->setDestAddr(tempAddr);
                        dtmsg->setControlInfo(controlInfo);

                        //Count the re-injected packets
                        reinjectedPkts++;

                        moduleOwner->sendDelayed(dtmsg, delay+previousPkts*ARP_DELAY, "ipOut");
                        delay += ARP_DELAY;
					}
				}
				else if (verdict==SPREAD_P) {
				    //Find the destination attempted to be contacted
				    //If intermAddr is zero, next hop is to final destination, this should not happen
                    if(intermAddr.isUnspecified()) {
                        //WHAT THE F**K - THIS SHOULD NOT HAPPEN!!!
                        std::cerr << "ERROR: Packets to be copied to final destination " << destAddr << "!!!" << endl;
                        //Set the final destination address
                        tempAddr = destAddr;
                    }
                    else tempAddr = intermAddr;

                    EV << "Coping delayed data packet for " << iter->destAddr << " to " << tempAddr << endl;

                    //Create a copy for the found DT Message
				    if(sendCopy(dtmsg, tempAddr, delay+previousPkts*ARP_DELAY)){
				        reinjectedPkts++;
				        delay += ARP_DELAY;
				    }

                    //Push the initial message into the collected datagrams list
                    datagrams->push_back( dtmsg );
				}
				else if (verdict==DROP_P && datagrams != NULL) {
					//Count the re-injected packets
					reinjectedPkts++;

					datagrams->push_back( dtmsg );
				}
				else if (verdict==DROP_P)
					delete dtmsg;

				tryAgain = true;
			}
		}
	}

	return reinjectedPkts;
}


/*****************************************************************************
 * Uses reinjectDatagramsTo to send queued DT Messages to the network.
 *****************************************************************************/
int SAORSBase_DataQueue::dequeuePacketsTo(IPv4Address destAddr, int prefix, IPv4Address intermAddr, int previousPkts) {
	int numOfPkts = 0;
	std::list<DT_MSG*> datagrams;

	numOfPkts = reinjectDatagramsTo(destAddr, prefix, intermAddr, ACCEPT_P, &datagrams, previousPkts);

	//For the packets that were not sent, put them back to the queue from datagrams list
    if(!datagrams.empty()) {
        while(datagrams.size()>0) {
            //Get the message from the list
            DT_MSG* dtmsg = datagrams.front();
            datagrams.pop_front();

            //Put it back in the queue
            queuePacket(dtmsg, prefix);
        }
    }

	return numOfPkts;
}


/*****************************************************************************
 * Uses reinjectDatagramsTo to send copies of the queued DT Messages to the
 * network.
 *****************************************************************************/
int SAORSBase_DataQueue::spreadPacketsTo(IPv4Address destAddr, int prefix, IPv4Address intermAddr, int previousPkts) {
    int numOfPkts = 0;
    std::list<DT_MSG*> datagrams;

    EV << "Trying to spread packets by using multiple copies for the address:  " <<destAddr << endl;

    numOfPkts = reinjectDatagramsTo(destAddr, prefix, intermAddr, SPREAD_P, &datagrams, previousPkts);

    //Since packets were being spread, put them back to the queue from datagrams list
    if(!datagrams.empty()) {
        while(datagrams.size()>0) {
            //Get the message from the list
            DT_MSG* dtmsg = datagrams.front();
            datagrams.pop_front();

            //Put it back in the queue
            queuePacket(dtmsg, prefix);
        }
    }

    return numOfPkts;
}


/*****************************************************************************
 * Uses reinjectDatagramsTo to delete or return queued DT Messages.
 *****************************************************************************/
void SAORSBase_DataQueue::dropPacketsTo(IPv4Address destAddr, int prefix,std::list<DT_MSG*>* datagrams) {
	reinjectDatagramsTo(destAddr, prefix, IPv4Address(), DROP_P, datagrams, 0);
}


/*****************************************************************************
 * Overloading the output stream operator.
 *****************************************************************************/
std::ostream& operator<<(std::ostream& os, const SAORSBase_DataQueue& o)
{
	os << o.info();
	return os;
}


/*****************************************************************************
 * Checks if list is empty.
 *****************************************************************************/
bool SAORSBase_DataQueue::isempty() {
	return dataQueue.empty();
}


/*****************************************************************************
 * Prints list information
 *****************************************************************************/
void SAORSBase_DataQueue::print_info() {

	for (std::list<SAORSBase_QueuedData>::iterator iter = dataQueue.begin(); iter != dataQueue.end(); iter++) {
		std::cout << "List size: " << iter->datagram_list.size() << endl;

		for (std::list<DT_MSG*>::iterator iter2 = iter->datagram_list.begin(); iter2 != iter->datagram_list.end(); iter2++) {
			DT_MSG* msg=*iter2;
			std::cout << "Packet read: " << msg->getByteLength() << endl;
		}
	}

	std::cout << "Info finished" << endl;
}


/*****************************************************************************
 * Checks if a carrier has received the packet specified, in the past.
 *****************************************************************************/
bool SAORSBase_DataQueue::hasCarrierBeenCopied(IPv4Address carrierAddress, std::string MsgName) {
    //Check map for the carrier address
    carriersMapIter ccIter = copiedCarriers.find(carrierAddress);

    //If a record exists, check all the copied DT Packets
    if(!copiedCarriers.empty() && ccIter!=copiedCarriers.end()) {
        //Check names of every copied packet, if found return true
        if(std::find(ccIter->second.begin(),ccIter->second.end(),MsgName)!=ccIter->second.end())
            return true;
    }

    //Otherwise return false
    return false;
}


/*****************************************************************************
 * Checks if a node was a past carrier of a message about to be spread.
 *****************************************************************************/
bool SAORSBase_DataQueue::wasPastCarrier(IPv4Address carrierAddress, std::string MsgName) {
    //Check map for the past carriers addressC++ list erase does not delete pointer
    carriersMapIter pcIter = pastCarriers.find(carrierAddress);

    //If a record exists, check all the copied DT Packets
    if(!pastCarriers.empty() && pcIter!=pastCarriers.end()) {
        //Check names of packet carried in the past, if found return true
        if(std::find(pcIter->second.begin(),pcIter->second.end(),MsgName)!=pcIter->second.end())
            return true;
    }

    //Otherwise return false
    return false;
}


/*****************************************************************************
 * Indicates that the carrier mentioned received the packet specified.
 *****************************************************************************/
bool SAORSBase_DataQueue::updateCopiedCarriers(IPv4Address carrierAddress, std::string MsgName) {
    //Try to find the record of the requested carrier
    carriersMapIter ccIter = copiedCarriers.find(carrierAddress);

    //If there is no such record
    if(ccIter==copiedCarriers.end()) {
        //Create a new list object and add it in the map
        std::list<std::string> copiedMsgs;
        copiedMsgs.push_back(MsgName);
        copiedCarriers.insert(std::pair<IPv4Address, std::list<std::string> > (carrierAddress, copiedMsgs));
    }
    //If there is such a record
    else {
        //If the copied messaqe's name isn't already on the list
        //If not in the list, put it in the back
        if(std::find(ccIter->second.begin(),ccIter->second.end(),MsgName)==ccIter->second.end())
            ccIter->second.push_back(MsgName);
        //Otherwise something went wrong
        else {
            EV << "The node contacted bas also been copied in the past!" << endl;
            return false;
        }
    }

    return true;
}


/*****************************************************************************
 * Indicates that the carrier mentioned has sent us the packet specified.
 *****************************************************************************/
bool SAORSBase_DataQueue::updatePastCarriers(IPv4Address carrierAddress, std::string MsgName) {
    //Try to find the record of the requested carrier
    carriersMapIter pcIter = pastCarriers.find(carrierAddress);

    //If there is no such record
    if(pcIter==pastCarriers.end()) {
        //Create a new list object and add it in the map
        std::list<std::string> copiedMsgs;
        copiedMsgs.push_back(MsgName);
        pastCarriers.insert(std::pair<IPv4Address, std::list<std::string> > (carrierAddress, copiedMsgs));
    }
    //If there is such a record
    else {
        //Check if the same message has been received in the past
        //If not in the list, put it in the back
        if(std::find(pcIter->second.begin(),pcIter->second.end(),MsgName)==pcIter->second.end())
            pcIter->second.push_back(MsgName);
        //Else return that it was found
        else {
            EV << "The DT message received, has been sent in the past as well!" << endl;
            return false;
        }
    }

    return true;
}


/*****************************************************************************
 * Generates and sends a copy of the given message according to the imposed
 * restrictions.
 *****************************************************************************/
bool SAORSBase_DataQueue::sendCopy(DT_MSG* dtmsg, IPv4Address destAddr, double delay_var) {

    //Find if the node has been copied in the past
    bool copied = hasCarrierBeenCopied(destAddr, std::string(dtmsg->getFullName()));

    //Find out the node carried this message in the past
    bool pastCarrier = wasPastCarrier(destAddr, std::string(dtmsg->getFullName()));

    //============================================//
    //       -- DT Message Copy Generation --     //
    //============================================//
    //If the packet has more available copies, sent one, to the newly copied carrier
    if( ( EPIDEMIC || ( dtmsg->getCopiesLeft()>1 && (SEND_COPIES>0 || SEND_COPIES_PC>0) ) )  && copied==false && pastCarrier==false) {

        //Duplicate the delay tolerant message
        DT_MSG *dtMsgCopy = dtmsg->dup();

        //Create a new control info item, to put in the new packet
        IPv4ControlInfo *controlInfo = new IPv4ControlInfo(*(check_and_cast<IPv4ControlInfo*>(dtmsg->getControlInfo())));
        controlInfo->setSrcAddr(IPv4Address::UNSPECIFIED_ADDRESS);
        controlInfo->setDestAddr(destAddr);
        dtMsgCopy->setControlInfo(controlInfo);

        //Check if the percentage is used for  computing the number of copies to be sent
        if(EPIDEMIC) {
            dtMsgCopy->setCopiesLeft(dtmsg->getCopiesLeft());
        }
        else if(SEND_COPIES_PC>0) {
            //Compute the percentage of copies to be sent
            uint8_t numOfCopies=(uint8_t)ceil(((double)SEND_COPIES_PC*(double)dtmsg->getCopiesLeft()/100));

            //If the number of copies if equal or larger than the existing copies, reduce it by 1
            if(numOfCopies>=dtmsg->getCopiesLeft())
                numOfCopies=(uint8_t)(dtmsg->getCopiesLeft()-1);

            dtmsg->setCopiesLeft((uint8_t)(dtmsg->getCopiesLeft()-numOfCopies));
            dtMsgCopy->setCopiesLeft(numOfCopies);
        }
        //Otherwise use the variable denoting the static number of copies
        else {
            //If there are enough copies, send them all
            if(dtmsg->getCopiesLeft()>SEND_COPIES) {
                dtmsg->setCopiesLeft((uint8_t)(dtmsg->getCopiesLeft()-SEND_COPIES));
                dtMsgCopy->setCopiesLeft(SEND_COPIES);
            }
            //If not, try to send one by one, until there are none
            else {

                dtmsg->setCopiesLeft((uint8_t)(dtmsg->getCopiesLeft()-1));
                dtMsgCopy->setCopiesLeft((uint8_t)1);
            }
        }
        dtMsgCopy->setIsCopy(true);

        //Send packet down to IP
        moduleOwner->sendDelayed(dtMsgCopy, delay_var, "ipOut");

        //Update the copied carriers list
        updateCopiedCarriers(destAddr, std::string(dtMsgCopy->getFullName()));

        //A copy has indeed been sent
        return true;
    }
    //============================================//

    //No copy has been sent
    return false;

}


/********************************************************************
 * Assisting function for creating DT_MSGs.
 * Creation of a DT_MSG by decapsulating an IP datagram.
 ********************************************************************/
/** Wrapper for creating a DT Message, encapsulating an IP datagram */
DT_MSG* createDTMSG(IPv4Datagram *dgram, IPv4Address myAddress, uint8_t copies) {

	// Decapsulating IP packet and acquiring the control info
	cPacket *packet=NULL;
	IPv4Address destAddr = dgram->getDestAddress();

	packet = decapsulateIP(dgram);
	IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo*>(packet->removeControlInfo());

	// Creating delay tolerant message to piggyback the transported packet
	DT_MSG *dtmsg = new DT_MSG(packet->getName());
	dtmsg->setByteLength(sizeof(uint32_t)+sizeof(int));
	dtmsg->encapsulate(packet);
	dtmsg->setTransportProtocol(controlInfo->getProtocol());
	dtmsg->setDstAddress(destAddr);
	dtmsg->setSrcAddress(myAddress);
	dtmsg->setCopiesLeft(copies);
	dtmsg->setIsCopy(false);

	// Change Control Info
	controlInfo->setProtocol(IP_PROT_MANET);
	dtmsg->setControlInfo(controlInfo);

	return dtmsg;
}



/** Copied from IP Level */
cPacket* decapsulateIP(IPv4Datagram *datagram)
{
    // decapsulate transport packet
    cPacket *packet = datagram->decapsulate();

    // create and fill in control info
    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setProtocol(datagram->getTransportProtocol());
    controlInfo->setSrcAddr(IPv4Address::UNSPECIFIED_ADDRESS);
    controlInfo->setDestAddr(IPv4Address::UNSPECIFIED_ADDRESS);
    controlInfo->setDiffServCodePoint(datagram->getDiffServCodePoint());
    controlInfo->setInterfaceId(-1);
    controlInfo->setTimeToLive(datagram->getTimeToLive());

    // original IP datagram might be needed in upper layers to send back ICMP error message
    controlInfo->setOrigDatagram(datagram);

    // attach control info
    packet->setControlInfo(controlInfo);

    return packet;
}
/********************************************************************/
} // namespace inetmanet

} // namespace inet

