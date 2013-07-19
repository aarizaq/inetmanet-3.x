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

#ifndef __INETMANET_PASERUDPTRAFFICSENDER_H_
#define __INETMANET_PASERUDPTRAFFICSENDER_H_

#include <omnetpp.h>
#include "csimplemodule.h"
#include "UDPSocket.h"
#include "PaserTrafficDataMsg_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "InterfaceEntry.h"
#include "IPv4InterfaceData.h"
#include "IPvXAddress.h"

//#define No_init_manetStats



class PASERUdpTrafficSender : public cSimpleModule
{
private:
    IPvXAddress destAddr;

	double bitRate;
	double messageFreq;
	double packetLength;
	double offset;
	double stopTime;
	int myCounter;
	int myPort;
	int destPort;
	int interfaceId;
	int numSent;
	bool runMe;
	std::string myId;
	IInterfaceTable *ift;



	UDPSocket socket;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void handleSelfMessage(cMessage *selfMsg);
    void handleExtMessage(cMessage *extMsg);
    void startDataMessage();
    PaserTrafficDataMsg *createDataMessage();
    void sendDataMessage(PaserTrafficDataMsg *dataMsg, std::string destId);
    InterfaceEntry* getInterface();

    void setSocketOptions();
  public:
    int getNumberOfSentMessages();
    double getDataRate();
    std::string getMyId();
};

#endif
