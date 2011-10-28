//*********************************************************************************
// File:           HipFsmBase.cc
//
// Authors:        Laszlo Tamas Zeke, Levente Mihalyi, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
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
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************

#include <omnetpp.h>
#include "HipFsmBase.h"
#include "InterfaceTable.h"
#include "HIP.h"

HipFsmBase::HipFsmBase(){}

// Destructor
HipFsmBase::~HipFsmBase()
{
	delete cancelEvent(creditMsg);
    creditMsg = NULL;
    if(triggerMsg)
    	delete triggerMsg;
}

//initialization
void HipFsmBase::initialize()
{
       //HIPFSM daemon init
	currentState = UNASSOCIATED;

	hipmodule = check_and_cast<HIP*> (this->getParentModule());

	ownHit.set(hipmodule->par("OWN_HIT"));
	//get partner HIT
	//partnerHit = check_and_cast<HIP*>(this->getParentModule())->getPartnerHIT();
	//if module is under RVS module



	isRvs = hipmodule->isRvs();
	//init
	isRegistration = false;
	localSPI = this->getId();
	partnerSPI = -1;

	dstIPwork = new IPv6Address();

    triggerMsg = NULL;
	viarvs = false;
	addrIsValidated = true;
	isUpdating = false;
	updSeq = 0;

	// Building source address list
	InterfaceTable* ift = (InterfaceTable*)InterfaceTableAccess().get();
	for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);
		if(!(ie->isLoopback()) && !(ie->isDown())) {
			if(hipmodule->mapIfaceToConnected.find(ie) != hipmodule->mapIfaceToConnected.end() && hipmodule->mapIfaceToConnected[ie] == true) {
				if(srcAddressList.empty())
					currentIfId = ie->getInterfaceId();
				srcAddressList.push_back(hipmodule->mapIfaceToIP[ie]);
			}

		}
    }

	WATCH_LIST(srcAddressList);
	WATCH(isUpdating);
	WATCH(addrIsValidated);

	// CBA init
	creditAgingFactor = 0.825;
	creditAgingInterval = 5; //in seconds
	creditCounter = 0;
	this->scheduleAt(simTime() + creditAgingInterval, creditMsg = new cPacket("CREDIT_AGING"));

	specInitialize();

}

// Handles the incoming messages
void HipFsmBase::handleMessage(cMessage *msg)
{
	//msg arrived from transport
	if(msg->arrivedOn("localIn"))
		handleMsgLocalIn(msg);
	//msg arrived from network
	else if (msg->arrivedOn("remoteIn"))
		handleMsgRemoteIn(msg);
	else if (msg->arrivedOn("HIPinfo") && currentState == ESTABLISHED){
		handleAddressChange();
		delete msg;
	}
	else if (msg->isSelfMessage() && msg->isName("CREDIT_AGING")) {
		handleCreditAging();
		delete msg;
	}
}

// The HIP Credit Algorithm (not used)
void HipFsmBase::handleCreditAging(){
	creditCounter = (unsigned int)(((float) creditCounter) * creditAgingFactor);
	this->scheduleAt(simTime() + creditAgingInterval, creditMsg = new cPacket("CREDIT_AGING"));
}

// Handles the change of a used interface and starts the updating process
void HipFsmBase::handleAddressChange(){

	ev << "IPv6 address changed, need updating" << endl;
	InterfaceTable* ift = (InterfaceTable*)InterfaceTableAccess().get();
	srcAddressList.clear();;
	for (int i=0; i<ift->getNumInterfaces(); i++)
    {
        InterfaceEntry *ie = ift->getInterface(i);

		if(!(ie->isLoopback()) && !(ie->isDown())) {
			if(hipmodule->mapIfaceToConnected.find(ie) != hipmodule->mapIfaceToConnected.end() && hipmodule->mapIfaceToConnected[ie] == true) {
				if(srcAddressList.empty())
					currentIfId = ie->getInterfaceId();
				srcAddressList.push_back(hipmodule->mapIfaceToIP[ie]);
			}
		}
    }

	isUpdating = true;

	// Create HIP update message 1
	HIPHeaderMessage *hipHead = new HIPHeaderMessage("UPDATE",UPDATE);
	hipHead->setDestHIT(partnerHit);
	hipHead->setSrcHIT(ownHit);

	HipLocator *srcLoc = new HipLocator();
	srcLoc->locatorESP_SPI = localSPI;
	srcLoc->locatorIPv6addr = srcAddressList.front();
	hipHead->setLocatorArraySize(srcAddressList.size() + 2);
	hipHead->setLocator(0,*srcLoc);
	HipLocator *destLoc = new HipLocator();
	destLoc->locatorESP_SPI = partnerSPI;
	destLoc->locatorIPv6addr = *dstIPwork;
	hipHead->setLocator(1,*destLoc);

	HipLocator *addrLoc;
	int i = 0;
	for(std::list<IPv6Address>::iterator iter = srcAddressList.begin(); iter != srcAddressList.end(); iter++){
		addrLoc = new HipLocator();
		addrLoc->locatorIPv6addr = *iter;
		hipHead->setLocator(i+2,*addrLoc);
		i++;
	}

	addControlInfoAndSend(hipHead);
	ev << "UPDATE 1 sent" << endl;
	if(!isRegistration)
		recordScalar("HO_start", simTime());
}

