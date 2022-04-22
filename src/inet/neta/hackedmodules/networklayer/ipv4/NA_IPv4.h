//
// Copyright (C) 2013, NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//
// This file is part of NETA.
//
//    NETA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NETA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NETA.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef NA_IPV4_H_
#define NA_IPV4_H_

#include "inet/networklayer/ipv4/IPv4.h"
#include <omnetpp.h>
#include "inet/neta/common/log/NA_NesgLog.h"
#include "inet/neta/hackedmodules/NA_HackedModule.h"
#include "inet/neta/attacks/controlmessages/droppingAttack/NA_DroppingMessage_m.h"
#include "inet/neta/attacks/controlmessages/delayAttack/NA_DelayMessage_m.h"
using namespace omnetpp;

/**
 * Constant to distinguish TCP packet data payload instead of ACK or SYN packets
 */
#define TCP_DATA "tcp"

/**
 * Constant to distinguish UDP packets data.
 */
#define UDP_DATA "UDP"

/**
 * Constant to distinguish PING application data packets.
 */
#define PING_DATA "ping"

/**
 * @brief Dropping attack hacked module
 *
 * @details This hacked module is in charge of implement the dropping behavior on
 * IP layer. When this module receive a dropping control message from the controller
 * this activate or deactivate the dropping behavior. The packets are discarded randomly
 * following a normal distribution with a @verbatim droppingAttackProbability @endverbatim
 * probability.
 *
 * Implemented attacks:
 * - Dropping
 * - Delay
 *
 * @see NA_HackedModule, NA_DroppingAttack
 *
 * @author Gabriel Maciá Fernández, gmacia@ugr.es
 * @date 01/22/2013
 *
 */

namespace inet {

namespace neta {

class NA_IPv4 : public IPv4, public NA_HackedModule {

private:

    /**
     * Log reference
     */
    NA_NesgLog log;

    /*----------------- DROPPING ATTACK  -------------------------*/
    /**
     * Flag to activate dropping attack
     */
    bool droppingAttackIsActive;

    /**
     * Probability for dropping packets when dropping attack is active
     */
    double droppingAttackProbability;

    /**
     * Count the number of packet discarded. For dropping statistics
     */
    long numDrops;

    /*----------------- DELAY ATTACK  -------------------------*/
    /**
     * Flag to activate delay attack
     */
    bool delayAttackIsActive;

    /**
     * Probability for delaying a packet
     */
    double delayAttackProbability;

    /**
     * Number of packets delayed. For statistics
     */
    long numDelays;

    /**
     * Accumulated delay time for all the packets delayed. For statistics
     */
    long totalDelayTime;

    /**
     * elayValue for the simulation. Implemented as a pointer to allow functions as values
     */
    omnetpp::cPar *delayAttackValue;

    /*---------------------OTHERs-------------------------------*/
    /**
     * Number of data packet received by the host.
     */
    long numRecvPacket;

public:

    /**
     * Overridden function
     */
    void handleMessageFromAttackController(cMessage *msg) override;

protected:

    /**
     * Method from cSimpleModule class, to initialize the simple module.
     * Overridden function.
     */
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /*----------------- DROPPING ATTACK  -------------------------*/
    /**
     * Drop signal for statistics
     */
    static simsignal_t dropSignal;

    /**
     * Overridden function to implement the dropping behavior.
     * First check if the dropping behavior is active. Then check if the received packet is
     * a valid packet to drop (PING, UDP and/or TCP). Finally discard it or not randomly.
     */
    virtual void handleIncomingDatagram(IPv4Datagram *datagram, const InterfaceEntry *fromIE) override;

    /*----------------- DELAY ATTACK  -------------------------*/
    /**
     * Overridden function to implement the delay behavior.
     * First check if the delay behavior is active. Then check if the received packet is
     * a valid packet to delay (PING, UDP and/or TCP). Finally add a delay time in a random way.
     */
    virtual simtime_t startService(cPacket *msg) override;

    /**
     * Delay signal for statistics
     */
    static simsignal_t delaySignal;

    /*------------------OTHERs ---------------------------------*/
    /**
     * Packet received signal for statistics.
     */
    static simsignal_t rcvdPktSignal;

};

};
};

#endif /* NA_IPV4_H_ */
