/*
 *\class       PASER_UB_RREQ
 *@brief       Class is an implementation of PASER_UB_RREQ messages
 *
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

#include <openssl/sha.h>
#include <openssl/x509.h>
#include "PASER_UB_RREQ.h"

PASER_UB_RREQ::PASER_UB_RREQ(const PASER_UB_RREQ &m) {
    setName(m.getName());
    operator=(m);
}


PASER_UB_RREQ::PASER_UB_RREQ(struct in_addr src, struct in_addr dest,
        u_int32_t seqNr) {
    type = UB_RREQ;
    srcAddress_var = src;
    destAddress_var = dest;
    seq = seqNr;

    keyNr = 0;

    sign.buf = NULL;
    sign.len = 0;

    timestamp = 0;
}


PASER_UB_RREQ::~PASER_UB_RREQ() {
//    X509 *x;
    if (GFlag) {
//        x = (X509*) cert.buf;
        free(cert.buf);
    }
//    x = (X509*) certForw.buf;
    free(certForw.buf);
    free(root);
    free(sign.buf);
}


PASER_UB_RREQ& PASER_UB_RREQ::operator =(const PASER_UB_RREQ &m) {
    if (this == &m)
        return *this;
    cPacket::operator=(m);

    // PASER_MSG
    type = m.type;
    srcAddress_var.S_addr = m.srcAddress_var.S_addr;
    destAddress_var.S_addr = m.destAddress_var.S_addr;
    seq = m.seq;
    keyNr = m.keyNr;
    seqForw = m.seqForw;

    // PASER_UB_RREQ
    GFlag = m.GFlag;
//    AddressRangeList.assign( m.AddressRangeList.begin(), m.AddressRangeList.end() );
    std::list<address_list> tempList(m.AddressRangeList);
    for (std::list<address_list>::iterator it = tempList.begin();
            it != tempList.end(); it++) {
        AddressRangeList.push_back((address_list) *it);
    }
//    routeFromQueryingToForwarding.assign( m.routeFromQueryingToForwarding.begin(), m.routeFromQueryingToForwarding.end() );
    metricBetweenQueryingAndForw = m.metricBetweenQueryingAndForw;

    if (GFlag) {
        //nonce
        nonce = m.nonce;
        //tlv_block cert;
        cert.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.cert.len));
        memcpy(cert.buf, m.cert.buf, (sizeof(u_int8_t) * m.cert.len));
        cert.len = m.cert.len;
    }
//    tlv_block certForw;
    certForw.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.certForw.len));
    memcpy(certForw.buf, m.certForw.buf, (sizeof(u_int8_t) * m.certForw.len));
    certForw.len = m.certForw.len;

    root = (u_int8_t *) malloc((sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(root, m.root, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));

    initVector = m.initVector;
//    std::list<u_int8_t *>temp (m.initVector);
//    for(std::list<u_int8_t *>::iterator it=temp.begin(); it!=temp.end(); ++it){
//        u_int8_t *buf = (u_int8_t *)malloc((sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
//        memcpy(buf, (u_int8_t *)*it, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
//        initVector.push_back(buf);
//    }

    geoQuerying.lat = m.geoQuerying.lat;
    geoQuerying.lon = m.geoQuerying.lon;
    geoForwarding.lat = m.geoForwarding.lat;
    geoForwarding.lon = m.geoForwarding.lon;

    sign.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.sign.len));
    memcpy(sign.buf, m.sign.buf, (sizeof(u_int8_t) * m.sign.len));
    sign.len = m.sign.len;
//    sign = (u_int8_t *)malloc((sizeof(u_int8_t) * PASER_sign_len));
//    memcpy(sign, m.sign, (sizeof(u_int8_t) * PASER_sign_len));

    timestamp = m.timestamp;

    return *this;
}


std::string PASER_UB_RREQ::detailedInfo() const {
    std::stringstream out;
    out << "Type: UBRREQ \n";
    out << "Querying node: " << srcAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Destination node: " << destAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Sequence: " << seq << "\n";
    out << "KeyNr: " << keyNr << "\n";
    out << "GFlag: " << (u_int32_t) GFlag << "\n";
    out << "AddL.size: " << AddressRangeList.size() << "\n";
    std::list<address_list> temp;
    temp.assign(AddressRangeList.begin(), AddressRangeList.end());
    for (std::list<address_list>::iterator it = temp.begin(); it != temp.end();
            it++) {
        address_list tempList = (address_list) *it;
        out << "Route: " << tempList.ipaddr.S_addr.getIPv4().str() << "\n";

        std::list<address_range> temp2;
        temp2.assign(tempList.range.begin(), tempList.range.end());
        for (std::list<address_range>::iterator it = temp2.begin();
                it != temp2.end(); it++) {
            address_range tempRange = (address_range) *it;
            out << "Range: " << tempRange.ipaddr.S_addr.getIPv4().str()
                    << "\n";
        }
    }
//    out << "Route.size: "<< routeFromQueryingToForwarding.size() << "\n";
//    std::list<struct in_addr> temp;
//    temp.assign(routeFromQueryingToForwarding.begin(), routeFromQueryingToForwarding.end());
//    for(std::list<struct in_addr>::iterator it=temp.begin(); it!=temp.end(); it++){
//        struct in_addr temp = (struct in_addr)*it;
//        out << "route: " << temp.S_addr.getIPv4().str() << "\n";
//    }
    out << "metric.size: " << (int32_t) metricBetweenQueryingAndForw << " .\n";
    out << "initVector: " << (int32_t) initVector << " .\n";
    out << "x: " << (int) geoForwarding.lat << "\n";
    out << "y: " << (int) geoForwarding.lon << "\n";
    return out.str();
}

/**
 * Creates and return an array of all fields that must be secured with hash or signature
 *
 *@param l length of created array
 *@return message array
 */
