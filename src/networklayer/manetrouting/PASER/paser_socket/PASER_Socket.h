/**
 *\class       PASER_Socket
 *@brief       Class comprises the <b>handleMessage</b> function that is called upon receipt of a message by the PASER module.
 *@ingroup     Socket
 *\authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
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
#include "Configuration.h"
#ifdef OPENSSL_IS_LINKED
#define OMNETPP

class PASER_Socket;

//#ifdef PASER_SOCKET_H_
//
//
//#else

#ifndef PASER_SOCKET_H_
#define PASER_SOCKET_H_
#include "PASER_MSG.h"
#include "PASER_TU_RREP.h"
#include "PASER_TU_RREP_ACK.h"
#include "PASER_TU_RREQ.h"
#include "PASER_UB_RREQ.h"
#include "PASER_UU_RREP.h"
#include "PASER_Routing_Table.h"
#include "PASER_Routing_Entry.h"
#include "PASER_Neighbor_Table.h"
#include "PASER_Neighbor_Entry.h"
#include "PASER_RREQ_List.h"
#include "PASER_Timer_Queue.h"
#include "PASER_Timer_Message.h"
#include "PASER_Message_Queue.h"
#include "PASER_Crypto_Root.h"
#include "PASER_Crypto_Sign.h"
#include "PASER_Crypto_Hash.h"
#include "PASER_Message_Processing.h"
#include "PASER_Route_Maintenance.h"
#include "PASER_Route_Discovery.h"
#include "PASER_Global.h"
#include "PASER_Configurations.h"
#include "PASER_Definitions.h"
#include "ManetAddress.h"
#include <omnetpp.h>
#include <Coord.h>
#include <ChannelControl.h>

#include "ManetRoutingBase.h"
#include "IPv4Datagram.h"

#include <list>
#include <compatibility.h>




class PASER_Socket: public ManetRoutingBase {
private:
    PASER_Timer_Queue *timer_queue;
    PASER_Routing_Table *routing_table;
    PASER_Neighbor_Table *neighbor_table;
    PASER_Message_Queue *message_queue;
    PASER_Crypto_Root *root;
    PASER_Crypto_Sign *crypto_sign;
    PASER_Crypto_Hash *crypto_hash;
    PASER_Message_Processing *paket_processing;
    PASER_Route_Discovery *route_findung;
    PASER_Route_Maintenance *route_maintenance;
    PASER_Global *paser_global;
    PASER_Configurations *paser_configuration;

    PASER_RREQ_List *rreq_list; ///< List of IP addresses to which a route discovery is started.
    PASER_RREQ_List *rrep_list; ///< List of IP addresses from which a TU-RREP-ACK is expected.

//    std::list<struct in_addr> AddL;

//    bool isGW;
//    bool isRegistered;
//    bool wasRegistered;

    bool firstRoot;

    network_device *netDevice; ///< Pointer to an Array of wireless cards on which PASER is running

#ifdef OMNETPP
    cMessage * startMessage;
    cMessage * sendMessageEvent;
    cMessage * genRootEvent;

    simtime_t GenRootTimer;
#endif

//    lv_block GTK;

//    Coord myPos;

public:
    InterfaceEntry * PUBLIC_getInterfaceEntry(int index);
    bool isMyLocalAddress(struct in_addr addr);
    int MYgettimeofday(struct timeval *, struct timezone *);
    int MYgetWlanInterfaceIndexByAddress(ManetAddress add);
    int MYgetNumWlanInterfaces();
    int MYgetNumInterfaces();
    void MY_omnet_chg_rte(const ManetAddress &dst, const ManetAddress &gtwy,
            const ManetAddress &netm, short int hops, bool del_entry, int index);
    void resetKernelRoutingTable();
    void MY_sendICMP(cPacket* pkt);

    bool parseIntTo(const char *s, double& destValue);
    double MY_getXPos();
    double MY_getYPos();

    int getIfIdFromIfIndex(int ifIndex);
    void send_message(cPacket * msg, struct in_addr dest_addr,
            u_int32_t ifIndex);

    void startGenNewRoot();
    void GenNewRoot();

    double paketProcessingDelay;
    double data_message_send_total_delay;

protected:
    /**
     * @brief Processing a received message. Check the type of the message
     * and call the corresponding function to handle it
     */
    void handleMessage(cMessage *msg);
    int numInitStages() const {
        return 5;
    }
    void initialize(int stage);

    /**
     * @brief ManetRoutingBase abstract Functions
     */
    bool supportGetRoute () {return false;};
    virtual uint32_t getRoute(const ManetAddress &, std::vector<ManetAddress> &) {
        ev << "computer says no\n";
        return 0;
    }
    ;
    virtual bool getNextHop(const ManetAddress &, ManetAddress &add, int &iface,
            double &val) {
        ev << "computer says no\n";
        return false;
    }
    ;
    virtual void setRefreshRoute(const ManetAddress &destination,
            const ManetAddress & nextHop, bool isReverse) {
    }
    ;
    virtual bool isProactive() {
        return false;
    }
    ;
    virtual bool isOurType(cPacket * msg) {
        if (dynamic_cast<PASER_MSG*>(msg))
            return true;
        else if (dynamic_cast<PASER_MSG*>(msg))
            return true;
        else
            return false;
    }
    ;
    virtual bool getDestAddress(cPacket *, ManetAddress &) {
        return false;
    }
    ;
    virtual void processLinkBreak(const cPolymorphic *details);
    //  ManetRoutingBase abstract Function END

    /**
     * @brief Schedule the next event
     */
    void scheduleNextEvent();

    /**
     * @brief Get ID of the wireless card ID on which the message arrived
     *
     *@param msg Pointer to the message
     */
    u_int32_t getMessageInterface(cPacket * msg);

    /**
     * @brief Send UDP message
     */
    void sendUDPToIp(cPacket *msg, int srcPort, const ManetAddress& destAddr,
            int destPort, int ttl, double delay, int index);

    /**
     * @brief Edit node's color upon registration at a gateway
     */
    void editNodeColor();

    /**
     * @brief Include the length of the message sent into the statistics
     */
    int addPaketLaengeZuStat(cPacket * msg);

public:
    PASER_Socket();
    ~PASER_Socket();
};

#endif /* PASER_SOCKET_H_ */
#endif
