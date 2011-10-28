//*********************************************************************************
// File:           HipFsmBase.h
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


#ifndef __HIPFSMBASE_H__
#define __HIPFSMBASE_H__

#include <omnetpp.h>
#include "IPv6ControlInfo.h"
#include "HipMessages_m.h"
#include "IPv6Datagram.h"
//#include <stdlib.h>
//#include <time.h>
#include "InterfaceTableAccess.h"
#include "IPv6InterfaceData.h"
#include "HIP.h"


class INET_API HipFsmBase : public cSimpleModule
{
private:
        //we save the trigger msg here, and process it after the basekey exchange finished
        cMessage* triggerMsg;
		cMessage* creditMsg;

		//Parent HIP module
		HIP* hipmodule;
public:
	// CBA variables
	float creditAgingFactor;
	unsigned int creditAgingInterval;
	unsigned int creditCounter;

	//working variables
	//own HIT
	IPv6Address ownHit;
	//partner HIT
	IPv6Address partnerHit;
	//if HIP module is an RVS module
	bool isRvs;
	//SPIs for ESP messages
	int localSPI;
	int partnerSPI;
	//if the key-exchange process is a registration to RVS
	bool isRegistration;
	//RVS HIT
	IPv6Address rvsHit;
	//if message arrived via RVS
	bool viarvs;

	//FSM states
	enum states
	{
		UNASSOCIATED = 0,
		I1_SENT = 1,
		I2_SENT = 2,
		R2_SENT = 3,
		ESTABLISHED = 4
	};

	//current state
	int currentState;

	//source address
	std::list<IPv6Address> srcAddressList;
	int currentIfId;

	//work variables
	IPv6Address *dstIPwork;
	std::vector<std::string> address;

	bool addrIsValidated;
	bool isUpdating;
	unsigned int updSeq;

	//constructor/destructor
	HipFsmBase();
	virtual ~HipFsmBase();
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

	//handle messages arrived from transport layer
	virtual void handleMsgLocalIn(cMessage *msg);
	//handle messages arrived from ip layer
	virtual void handleMsgRemoteIn(cMessage *msg);

	virtual void handleAddressChange();
	virtual void handleCreditAging();

	virtual void specInitialize();
	virtual void findIPaddress(IPv6Address &HIT);
	virtual void setIPaddress(IPv6Address &HIT, IPv6Address& IP);
	virtual void addIPaddress(IPv6Address &HIT, IPv6Address& IP);
	virtual void deleteAllIPaddress(IPv6Address &HIT);
	virtual void createUpdHIPpacket(int updKind, HIPHeaderMessage * old);
	virtual void createHIPpacket(int kind, const char name[]);
	virtual void createHIPpacket(int kind, const char name[], IPv6Address& via_rvs);
	virtual void addControlInfoAndSend(HIPHeaderMessage *hipPacket);
};

#endif