u_int8_t * PASER_UB_RREQ::toByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(destAddress_var.S_addr);
    len += sizeof(seq);
    len += sizeof(keyNr);
    len += sizeof(seqForw);

    len += sizeof(GFlag);
    len += sizeof(len); // groesse der Adl
    for (std::list<address_list>::iterator it = AddressRangeList.begin();
            it != AddressRangeList.end(); it++) {
        address_list temp = (address_list) *it;
        len += sizeof(temp.ipaddr.S_addr);
        len += sizeof(len); // groesse der add_r
        for (std::list<address_range>::iterator it2 = temp.range.begin();
                it2 != temp.range.end(); it2++) {
            struct in_addr temp_addr;
            temp_addr.S_addr = ((address_range) *it2).ipaddr.S_addr;
            len += sizeof(temp_addr.S_addr);
            temp_addr.S_addr = ((address_range) *it2).mask.S_addr;
            len += sizeof(temp_addr.S_addr);
        }
    }
    len += sizeof(metricBetweenQueryingAndForw);
    if (GFlag) {
        len += sizeof(nonce); //nonce
        len += sizeof(cert.len);
        len += cert.len;
    }
    len += sizeof(certForw.len);
    len += certForw.len;
    len += 32;
    len += sizeof(initVector);
    len += sizeof(geoQuerying.lat);
    len += sizeof(geoQuerying.lon);
    len += sizeof(geoForwarding.lat);
    len += sizeof(geoForwarding.lon);
    len += sizeof(timestamp);

    //messageType
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    memset(data, 0xEE, len);
    buf = data;

    //********************************  Comment this section due to signature errors in the inet2.0 version for unkown reasons.********************************/
      /**
    //messageType
    data[0] = 0x00;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    //Dest node
    memcpy(buf, &destAddress_var.S_addr,
            sizeof(destAddress_var.S_addr));
    buf += sizeof(destAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);
    // Key number
    memcpy(buf, (u_int8_t *) &keyNr, sizeof(keyNr));
    buf += sizeof(keyNr);

    // Forwarding Sequence number
    memcpy(buf, (u_int8_t *) &seqForw, sizeof(seqForw));
    buf += sizeof(seqForw);
    // GFlag
    if (GFlag)
        buf[0] = 0x01;
    else
        buf[0] = 0x00;
    buf += sizeof(GFlag);
    // AddL
    // Groesse der ADL
    int tempListSize = AddressRangeList.size();
    memcpy(buf, (u_int8_t *) &tempListSize, sizeof(tempListSize));
    buf += sizeof(tempListSize);
    for (std::list<address_list>::iterator it = AddressRangeList.begin();
            it != AddressRangeList.end(); it++) {
        address_list temp = (address_list) *it;
        memcpy(buf, &temp.ipaddr.S_addr,
                sizeof(temp.ipaddr.S_addr));
        buf += sizeof(temp.ipaddr.S_addr);
        // Groesse der address_range
        int tempAdd = temp.range.size();
        memcpy(buf, (u_int8_t *) &tempAdd, sizeof(tempAdd));
        buf += sizeof(tempAdd);
        for (std::list<address_range>::iterator it2 = temp.range.begin();
                it2 != temp.range.end(); it2++) {
            struct in_addr temp_addr;
            temp_addr.S_addr = ((address_range) *it2).ipaddr.S_addr;
            memcpy(buf, &temp_addr.S_addr,
                    sizeof(temp_addr.S_addr));
            buf += sizeof(temp_addr.S_addr);
            temp_addr.S_addr = ((address_range) *it2).mask.S_addr;
            memcpy(buf, &temp_addr.S_addr,
                    sizeof(temp_addr.S_addr));
            buf += sizeof(temp_addr.S_addr);
        }
    }
    // Metric
    memcpy(buf, (u_int8_t *) &metricBetweenQueryingAndForw,
            sizeof(metricBetweenQueryingAndForw));
    buf += sizeof(metricBetweenQueryingAndForw);
    // Cert of querying node
    if (GFlag) {
        memcpy(buf, (u_int8_t *) &nonce, sizeof(nonce));
        buf += sizeof(nonce);
        memcpy(buf, (u_int8_t *) &cert.len, sizeof(cert.len));
        buf += sizeof(cert.len);
        memcpy(buf, cert.buf, cert.len);
        buf += cert.len;
    }
    // Cert of forwarding node
    memcpy(buf, (u_int8_t *) &certForw.len, sizeof(certForw.len));
    buf += sizeof(certForw.len);
    memcpy(buf, certForw.buf, certForw.len);
    buf += certForw.len;
    // root
    memcpy(buf, root, 32);
    buf += 32;
    // IV
    memcpy(buf, (u_int8_t *) &initVector, sizeof(initVector));
    buf += sizeof(initVector);
    // GEO of querying node
  memcpy(buf, (u_int8_t *) &geoQuerying.lat, sizeof(geoQuerying.lat));
  buf += sizeof(geoQuerying.lat);
  memcpy(buf, (u_int8_t *) &geoQuerying.lon, sizeof(geoQuerying.lon));
  buf += sizeof(geoQuerying.lon);
  // GEO of forwarding node
  memcpy(buf, (u_int8_t *) &geoForwarding.lat, sizeof(geoForwarding.lat));
  buf += sizeof(geoForwarding.lat);
    memcpy(buf, (u_int8_t *) &geoForwarding.lon, sizeof(geoForwarding.lon));
    buf += sizeof(geoForwarding.lon);
    //timestamp
    memcpy(buf, (u_int8_t *) &timestamp, sizeof(timestamp));
    buf += sizeof(timestamp);*/

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}

