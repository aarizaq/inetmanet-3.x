/*
 *\class       PASERUDpTrafficSender
 *@brief       Class provides a constant bit rate UDP sender. It simulates to a large extent an Iperf UDP client.
 *
 *\authors     Dennis Kaulbars | Sebastian Subik \@tu-dortmund.de | Eugen.Paul | Mohamad.Sbeiti \@paser.info
 *
 *\copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
 *                  at Technische Universitaet Dortmund, Germany
 *                  http://www.kn.e-technik.tu-dortmund.de/
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ********************************************************************************
 * This work is part of the secure wireless mesh networks framework, which is currently under development by CNI
 ********************************************************************************/

#include "PASERUdpTrafficSender.h"
#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTableAccess.h"

Define_Module(PASERUdpTrafficSender);

void PASERUdpTrafficSender::initialize()
{
    numSent = 0;
	//Initialize InterfaceTable
	ift = InterfaceTableAccess().get();

	//Initialize my Parameters
	bitRate = par("bitRate");
	offset = par("offset");
	stopTime = par("stopTime");
	packetLength = par("packetLength");
	myCounter = 0;
	myId="";
	myPort = par("port");
	destPort = par("port");


	//Set Message Frequency
	messageFreq = packetLength/bitRate;
	WATCH(messageFreq);
	WATCH(myCounter);

	//should this node send messages? Default=true
	runMe=true;

	if(runMe==true){
		cMessage *newTimer = new cMessage("messageTimer");
		scheduleAt(simTime()+offset,newTimer);
	}

	//Start Interface Timer
	cMessage *interfaceTimer = new cMessage("interfaceTimer");
	scheduleAt(simTime()+0.01,interfaceTimer);

	socket.setOutputGate(gate("udpOut"));
    socket.bind(myPort);
    setSocketOptions();

	return;
}

void PASERUdpTrafficSender::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0])
    {
        IInterfaceTable *ift = InterfaceTableAccess().get(this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups)
        socket.joinLocalMulticastGroups();
}

void PASERUdpTrafficSender::handleMessage(cMessage *msg)
{
	//Check if message is a selfMessage
	if(msg->isSelfMessage()==true){
		handleSelfMessage(msg);
		return;
	}

	//External Message
	if(msg->isSelfMessage()==false){
		handleExtMessage(msg);
		return;
	}

	//Other messages
	EV << "Unknown message type" << endl;
	delete msg;

	return;
}

void PASERUdpTrafficSender::handleSelfMessage(cMessage *selfMsg)
{
	//Check if message is a messageTimer
	if(strcmp(selfMsg->getName(),"messageTimer")==0){
        // on first call we need to initialize
        if (destAddr.isUnspecified())
        {
            destAddr = IPvXAddressResolver().resolve(par("destAddr"));
            ASSERT(!destAddr.isUnspecified());
        }
		startDataMessage();
		delete selfMsg;
		return;
	}

	//Check if message is a interfaceTimer
	if(strcmp(selfMsg->getName(),"interfaceTimer")==0){
		myId = getInterface()->ipv4Data()->getIPAddress().str();

		//Delete messages and finish
		delete selfMsg;
		return;
	}

	//Other type
	std::cout << "Unknown selfMessage Type!" << std::endl;
	delete selfMsg;
	return;
}

void PASERUdpTrafficSender::handleExtMessage(cMessage *extMsg)
{
	EV << "This module is not able to handle external messages!" << endl;
	delete extMsg;
	return;
}

void PASERUdpTrafficSender::startDataMessage(){

	//Create message
	PaserTrafficDataMsg *newPacket = createDataMessage();

	//Send message
	sendDataMessage(newPacket,destAddr.str());

	//Start new Timer
	if(simTime()<stopTime){
		cMessage *newTimer = new cMessage("messageTimer");
		scheduleAt(simTime()+messageFreq,newTimer);
	}


	return;
}

InterfaceEntry* PASERUdpTrafficSender::getInterface()
{
	EV<< "Method " << this->getFullPath() << ": getInterface() called..." << endl;

	InterfaceEntry *   ie;
	InterfaceEntry *   i_face;
	int num_80211=0;
	const char *name;

	for (int i = 0; i < ift->getNumInterfaces(); i++)
	{
		ie = ift->getInterface(i);
	    name = ie->getName();
	    EV << "Interface Name: " << name << endl;
	    if (strstr (name,"wlan")!=NULL)
	    {
	    	i_face = ie;
	        num_80211++;
	        interfaceId = i;
	    }
	}

	// One enabled network interface (in total)
	if (num_80211==1){
		EV << "Interface found and returned, ID=" << i_face->getInterfaceId() << endl;
		return i_face;
	}
	else
		return NULL;
}

PaserTrafficDataMsg* PASERUdpTrafficSender::createDataMessage(){
	EV<< "Method " << this->getFullPath() << ": createDataMessage() called..." << endl;

	//Create message
	PaserTrafficDataMsg *dataPkt = new PaserTrafficDataMsg("TrafficData");

	//Set parameters
	dataPkt->setBitLength(packetLength);
	dataPkt->setSrcId(myId.c_str());
	dataPkt->setCounter(myCounter);
	dataPkt->setTimestamp(simTime().dbl());

	//Incremenent Counter
	myCounter++;

	//return packet, finish
	return dataPkt;
}

void PASERUdpTrafficSender::sendDataMessage(PaserTrafficDataMsg *dataMsg, std::string destId){
	EV<< "Method " << this->getFullPath() << ": sendDataMessage() called..." << endl;

	//send Packet to UDP-Layer
	socket.sendTo(dataMsg,IPv4Address(destId.c_str()),destPort);
//	sendToUDP(dataMsg,myPort,IPv4Address(destId.c_str()),destPort);

	numSent++;
    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "sent: %d pks", numSent);
        getDisplayString().setTagArg("t",0,buf);
    }
	return;
}

int PASERUdpTrafficSender::getNumberOfSentMessages(){
	EV<< "Method " << this->getFullPath() << ": getNumberOfSentMessages() called..." << endl;

	return myCounter;
}

std::string PASERUdpTrafficSender::getMyId(){
	EV<< "Method " << this->getFullPath() << ": getMyId() called..." << endl;

	return myId;
}

double PASERUdpTrafficSender::getDataRate(){
	EV<< "Method " << this->getFullPath() << ": getDataRate() called..." << endl;

	return bitRate;
}
