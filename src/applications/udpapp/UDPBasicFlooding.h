//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2011 Zoltan Bojthe
// Copyright (C) 2013 Universidad de Malaga
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


#ifndef __INET_UDPBASICFLOODING_H
#define __INET_UDPBASICFLOODING_H

#include <vector>
#include <map>

#include "INETDefs.h"
#include "UDPSocket.h"
#include "AddressModule.h"


/**
 * UDP application. See NED for more info.
 */
class INET_API UDPBasicFlooding : public cSimpleModule
{
  protected:
    AddressModule * addressModule;
    UDPSocket socket;
    int localPort, destPort;

    int destAddrRNG;
    int myId;

    typedef std::map<int,int> SourceSequence;
    SourceSequence sourceSequence;
    simtime_t delayLimit;
    cMessage *timerNext;
    simtime_t stopTime;
    simtime_t nextPkt;
    simtime_t nextBurst;
    simtime_t nextSleep;
    bool activeBurst;
    bool isSource;
    bool haveSleepDuration;
    std::vector<int> outputInterfaceMulticastBroadcast;

    static int counter; // counter for generating a global number for each packet

    int numSent;
    int numReceived;
    int numDeleted;
    int numDuplicated;
    int numFlood;

    // volatile parameters:
    cPar *messageLengthPar;
    cPar *burstDurationPar;
    cPar *sleepDurationPar;
    cPar *sendIntervalPar;

    //statistics:
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t floodPkSignal;
    static simsignal_t outOfOrderPkSignal;
    static simsignal_t dropPkSignal;

    // chooses random destination address
    virtual cPacket *createPacket();
    virtual void processPacket(cPacket *msg);
    virtual void generateBurst();

  protected:
    virtual int numInitStages() const {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual bool sendBroadcast(const IPvXAddress &dest, cPacket *pkt);

  public:
    UDPBasicFlooding();
    virtual ~UDPBasicFlooding();
};

#endif