// Handles messages from transport layer and encapsulates them into ESP messages
void HipFsmBase::handleMsgLocalIn(cMessage *msg)
{
	ev << "Message arrived at FSM from upper layer." << endl;
	switch (currentState)
	{
	case UNASSOCIATED:
	{
           if(msg->isName("HIP_REGISTER_AT_RVS"))
           {
        	    ev << "Trigger to start RVS registration.\n";
				//START RVS registration
				//addressAtTransport src??
				ev << "HIP Base Exchange started" << endl;
				dstIPwork = hipmodule->getRvsAddress();
				partnerHit = *hipmodule->getRvsHit();
				//addressAtTransport = check_and_cast<HIP*>(this->getParentModule())->getSrcWorkAddress();
				isRegistration = true;
				createHIPpacket(I1,"I1");
				currentState = I1_SENT;
				delete msg;
           }
           else
           {
        	   ev << "Trigger to start registration.\n";
				isRegistration = false;
				//START base exchange
				IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());
				ev << "HIP Base Exchange started" << endl;

				//destination HIT
				partnerHit = networkControlInfo->getDestAddr();
				findIPaddress(partnerHit);

				createHIPpacket(I1,"I1");
				currentState = I1_SENT;
				//save trigger msg
				msg->setControlInfo(networkControlInfo);
				triggerMsg = msg;
           }

		break;
         }
	case ESTABLISHED:
	{
		/* TODO credit code!
		if (!addrIsValidated) {
			if(creditCounter > msg->getByteLength()){
				creditCounter -= msg->getByteLength();
			}
			else {
				ev << "Not enough credit for sending packet, dropping" << endl;
				delete msg;
				return;
			}
		}
		else { */
			//create ESP packet
			ev << "Association established, encapsulating in ESP..." << endl;
			char msgName[25] = "ESP";
			if (strlen(msg->getName()) < 18)
			{
				strcat(msgName," (");
         		strcat(msgName,msg->getName());
			    strcat(msgName,")");
            }

			ESPMessage *esp = new ESPMessage(msgName, ESP_DATA);
			IPv6EncapsulatingSecurityPayloadHeader *espHead = new IPv6EncapsulatingSecurityPayloadHeader();
			espHead->setSpi(partnerSPI);
			esp->setEsp(espHead);
            IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());
			findIPaddress(partnerHit);

			// Switch HITs to IP addresses
			IPv6ControlInfo *ipControlInfo = new IPv6ControlInfo();
			ipControlInfo->setProtocol(networkControlInfo->getProtocol());
			ipControlInfo->setHopLimit(networkControlInfo->getHopLimit());
			ipControlInfo->setInterfaceId(networkControlInfo->getInterfaceId());
			ipControlInfo->setDestAddr(*dstIPwork);
			ipControlInfo->setSrcAddr(srcAddressList.front());
			ipControlInfo->setInterfaceId(currentIfId);
            esp->setControlInfo(ipControlInfo);
			delete networkControlInfo;
            //        dynamic_cast<IPv6Datagram*>(msg)->addExtensionHeader(esp);
			esp->encapsulate(check_and_cast<cPacket *> (msg));
			sendDirect(esp, this->getParentModule(), "fromFsmOut");
		//}
		break;

    }
	default:
		ev << "BE probably still running, dropping packet..." << endl;
		delete msg;
		break;
	}
}


