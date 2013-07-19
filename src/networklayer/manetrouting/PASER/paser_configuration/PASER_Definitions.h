/**
 *\file  		PASER_Definitions.h
 *@brief       Class comprises additional PASER configuration parameters. These should not be changed unless you know what you are doing.
 *@ingroup     Configuration
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
/**
 *\mainpage PASER Documentation
 *\section     Overview This page provides a thorough documentation of the PASER implementation in OMNeT++.
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
 ********************************************************************************
 */

/**
 *  @defgroup Buffer Buffer
 *  @defgroup Configuration Configuration
 *  @defgroup Cryptography Cryptography
 *  @defgroup KDC Key Distribution Center
 *  @defgroup MP Message Processing
 *  @defgroup MS Message Structure
 *  @defgroup RD Route Discovery
 *  @defgroup RM Route Maintenance
 *  @defgroup Socket Socket
 *  @defgroup Tables Tables
 *  @defgroup TM Timer Management
 *
 */

#include "Configuration.h"
#ifdef OPENSSL_IS_LINKED

#ifndef PASER_DEFS_H_
#define PASER_DEFS_H_

#include <openssl/ssl.h>
#include "ManetAddress.h"
#include "compatibility.h"
#include <list>


/** Port number of the PASER UDP socket*/
#define PASER_PORT 653

/// Port number of the KDC TCP socket
#define PASER_PORT_CRL 653

/// Path to nodes certificates
#define PASER_router_cert_file      "cert/router.pem"
#define PASER_router_cert_key_file  "cert/router.key"
#define PASER_gateway_cert_file     "cert/gateway.pem"
#define PASER_gateway_cert_key_file "cert/gateway.key"

/// Path to CA certificate
#define PASER_CA_cert_file          "cert/cacert.pem"

#define PASER_router_old_cert_file      "cert/oldcert.pem"
#define PASER_router_old_cert_key_file  "cert/oldcert.key"
#define PASER_gateway_old_cert_file     "cert/old_gateway.pem"
#define PASER_gateway_old_cert_key_file "cert/old_gateway.key"

/// Maximum length of the PASER signature
#define PASER_sign_len 4096

/// Maximum transmitting range of wireless cards
#define PASER_radius paser_modul->par("PASER_radius").doubleValue()

/// Active route timeouts
#define PASER_ROUTE_DELETE_TIME (double)paser_modul->par("PASER_RouteDeleteTimeOut")/(double)1000
#define PASER_ROUTE_VALID_TIME (double)paser_modul->par("PASER_RouteTimeOut")/(double)1000
#define PASER_NEIGHBOR_DELETE_TIME (double)paser_modul->par("PASER_NeighborDeleteTimeOut")/(double)1000
#define PASER_NEIGHBOR_VALID_TIME (double)paser_modul->par("PASER_NeighborTimeOut")/(double)1000
//#define PASER_ROUTE_DELETE_TIME 30.0
//#define PASER_ROUTE_VALID_TIME 6.0
//#define PASER_NEIGHBOR_DELETE_TIME 20.0
//#define PASER_NEIGHBOR_VALID_TIME 6.0

/// HELLO message sending interval
#define PASER_TB_Hello_Interval (double)paser_modul->par("HELLO_Interval")/(double)1000

/// Maximum wait time (ms) for a route reply
#define PASER_UB_RREQ_WAIT_TIME (double)paser_modul->par("PASER_RREQWaitTime")/(double)1000
/// Maximum wait time (ms) for a route reply acknowledge
#define PASER_UU_RREP_WAIT_TIME (double)paser_modul->par("PASER_RREPWaitTime")/(double)1000

/// Number of RREQ retries
#define PASER_UB_RREQ_TRIES (int)paser_modul->par("PASER_RREQTries")
/// Number of RREP retries
#define PASER_UU_RREP_TRIES (int)paser_modul->par("PASER_RREPTries")

/// Maximum wait time (ms) for a KDC reply
#define PASER_KDC_REQUEST_TIME (double)paser_modul->par("PASER_KDCWaitTime")/(double)1000

/// Delay (ms) of releasing a buffered message
#define PASER_DATA_PACKET_SEND_DELAY (double)paser_modul->par("PASER_Data_Message_Send_Delay")

/// Length of PASER secret
#define PASER_SECRET_LEN 32

/// Number of generated secrets:= 2^secret_parameter
#define PASER_Crypto_Root_param (int)paser_modul->par("secret_parameter")
//#define PASER_Crypto_Root_repeat_timeout 1000
//#define PASER_Crypto_Root_repeat 2

/** Broadcast address (255.255.255.255) */
#define PASER_BROADCAST ManetAddress(IPv4Address::ALLONES_ADDRESS)

#define PASER_SUBNETZ ((in_addr_t) 0x0A000100)
#define PASER_MASK ((in_addr_t) 0xFFFF0000)

#define PASER_time_diff 120

/// How often a RERR message to the same IP address will be sent(ms)
/// Number of RERR retries
#define PASER_rerr_limit 500

/// Enable link layer feedback
#define PASER_linkLayerFeeback paser_modul->par("isLinkLayer").boolValue()

/// Maximum sequence number
#define PASER_MAXSEQ ((u_int32_t)0xFFFFFFFF)

typedef struct {
    double lat;
    double lon;
} geo_pos;

typedef struct {
    unsigned char *buf;
    u_int32_t len;
} lv_block;

typedef struct {
    struct in_addr addr;
    u_int32_t seq;
} unreachableBlock;

typedef struct {
    lv_block GTK;
    u_int32_t nonce;
    lv_block CRL;
    lv_block cert_kdc;
    lv_block sign;
    u_int32_t key_nr;
    lv_block sign_key;
} kdc_block;

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif  /* IFNAMSIZ */

/*
 *A struct containing all necessary information about each
 *interface participating in the PASER routing
 */
typedef struct {
    int enabled; ///< 1 if struct is used, else 0
    int sock; ///< PASER socket associated with this device
    int icmp_sock; ///< Raw socket used to send/receive ICMP messages
    u_int32_t ifindex; ///< Index of this interface
    char ifname[IFNAMSIZ]; ///< Interface name
    struct in_addr ipaddr; ///< Local IP address
    struct in_addr bcast; ///< Broadcast address
    struct in_addr mask; ///< Mask
} network_device;

struct address_range {
    struct in_addr ipaddr; /* The IP address */
    struct in_addr mask; /* mask */

    address_range() {
        ipaddr.S_addr = ManetAddress::ZERO;
        mask.S_addr = ManetAddress::ZERO;
    }
    address_range & operator =(const address_range &other) {
        if (this == &other)
            return *this;
        ipaddr = other.ipaddr;
        mask = other.mask;
        return *this;
    }
};

struct address_list {
    struct in_addr ipaddr; /* The IP address */
    std::list<address_range> range; /* List of the node's subnetworks */

    address_list() {
        ipaddr.S_addr = ManetAddress::ZERO;
    }
    address_list & operator =(const address_list &other) {
        if (this == &other)
            return *this;
        ipaddr = other.ipaddr;
        range.assign(other.range.begin(), other.range.end());
        return *this;
    }
};

//#ifndef _TIMEVAL_DEFINED /* also in winsock[2].h */
//#define _TIMEVAL_DEFINED
//struct timeval {
//  long tv_sec;
//  long tv_usec;
//};
//#endif

#endif /* PASER_DEFS_H_ */
#endif
