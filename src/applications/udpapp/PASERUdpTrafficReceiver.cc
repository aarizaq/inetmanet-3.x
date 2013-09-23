/*
 *\class       PASERUDpTrafficReceiver
 *@brief       Class provides a constant bit rate UDP receiver. It simulates to a large extent an Iperf UDP server.
 *@brief       Results including PDR and delay of the received packets are saved in a file in the results directory.
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

#include "PASERUdpTrafficReceiver.h"
#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTableAccess.h"


#include <fstream>
#include <iostream>

#include <ModuleAccess.h>
#include "compatibility.h"
Define_Module(PASERUdpTrafficReceiver);


PASERUdpTrafficReceiver::~PASERUdpTrafficReceiver(){
    if(plotTime){
        if(plotTime->isScheduled()){
            cancelEvent(plotTime);
        }
        delete plotTime;
    }
}

void PASERUdpTrafficReceiver::initialize()
{

	EV << "Method " << this->getFullPath() << ": initialize() called..." << endl;

	//Get reference of current Simulation
	currentSim = cSimulation::getActiveSimulation();

	numReceived = 0;

    //Initialize parameters
	portNumber = par("port");
	receivedPacketSize=0;
	fileNameBegin = par("fileName").stringValue();
	plotTime = 0;
	plotTimer = par("plotTimer").doubleValue();
	if(plotTimer > 0){
	    plotTime = new cMessage("itsPlotTime");
	    scheduleAt(simTime() + plotTimer, plotTime);
	}

	//Initialize statistical values
	numberOfTotalDataRate=0.0;
	numberOfTotalReceivedPackets=0.0;
	numberOfOKReceivedPackets=0.0;
	numberOfTotalSentPackets=0.0;

	//Define userFilePath for Result-File
#ifndef __unix__
    userFilePath = "results\\";
#else
//	userFilePath = "/home/chebi/diplom/omnet_workspace/PASER_OMNET_v3/results/";
	userFilePath = "results/";
#endif

	//Create plotFile
    std::ostringstream filePath;
    filePath << userFilePath << fileNameBegin << ".txt";
	PlotFileName = filePath.str();

	//Initialize Port
//	bindToPort(portNumber);
	socket.setOutputGate(gate("udpOut"));
    socket.bind(portNumber);
    setSocketOptions();

	//Initialize variable for fetching maximal Hop
	maxHop=0;
	senderMap.clear();


//  //Generierung der Dateien fuer RoutingTable
//	for(int i=0; i<100; i++){
//        std::ostringstream filePath;
//        filePath << userFilePath << "table/host_no_send_ohne_subnetz" << i << ".rt";
//        std::fstream dataFile;
//        dataFile.open(filePath.str().c_str(),std::ios::out | std::ios::app);
//
//        dataFile << "ifconfig:\n"
//                << "name: wlanG0        inet_addr: 10.0.1." << 30+i << " Mask: 255.255.255.0    MTU: 1500   Metric: 1 BROADCAST MULTICAST\n"
//                << "#name: wlan0        inet_addr: 10.0." << 30+i << ".1 Mask: 255.255.255.0    MTU: 1500   Metric: 1 BROADCAST MULTICAST\n"
//                << "ifconfigend.\n"
//                << "\n"
//                << "route:\n"
//                << "#default:   *   0.0.0.0 G   0   lo0\n"
//                << "routeend.\n"
//                << "\n" << endl;
//
//        std::cout << "MaxHop: " << maxHop << std::endl;
//
//        //Close File
//        dataFile.close();
//	}
}

void PASERUdpTrafficReceiver::setSocketOptions()
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

void PASERUdpTrafficReceiver::handleMessage(cMessage *msg)
{
	EV << "Method " << this->getFullPath() << ": handleMessage() called..." << endl;

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

void PASERUdpTrafficReceiver::handleSelfMessage(cMessage *selfMsg)
{
	EV << "Method " << this->getFullPath() << ": handleSelfMsg() called..." << endl;

	if(selfMsg == plotTime){
	    savePlotParams();
	    scheduleAt(simTime() + plotTimer, plotTime);
	    return;
	}

	delete selfMsg;
	return;
}

void PASERUdpTrafficReceiver::handleExtMessage(cMessage *extMsg)
{
	EV << "Method " << this->getFullPath() << ": handleExtMessage() called..." << endl;

	//Check if message is a TrafficMsg
	PaserTrafficDataMsg *trafficMsg = dynamic_cast<PaserTrafficDataMsg*>(extMsg);
	if(extMsg!=NULL){
		//handle trafficMsg
		handleTrafficMsg(trafficMsg);
		numReceived++;
	    if (ev.isGUI())
	    {
	        char buf[40];
	        sprintf(buf, "rcvdTotal: %d pks", numReceived);
	        getDisplayString().setTagArg("t",0,buf);
	    }
		return;
	}

	delete extMsg;
	return;
}

void PASERUdpTrafficReceiver::handleTrafficMsg(PaserTrafficDataMsg *trafficMsg)
{
	EV << "Method " << this->getFullPath() << ": handleTrafficMsg() called..." << endl;



	//Check if entry exists in senderMap
	if(hasMapEntry(trafficMsg->getSrcId())==true){
		//Update entry
		updateMapEntry(trafficMsg);
	}
	else{
		addSenderToMap(trafficMsg->getSrcId());
		updateMapEntry(trafficMsg);
	}

	//save packetSize for GoodPut
	receivedPacketSize=receivedPacketSize+trafficMsg->getBitLength();
	numberOfTotalReceivedPackets = numberOfTotalReceivedPackets + 1;

	if(trafficMsg->getHops()>maxHop){
		maxHop=trafficMsg->getHops();
	}

	//Delete message
	delete trafficMsg;

	return;
}

void PASERUdpTrafficReceiver::addSenderToMap(std::string senderId)
{
	EV << "Method " << this->getFullPath() << ": addSenderToMap() called..." << endl;

	//Check if senderId does not have an entry
	if(hasMapEntry(senderId)==true){
		error("Error! Sender already has a map entry. Please use Update-Function");
		return;
	}

	//Create new Entry with empty values
	mapEntry newEntry;
	newEntry.pdr=0;
	newEntry.averageNumberOfHops=0;
	newEntry.averageDelay=0;
	newEntry.receivedCounterVectorIt=0;
	newEntry.expectedCounterVectorIt=0;
	newEntry.hopVectorIt=0;
	newEntry.delayVectorIt=0;
	newEntry.maxDelay=0;
	newEntry.minDelay=9999;

	//Insert into map
	senderMap.insert(std::pair<std::string,mapEntry>(senderId,newEntry));

	return;
}

void PASERUdpTrafficReceiver::updateMapEntry(PaserTrafficDataMsg *trafficMsg)
{
	EV << "Method " << this->getFullPath() << ": updateMapEntry() called..." << endl;

	//Initialize iterator
	std::multimap<std::string,mapEntry>::iterator it;

	//check if map entry exists
	if(hasMapEntry(trafficMsg->getSrcId())==false){
		error("Map entry does not exist. Please use addSenderToMap() Method!");
		return;
	}

	//Get information of trafficMsg and update mapEntry
	it=findMapEntry(trafficMsg->getSrcId());

	if(it==senderMap.end()){
		error("Cannot find entry in table!");
	}

	//Update informations of entry
	mapEntry currentEntry = (*it).second;
	currentEntry.receivedCounterVector.insert(currentEntry.receivedCounterVector.end(),trafficMsg->getCounter());
	int expectedValue=currentEntry.expectCounterVector.size();
	currentEntry.expectCounterVector.insert(currentEntry.expectCounterVector.end(),expectedValue);
	currentEntry.hopVector.insert(currentEntry.hopVector.end(),trafficMsg->getHops());
	double packetDelay = (simTime().dbl()-trafficMsg->getTimestamp());
	ev << "packetDelay: " << packetDelay << "\n";
	currentEntry.delayVector.insert(currentEntry.delayVector.end(),packetDelay);
	if(currentEntry.maxDelay < packetDelay){
	    currentEntry.maxDelay = packetDelay;
	}
	if(currentEntry.minDelay > packetDelay){
	    currentEntry.minDelay = packetDelay;
	}

	//Save entry
	(*it).second=currentEntry;

	return;
}

bool PASERUdpTrafficReceiver::hasMapEntry(std::string searchId)
{
	EV<< "Method " << this->getFullPath() << ": hasMapEntry() called..." << endl;

	//Initialize iterator
	std::multimap<std::string,mapEntry>::iterator it;

	for(it=senderMap.begin();it!=senderMap.end();it++){
		if(strcmp((*it).first.c_str(),searchId.c_str())==0){
			return true;
		}
	}

	//No entry exist
	return false;
}

std::multimap<std::string,mapEntry>::iterator PASERUdpTrafficReceiver::findMapEntry(std::string searchId)
{
	EV<< "Method " << this->getFullPath() << ": findMapEntry() called..." << endl;

	//Initialize iterator
	std::multimap<std::string,mapEntry>::iterator it;

	//Check if map entry exists
	if(hasMapEntry(searchId)==false){
		error("Map Entry does not exist!");
	}

	//find position and return iterator
	it=senderMap.find(searchId);

	if(it==senderMap.end()){
		error("Map Entry does not exist! HUGE ERROR!");
	}

	return it;
}

void PASERUdpTrafficReceiver::calculateAverageValues()
{
    numberOfOKReceivedPackets = 0;
	EV<< "Method " << this->getFullPath() << ": calculateAverageValues() called..." << endl;

	//Initialize iterator
	std::multimap<std::string,mapEntry>::iterator it;

	//Go threw senderMap and calculate values for every entry
	for(it=senderMap.begin();it!=senderMap.end();it++){
		//Fetch entry
		std::string currentId = (*it).first;
		mapEntry currentEntry = (*it).second;
		ev << "Sender: " << currentId << "\n";
		//Calculate OK Received Packet
		numberOfOKReceivedPackets += calculateOkReceivedPackets(currentEntry);
//		printLostPackets(currentEntry);


        currentEntry.pdr=0;

		currentEntry.expectedCounterVectorIt=currentEntry.expectCounterVector.size();

		//Calculate Average Hops
//		currentEntry.averageNumberOfHops=calculateAvgHops(currentEntry);
		currentEntry.averageNumberOfHops=1;

		//Calculate Average Delay
		currentEntry.averageDelay=calculateAvgDelay(currentEntry);


		//Save new values to table
		(*it).second=currentEntry;
	}

	return;
}

void PASERUdpTrafficReceiver::printLostPackets(mapEntry currentEntry)
{
	EV << "Method " << this->getFullPath() << ": printLostPackets() called..." << endl;

	for(u_int32_t i=0;i<currentEntry.receivedCounterVector.size();i++){
	    int temp = i;
	    bool found = false;
	    for(u_int32_t j=0; j<currentEntry.receivedCounterVector.size(); j++){
	        if(currentEntry.receivedCounterVector.at(j) == temp){
	            found = true;
	            break;
	        }
	    }
	    if(!found){
	        EV << "lostPaketID: " << temp << "\n";
	    }
	}
}

double PASERUdpTrafficReceiver::calculateOkReceivedPackets(mapEntry currentEntry)
{
	EV << "Method " << this->getFullPath() << ": calculateOkReceivedPackets() called..." << endl;

	//Initialize number-values
	double numOfOkPackets=0;

	for(u_int32_t i=0;i<currentEntry.receivedCounterVector.size();i++){
	    int temp = currentEntry.receivedCounterVector.at(i);
	    bool found = false;
	    for(u_int32_t j=i+1; j<currentEntry.receivedCounterVector.size(); j++){
	        if(currentEntry.receivedCounterVector.at(j) == temp){
	            found = true;
	            break;
	        }
	        if(currentEntry.receivedCounterVector.at(j) > (temp+1000)){
	            break;
	        }
	    }
	    if(!found){
	        numOfOkPackets++;
	    }
	}

	return (numOfOkPackets);
}


bool PASERUdpTrafficReceiver::isInVector(std::vector<int>vector, int value){

	for(u_int32_t i=0;i<vector.size();i++){
		if(vector.at(i)==value){
			return true;
		}
	}
	return false;
}

double PASERUdpTrafficReceiver::calculateAvgHops(mapEntry currentEntry)
{
	EV << "Method " << this->getFullPath() << ": calculateAvgHops() called..." << endl;

	//Initialize number-values
	double numOfEntries=0;
	double hopSum=0;

	//Go threw list and calculate new avgHops
	//for(currentEntry.hopVectorIt;currentEntry.hopVectorIt<currentEntry.hopVector.size();currentEntry.hopVectorIt++){
    for(;currentEntry.hopVectorIt<(int)currentEntry.hopVector.size();currentEntry.hopVectorIt++){
		numOfEntries=numOfEntries+1;
		hopSum=hopSum+currentEntry.hopVector.at(currentEntry.hopVectorIt);
	}

	//return new average number of hops
	return (hopSum/numOfEntries);
}

double PASERUdpTrafficReceiver::calculateAvgDelay(mapEntry currentEntry)
{
	EV << "Method " << this->getFullPath() << ": calculateAvgDelay() called..." << endl;

	//Initialize number-values
	double numOfEntries=0;
	double delaySum=0;

	//Go threw list and calculate new avgDelay
//	for(currentEntry.delayVectorIt;currentEntry.delayVectorIt<currentEntry.delayVector.size();currentEntry.delayVectorIt++){
	for(;currentEntry.delayVectorIt<(int)currentEntry.delayVector.size();currentEntry.delayVectorIt++){
		numOfEntries = numOfEntries + 1;
		delaySum = delaySum + currentEntry.delayVector.at(currentEntry.delayVectorIt);
	}

	//return new average delay
	return (delaySum/numOfEntries);
}

int PASERUdpTrafficReceiver::getNumberOfSenders()
{
	EV<< "Method " << this->getFullPath() << ": getNumberOfSenders() called..." << endl;

	return senderMap.size();
}

void PASERUdpTrafficReceiver::finish()
{
	EV<< "Method " << this->getFullPath() << ": finish() called..." << endl;


	//Calculate Average over all values
	double delay=0;
	double hopCount=0;
	double maxDelay=0;
	double minDelay=0;
	std::multimap<std::string,mapEntry>::iterator it;
	calculateAverageValues();
	for(it=senderMap.begin();it!=senderMap.end();it++){
		delay = delay+(*it).second.averageDelay;
		hopCount = hopCount+(*it).second.averageNumberOfHops;
		maxDelay = maxDelay+(*it).second.maxDelay;
		minDelay = minDelay+(*it).second.minDelay;
	}

	//Get Mobility Module for waitTime and speed (mps) (for Filename)
	RandomWPMobility *mobilityModule = dynamic_cast<RandomWPMobility*>(findModuleWhereverInNode("mobility",this));
	std::string waitTime="";
	std::string speed="";
	if(mobilityModule!=NULL){
		waitTime=mobilityModule->par("waitTime").str();
		speed=mobilityModule->par("speed").str();
	}

	std::string NameDesKnotens = this->getParentModule()->getFullName();
	//Add Values to Erg-File
	std::ostringstream filePath;
//	filePath << userFilePath << fileNameBegin << "_" << senderMap.size() << "_" << speed << "-" << waitTime << ".dat";
	filePath << userFilePath << NameDesKnotens << "_" << fileNameBegin << "_" << 1 << "_" << speed << "-" << waitTime << ".dat";

	//Create file (append mode)
	std::fstream dataFile;
	dataFile.open(filePath.str().c_str(),std::ios::out | std::ios::app | std::ios::in);


	numberOfTotalReceivedPackets = numberOfOKReceivedPackets;
//	double pdr = numberOfTotalReceivedPackets/numberOfTotalSentPackets;


	std::string collisionFileName = "coll_";
    std::ostringstream collFilePath;
    collFilePath << userFilePath << collisionFileName << fileNameBegin << "_" << 1 << "_" << speed << "-" << waitTime << ".dat";
    std::fstream collDataFile;
    collDataFile.open(collFilePath.str().c_str(),std::ios::out | std::ios::app | std::ios::in);

    collDataFile.close();

	//Close File
	dataFile.close();
	//delete Statistik
	senderMap.clear();
	return;
}


void PASERUdpTrafficReceiver::savePlotParams()
{
//    if(!isStatsInit){
//        return;
//    }
	EV<< "Method " << this->getFullPath() << ": savePlotParams() called..." << endl;


	//Calculate Average over all values
	double delay=0;
	double hopCount=0;
	double maxDelay=0;
	double minDelay=0;
	std::multimap<std::string,mapEntry>::iterator it;
	calculateAverageValues();
	for(it=senderMap.begin();it!=senderMap.end();it++){
		delay = delay+(*it).second.averageDelay;
		hopCount = hopCount+(*it).second.averageNumberOfHops;
		maxDelay = maxDelay+(*it).second.maxDelay;
		minDelay = minDelay+(*it).second.minDelay;
	}

	//Get Mobility Module for waitTime and speed (mps) (for Filename)
	RandomWPMobility *mobilityModule = dynamic_cast<RandomWPMobility*>(findModuleWhereverInNode("mobility",this));
	std::string waitTime="";
	std::string speed="";
	if(mobilityModule!=NULL){
		waitTime=mobilityModule->par("waitTime").str();
		speed=mobilityModule->par("speed").str();
	}

	//Add Values to Erg-File
	std::ostringstream filePath;
//	filePath << userFilePath << fileNameBegin << "_" << senderMap.size() << "_" << speed << "-" << waitTime << ".dat";
	filePath << userFilePath << fileNameBegin << "_" << 1 << "_" << speed << "-" << waitTime << (int)floor(simTime().dbl()) <<".dat";

	//Create file (append mode)
	std::fstream dataFile;
	dataFile.open(filePath.str().c_str(),std::ios::out | std::ios::app | std::ios::in);


	numberOfTotalReceivedPackets = numberOfOKReceivedPackets;
//	double pdr = numberOfTotalReceivedPackets/numberOfTotalSentPackets;



	std::string collisionFileName = "coll_";
    std::ostringstream collFilePath;
    collFilePath << userFilePath << collisionFileName << fileNameBegin << "_" << 1 << "_" << speed << "-" << waitTime << (int)floor(simTime().dbl()) << ".dat";
    std::fstream collDataFile;
    collDataFile.open(collFilePath.str().c_str(),std::ios::out | std::ios::app | std::ios::in);
    collDataFile.close();



	//Close File
	dataFile.close();
	//delete Statistik
//	manetStats::destroy();
//	isStatsInit = false;
//	senderMap.clear();
	return;
}