// Handles messages from network (HIP protocol messages and payload messages also)
void HipFsmBase::handleMsgRemoteIn(cMessage *msg)
{
	switch (currentState)
	{
	case UNASSOCIATED:
		if (msg->getKind() == I1)
		{
			//generate R1 and send
            HIPHeaderMessage *i1 = check_and_cast<HIPHeaderMessage*>(msg);
			IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());
			//DEBUG
            ev << "datagram source " << networkControlInfo->getSrcAddr() << endl;

			partnerHit = i1->getSrcHIT();
			partnerSPI = i1->getLocator(0).locatorESP_SPI; //srcSPI

			//is it via RVS?
			if (i1->getFrom_i().isUnspecified())
			{
				setIPaddress(partnerHit, networkControlInfo->getSrcAddr());
			}
			else
			{
				viarvs = true;
				setIPaddress(partnerHit, i1->getFrom_i());
			}
			//DEBUG
            //ev << "***HIP dest address " << networkControlInfo->srcAddr();
			findIPaddress(partnerHit);
			if (viarvs)
				createHIPpacket(R1,"R1",networkControlInfo->getSrcAddr());
			else
				createHIPpacket(R1,"R1");
			delete msg;
		}
		else if(msg->getKind() == I2)
		{
			//generate R2 and send
			//HIPHeaderMessage *i2 = dynamic_cast<HIPHeaderMessage *>(msg);
			findIPaddress(partnerHit);
			createHIPpacket(R2,"R2");
			if(isRvs)
				currentState = ESTABLISHED;
			else
				currentState = R2_SENT;
			delete msg;
		}
		else
		{
			delete msg;
		}
		break;

	case I1_SENT:

		if(msg->getKind() == R1)
		{

			HIPHeaderMessage *r1 = dynamic_cast<HIPHeaderMessage *>(msg);
			IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());

			//if arrived via rvs
			if (!r1->getVia_rvs().isUnspecified())
			{
				viarvs = true;
			}

			partnerSPI = r1->getLocator(0).locatorESP_SPI;

            setIPaddress(partnerHit, networkControlInfo->getSrcAddr());
			findIPaddress(r1->getSrcHIT());
			createHIPpacket(I2,"I2");
			currentState = I2_SENT;
			delete msg;
		}
		else
		{
			delete msg;
		}
		break;

	case I2_SENT:

		if (msg->getKind() == R2)
		{
			currentState = ESTABLISHED;
			delete msg;
			if (triggerMsg != NULL)
			{
				//if viarvs then restore original dest address - the HIT
				if (viarvs)
				{
					ev << "REGISTR viarvs setting destaddr to " << partnerHit <<endl;
					IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(triggerMsg->removeControlInfo());
					networkControlInfo->setDestAddr(partnerHit);
					triggerMsg->setControlInfo(networkControlInfo);
				}

				ev << "Sending trigger msg" <<endl;
                sendDirect(triggerMsg, this->getParentModule(), "fromFsmTrigger"); //process triggerMsg
                triggerMsg = NULL;
			}
			/*
			if (isRegistration)
			{
				sendDirect(new cPacket("RegRVSAtDNS"), 0, this->getParentModule(), "fromFsmOut");
			}
			*/
		}
		else
		{
			delete msg;
		}
		break;

	case R2_SENT:

		if (msg->getKind() == ESP_DATA)
		{   ev << "Got an esp at fsm" << endl;
			currentState = ESTABLISHED;
			IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());

			// Credit increment
			if(creditCounter < 500000) {
				creditCounter += check_and_cast<cPacket *> (msg)->getByteLength();
				if(creditCounter > 500000)
					creditCounter = 500000;
			}


			// We need to change the IPv6 addresses to HITs before passing to upper layer
			IPv6ControlInfo * hipControlInfo = new IPv6ControlInfo();

			hipControlInfo->setProtocol(networkControlInfo->getProtocol());
			hipControlInfo->setHopLimit(networkControlInfo->getHopLimit());
			hipControlInfo->setInterfaceId(networkControlInfo->getInterfaceId());
			hipControlInfo->setSrcAddr(partnerHit);
			hipControlInfo->setDestAddr(ownHit);

			cPacket *transportPacket = check_and_cast<cPacket *> (msg)->decapsulate();
			transportPacket->setControlInfo(hipControlInfo);

            sendDirect(transportPacket, this->getParentModule(), "fromFsmIn");
			delete msg;
			delete networkControlInfo;

		}
		break;

	case ESTABLISHED:
		{
			// Normal payload, decapsulate
			if (msg->getKind() == ESP_DATA)
			{
				ev << "Got an esp at fsm" << endl;
				IPv6ControlInfo *networkControlInfo = check_and_cast<IPv6ControlInfo*>(msg->removeControlInfo());

				// We need to change the IPv6 addresses to HITs before passing to upper layer
				IPv6ControlInfo * hipControlInfo = new IPv6ControlInfo();

				hipControlInfo->setProtocol(networkControlInfo->getProtocol());
				hipControlInfo->setHopLimit(networkControlInfo->getHopLimit());
				hipControlInfo->setInterfaceId(networkControlInfo->getInterfaceId());
				hipControlInfo->setSrcAddr(partnerHit);
				hipControlInfo->setDestAddr(ownHit);

				cPacket *transportPacket = check_and_cast<cPacket *> (msg)->decapsulate();
				transportPacket->setControlInfo(hipControlInfo);

				sendDirect(transportPacket, this->getParentModule(), "fromFsmIn");
				delete msg;
				delete networkControlInfo;
			}
			else
			{
				// Updates message, handle them according to state
				if (msg->getKind() == UPDATE && !isUpdating)
				{
					// First UPDATE message (the partner is trying to notify us about new addresses)
					if (addrIsValidated)
					{
						ev << "UPDATE 1 recieved" << endl;
						IPv6Address addr;
						HIPHeaderMessage *upd = check_and_cast<HIPHeaderMessage *>(msg);
						if( upd->getLocatorArraySize() <= 2) {
							ev << "Too few LOCATORs, probably a duplicate?" << endl;
							delete msg;
							return;
						}
						// Update the stored addresses
						deleteAllIPaddress(partnerHit);
						for(unsigned int i = 2; i < upd->getLocatorArraySize(); i++) {
							addIPaddress(partnerHit, upd->getLocator(i).locatorIPv6addr);
							ev << "Addresses stored: " << upd->getLocator(i).locatorIPv6addr << endl;
						}
						addrIsValidated = false;

						findIPaddress(partnerHit);
						if (dstIPwork == NULL)
							error("Address not stored?");
						ev << "Sent to: " << *dstIPwork << endl;
						createUpdHIPpacket(2, upd);
						ev << "UPDATE 2 sent" << endl;

						delete msg;
					}
					// Validate update message (end of Update sequence)
					else
					{
						IPv6Address addr;
						ev << "UPDATE 3 recieved" << endl;
						//HIPHeaderMessage *upd = check_and_cast<HIPHeaderMessage *>(msg);

						//if(upd.getAck() == updSeq){
						addrIsValidated = true;
						updSeq++;
						//}

						delete msg;
					}
				}
				// Send out third Update message (this node is trying to update its interfaces)
				else if (msg->getKind() == UPDATE && isUpdating) {
					IPv6Address addr;
					HIPHeaderMessage *upd = check_and_cast<HIPHeaderMessage *>(msg);

					ev << "UPDATE 2 recieved" << endl;
					createUpdHIPpacket(3, upd);
					ev << "UPDATE 3 sent" << endl;

					isUpdating = false;
					delete msg;
					if(!isRegistration)
						recordScalar("HO_finish", simTime());
				}
				else
					delete msg;
			}


		}
	break;
	}
}