/**
 * Creates and return an array of all fields of the package
 *
 *@param l length of created array
 *@return message array
 */
u_int8_t * PASER_UB_RREQ::getCompleteByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(destAddress_var.S_addr);
    len += sizeof(seq);
    len += sizeof(keyNr);
    len += sizeof(seqForw);

    len += sizeof(GFlag);
    len += sizeof(len); // groesse der Adl
    int tempLen = len;
    for (std::list<address_list>::iterator it = AddressRangeList.begin();
            it != AddressRangeList.end(); it++) {
        address_list temp = (address_list) *it;
        len += sizeof(temp.ipaddr.S_addr);
        len += sizeof(len); // groesse der add_r
        for (std::list<address_range>::iterator it2 = temp.range.begin();
                it2 != temp.range.end(); it2++) {
            struct in_addr temp_addr;
            temp_addr.S_addr = ((address_range) *it2).ipaddr.S_addr;
            len += sizeof(temp_addr.S_addr);
            temp_addr.S_addr = ((address_range) *it2).mask.S_addr;
            len += sizeof(temp_addr.S_addr);
        }
    }
    EV << "AddL = " << (len - tempLen) << "\n";
//    for (std::list<struct in_addr>::iterator it=routeFromQueryingToForwarding.begin(); it!=routeFromQueryingToForwarding.end(); it++){
//        struct in_addr temp = (struct in_addr)*it;
//        len += sizeof(temp.S_addr);
//    }
    len += sizeof(metricBetweenQueryingAndForw);
    if (GFlag) {
        len += sizeof(nonce); //nonce
        len += sizeof(cert.len);
        len += cert.len;
    }
    len += sizeof(certForw.len);
    len += certForw.len;
    len += 32;
    len += sizeof(initVector);
    len += sizeof(geoQuerying.lat);
    len += sizeof(geoQuerying.lon);
    len += sizeof(geoForwarding.lat);
    len += sizeof(geoForwarding.lon);
    len += sizeof(timestamp);

    len += sizeof(sign.len);
    len += sign.len;
    //messageType
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    buf = data;
    //messageType
    data[0] = 0x00;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    //Dest node
    memcpy(buf, &destAddress_var.S_addr,
            sizeof(destAddress_var.S_addr));
    buf += sizeof(destAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);
    // Key number
    memcpy(buf, (u_int8_t *) &keyNr, sizeof(keyNr));
    buf += sizeof(keyNr);

    // Forwarding Sequence number
    memcpy(buf, (u_int8_t *) &seqForw, sizeof(seqForw));
    buf += sizeof(seqForw);
    // GFlag
    if (GFlag)
        buf[0] = 0x01;
    else
        buf[0] = 0x00;
    buf += sizeof(GFlag);
    // AddL
    // Groesse der ADL
    int tempListSize = AddressRangeList.size();
    memcpy(buf, (u_int8_t *) &tempListSize, sizeof(tempListSize));
    buf += sizeof(tempListSize);
    for (std::list<address_list>::iterator it = AddressRangeList.begin();
            it != AddressRangeList.end(); it++) {
        address_list temp = (address_list) *it;
        memcpy(buf, &temp.ipaddr.S_addr,
                sizeof(temp.ipaddr.S_addr));
        buf += sizeof(temp.ipaddr.S_addr);
        // Groesse der address_range
        int tempAdd = temp.range.size();
        memcpy(buf, (u_int8_t *) &tempAdd, sizeof(tempAdd));
        buf += sizeof(tempAdd);
        for (std::list<address_range>::iterator it2 = temp.range.begin();
                it2 != temp.range.end(); it2++) {
            struct in_addr temp_addr;
            temp_addr.S_addr = ((address_range) *it2).ipaddr.S_addr;
            memcpy(buf, &temp_addr.S_addr,
                    sizeof(temp_addr.S_addr));
            buf += sizeof(temp_addr.S_addr);
            temp_addr.S_addr = ((address_range) *it2).mask.S_addr;
            memcpy(buf, &temp_addr.S_addr,
                    sizeof(temp_addr.S_addr));
            buf += sizeof(temp_addr.S_addr);
        }
    }
    // Metric
    memcpy(buf, (u_int8_t *) &metricBetweenQueryingAndForw,
            sizeof(metricBetweenQueryingAndForw));
    buf += sizeof(metricBetweenQueryingAndForw);
    // Cert of querying node
    if (GFlag) {
        memcpy(buf, (u_int8_t *) &nonce, sizeof(nonce));
        buf += sizeof(nonce);
        memcpy(buf, (u_int8_t *) &cert.len, sizeof(cert.len));
        buf += sizeof(cert.len);
        memcpy(buf, cert.buf, cert.len);
        buf += cert.len;
        EV << "Certifikatlenge (querying) = " << cert.len << "\n";
    }
    // Cert of forwarding node
    memcpy(buf, (u_int8_t *) &certForw.len, sizeof(certForw.len));
    buf += sizeof(certForw.len);
    memcpy(buf, certForw.buf, certForw.len);
    buf += certForw.len;
    EV << "Certifikatlenge (forwarding) = " << certForw.len << "\n";
    // root
    memcpy(buf, root, 32);
    buf += 32;
    // IV
    memcpy(buf, (u_int8_t *) &initVector, sizeof(initVector));
    buf += sizeof(initVector);
    // GEO of querying node
    memcpy(buf, (u_int8_t *) &geoQuerying.lat, sizeof(geoQuerying.lat));
    buf += sizeof(geoQuerying.lat);
    memcpy(buf, (u_int8_t *) &geoQuerying.lon, sizeof(geoQuerying.lon));
    buf += sizeof(geoQuerying.lon);
    // GEO of forwarding node
    memcpy(buf, (u_int8_t *) &geoForwarding.lat, sizeof(geoForwarding.lat));
    buf += sizeof(geoForwarding.lat);
    memcpy(buf, (u_int8_t *) &geoForwarding.lon, sizeof(geoForwarding.lon));
    buf += sizeof(geoForwarding.lon);
    //timestamp
    memcpy(buf, (u_int8_t *) &timestamp, sizeof(timestamp));
    buf += sizeof(timestamp);

    //sign
    memcpy(buf, (u_int8_t *) &sign.len, sizeof(sign.len));
    buf += sizeof(sign.len);
    memcpy(buf, sign.buf, sign.len);
    buf += sign.len;
    EV << "signlenge = " << sign.len << "\n";

    *l = len;
    EV << "Paketlenge = " << len << "\n";
    return data;
}
#endif
