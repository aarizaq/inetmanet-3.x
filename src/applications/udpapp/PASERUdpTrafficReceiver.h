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

#ifndef __INETMANET_PASERUDPTRAFFICRECEIVER_H_
#define __INETMANET_PASERUDPTRAFFICRECEIVER_H_

#include <omnetpp.h>
#include "PaserTrafficDataMsg_m.h"
#include "csimplemodule.h"
#include "UDPSocket.h"
#include "RandomWPMobility.h"

//#define No_init_manetStats



struct mapEntry{
	std::vector<int> receivedCounterVector;
	std::vector<int> expectCounterVector;
	std::vector<int> hopVector;
	std::vector<double> delayVector;
	double pdr;
	double averageNumberOfHops;
	double averageDelay;
	double maxDelay;
    double minDelay;
	int receivedCounterVectorIt;
	int expectedCounterVectorIt;
	int hopVectorIt;
	int delayVectorIt;
};

class  PASERUdpTrafficReceiver : public cSimpleModule
{
  private:
	std::multimap<std::string,mapEntry> senderMap;
	int portNumber;
	cSimulation *currentSim;
	double receivedPacketSize;
	double numberOfTotalSentPackets;
	double numberOfTotalReceivedPackets;
	double numberOfOKReceivedPackets;
	double numberOfTotalDataRate;
	double plotTimer;
	int maxHop;
    int numReceived;
	std::string userFilePath;
	std::string userCollFilePath;
	std::string fileNameBegin;
	std::string PlotFileName;



	cMessage* plotTime;
	UDPSocket socket;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void handleSelfMessage(cMessage *selfMsg);
    void handleExtMessage(cMessage *extMsg);
    void handleTrafficMsg(PaserTrafficDataMsg *trafficMsg);
    void addSenderToMap(std::string senderId);
    bool hasMapEntry(std::string searchId);
    std::multimap<std::string,mapEntry>::iterator findMapEntry(std::string searchId);
    void updateMapEntry(PaserTrafficDataMsg *trafficMsg);
    void calculateAverageValues();
    double calculateAvgHops(mapEntry currentEntry);
    double calculateAvgDelay(mapEntry currentEntry);
    double calculateOkReceivedPackets(mapEntry currentEntry);
    void printLostPackets(mapEntry currentEntry);
    bool isInVector(std::vector<int>vector, int value);
    void finish();
    void savePlotParams();

    void setSocketOptions();

  public:
    int getNumberOfSenders();

    ~PASERUdpTrafficReceiver();
};

#endif