// Functions to be overridden by child (see HipFsm)
void HipFsmBase::specInitialize(){}
void HipFsmBase::findIPaddress(IPv6Address &HIT){}
void HipFsmBase::setIPaddress(IPv6Address &HIT,IPv6Address& IP){}
void HipFsmBase::addIPaddress(IPv6Address &HIT, IPv6Address& IP) {}
void HipFsmBase::deleteAllIPaddress(IPv6Address &HIT) {}

// Create HIP packet with no via_rvs
void HipFsmBase::createHIPpacket(int kind, const char name[])
{
	IPv6Address* emptyIPv6Address = new IPv6Address();
	createHIPpacket(kind, name, *emptyIPv6Address);
	delete emptyIPv6Address;
}

// Create an Update HIP packet
void HipFsmBase::createUpdHIPpacket(int updKind, HIPHeaderMessage * old){
	HIPHeaderMessage *hipHead = new HIPHeaderMessage("UPDATE",UPDATE);
	hipHead->setDestHIT(partnerHit);
	hipHead->setSrcHIT(ownHit);

	HipLocator *srcLoc = new HipLocator();
	srcLoc->locatorESP_SPI = localSPI;
	srcLoc->locatorIPv6addr = srcAddressList.front();
	hipHead->setLocatorArraySize(2);
	hipHead->setLocator(0,*srcLoc);
	HipLocator *destLoc = new HipLocator();
	destLoc->locatorESP_SPI = partnerSPI;
	destLoc->locatorIPv6addr = *dstIPwork;
	hipHead->setLocator(1,*destLoc);

	srand(time(NULL));
	int randSeq = rand() % 1024 + 1;

	switch(updKind) {
		// CONFIRM UPDATE message params
		case 2:
			hipHead->setSeq(old->getSeq());
			hipHead->setAck(randSeq);
			hipHead->setEcho_request_unsigned(1);
			break;
		case 3:
			hipHead->setSeq(old->getSeq());
			hipHead->setAck(old->getAck());
			break;

		default:
			return;
	}

	addControlInfoAndSend(hipHead);

}

// Creates a BE HIP packet
void HipFsmBase::createHIPpacket(int kind, const char name[], IPv6Address& via_rvs)
{
	//create hip header
	HIPHeaderMessage *hipHead = new HIPHeaderMessage(name, kind);
	hipHead->setDestHIT(partnerHit);
    hipHead->setSrcHIT(ownHit);

	// Working variables
	int z = 0;
	HipLocator *addrLoc;
	if (isRegistration || isRvs)
	{
	  switch (kind)
	  {
			case R1: hipHead->setReg_info(1);
				hipHead->setLocatorArraySize(2 + srcAddressList.size());
				for(std::list<IPv6Address>::iterator i=srcAddressList.begin(); i!=srcAddressList.end(); i++){
					addrLoc = new HipLocator();
					addrLoc->locatorIPv6addr = *i;
					hipHead->setLocator(z + 2, *addrLoc);
					z++;
				}
				break;
			case I2: hipHead->setReg_req(1);
				hipHead->setLocatorArraySize(2 + srcAddressList.size());
				for(std::list<IPv6Address>::iterator i=srcAddressList.begin(); i!=srcAddressList.end(); i++){
					addrLoc = new HipLocator();
					addrLoc->locatorIPv6addr = *i;
					hipHead->setLocator(z + 2, *addrLoc);
					z++;
				}
				break;
			case R2: hipHead->setReg_res(1);
				hipHead->setLocatorArraySize(2);
				break;
			default:
				break;
	  }
	}

	if (! via_rvs.isUnspecified())
	{
		hipHead->setVia_rvs(via_rvs);
	}
	//create source and dest locator
    HipLocator *srcLoc = new HipLocator();
    srcLoc->locatorESP_SPI = localSPI;
    srcLoc->locatorIPv6addr = srcAddressList.front();
    hipHead->setLocatorArraySize(2);
    hipHead->setLocator(0,*srcLoc);
    HipLocator *destLoc = new HipLocator();
    destLoc->locatorESP_SPI = partnerSPI;
	destLoc->locatorIPv6addr = *dstIPwork;
    hipHead->setLocator(1,*destLoc);
	addControlInfoAndSend(hipHead);
}

// Adds a controlinfo to packets to be sent out through the network
void HipFsmBase::addControlInfoAndSend(HIPHeaderMessage *hipPacket)
{
	//add control info, set dest and src and send it to HIP module
	IPv6ControlInfo *networkControlInfo = new IPv6ControlInfo();
	networkControlInfo->setDestAddr(*dstIPwork);
	networkControlInfo->setSrcAddr(srcAddressList.front());
	networkControlInfo->setInterfaceId(currentIfId);
	networkControlInfo->setProtocol(6);
	hipPacket->setControlInfo(networkControlInfo);
	sendDirect(hipPacket, this->getParentModule(), "fromFsmOut");
	hipmodule->incHipMsgCounter();
}
