/*
 *\class       PASER_UU_RREP
 *@brief       Class is an implementation of PASER_UU_RREP messages
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
#include "PASER_UU_RREP.h"
#include <openssl/sha.h>
#include <openssl/x509.h>

PASER_UU_RREP::PASER_UU_RREP(const PASER_UU_RREP &m) {
    setName(m.getName());
    operator=(m);
}

PASER_UU_RREP::PASER_UU_RREP(struct in_addr src, struct in_addr dest,
        u_int32_t seqNr) {
    type = UU_RREP;
    srcAddress_var = src;
    destAddress_var = dest;
    seq = seqNr;

    keyNr = 0;

    sign.buf = NULL;
    sign.len = 0;
}

/**
 * Destructor
 */
PASER_UU_RREP::~PASER_UU_RREP() {
    if (GFlag) {
//        free(groupTransientKey.buf);
        free(kdc_data.GTK.buf);
        free(kdc_data.CRL.buf);
        free(kdc_data.cert_kdc.buf);
        free(kdc_data.sign.buf);
        free(kdc_data.sign_key.buf);
    }
//    X509 *x =(X509 *)certForw.buf;
    free(certForw.buf);
    free(root);
    free(sign.buf);
}

PASER_UU_RREP& PASER_UU_RREP::operator =(const PASER_UU_RREP &m) {
    if (this == &m)
        return *this;
    cPacket::operator=(m);

    // PASER_MSG
    type = m.type;
    srcAddress_var.S_addr = m.srcAddress_var.S_addr;
    destAddress_var.S_addr = m.destAddress_var.S_addr;
    seq = m.seq;

    keyNr = m.keyNr;

    // PASER_UU_RREP
    GFlag = m.GFlag;
//    AddressRangeList.assign( m.AddressRangeList.begin(), m.AddressRangeList.end() );
    std::list<address_list> tempList(m.AddressRangeList);
    for (std::list<address_list>::iterator it = tempList.begin();
            it != tempList.end(); it++) {
        AddressRangeList.push_back((address_list) *it);
    }
//    routeFromQueryingToForwarding.assign( m.routeFromQueryingToForwarding.begin(), m.routeFromQueryingToForwarding.end() );
    metricBetweenQueryingAndForw = m.metricBetweenQueryingAndForw;
    metricBetweenDestAndForw = m.metricBetweenDestAndForw;

//    tlv_block certForw;
    certForw.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.certForw.len));
    memcpy(certForw.buf, m.certForw.buf, (sizeof(u_int8_t) * m.certForw.len));
    certForw.len = m.certForw.len;

    root = (u_int8_t *) malloc((sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(root, m.root, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));

    initVector = m.initVector;

    geoDestination.lat = m.geoDestination.lat;
    geoDestination.lon = m.geoDestination.lon;
    geoForwarding.lat = m.geoForwarding.lat;
    geoForwarding.lon = m.geoForwarding.lon;

    if (GFlag) {
//        groupTransientKey.buf = (u_int8_t *)malloc((sizeof(u_int8_t) * m.groupTransientKey.len));
//        memcpy(groupTransientKey.buf, m.groupTransientKey.buf, (sizeof(u_int8_t) * m.groupTransientKey.len));
//        groupTransientKey.len = m.groupTransientKey.len;
        kdc_data.GTK.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * m.kdc_data.GTK.len));
        memcpy(kdc_data.GTK.buf, m.kdc_data.GTK.buf,
                (sizeof(u_int8_t) * m.kdc_data.GTK.len));
        kdc_data.GTK.len = m.kdc_data.GTK.len;

        kdc_data.nonce = m.kdc_data.nonce;

        kdc_data.CRL.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * m.kdc_data.CRL.len));
        memcpy(kdc_data.CRL.buf, m.kdc_data.CRL.buf,
                (sizeof(u_int8_t) * m.kdc_data.CRL.len));
        kdc_data.CRL.len = m.kdc_data.CRL.len;

        kdc_data.cert_kdc.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * m.kdc_data.cert_kdc.len));
        memcpy(kdc_data.cert_kdc.buf, m.kdc_data.cert_kdc.buf,
                (sizeof(u_int8_t) * m.kdc_data.cert_kdc.len));
        kdc_data.cert_kdc.len = m.kdc_data.cert_kdc.len;

        kdc_data.sign.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * m.kdc_data.sign.len));
        memcpy(kdc_data.sign.buf, m.kdc_data.sign.buf,
                (sizeof(u_int8_t) * m.kdc_data.sign.len));
        kdc_data.sign.len = m.kdc_data.sign.len;

        kdc_data.key_nr = m.kdc_data.key_nr;

        kdc_data.sign_key.buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * m.kdc_data.sign_key.len));
        memcpy(kdc_data.sign_key.buf, m.kdc_data.sign_key.buf,
                (sizeof(u_int8_t) * m.kdc_data.sign_key.len));
        kdc_data.sign_key.len = m.kdc_data.sign_key.len;
    }

    sign.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.sign.len));
    memcpy(sign.buf, m.sign.buf, (sizeof(u_int8_t) * m.sign.len));
    sign.len = m.sign.len;

    timestamp = m.timestamp;

    return *this;
}

std::string PASER_UU_RREP::detailedInfo() const {
    std::stringstream out;
    out << "Type : UURREP \n";
    out << "Querying node : " << srcAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Destination node : " << destAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Sequence : " << seq << "\n";
    out << "KeyNr : " << keyNr << "\n";
    out << "GFlag : " << (int32_t) GFlag << "\n";
    out << "AddL.size : " << AddressRangeList.size() << "\n";
//    out << "Route.size : "<< routeFromQueryingToForwarding.size() << "\n";
//    std::list<struct in_addr> temp;
//    temp.assign(routeFromQueryingToForwarding.begin(), routeFromQueryingToForwarding.end());
//    for(std::list<struct in_addr>::iterator it=temp.begin(); it!=temp.end(); it++){
//        struct in_addr temp = (struct in_addr)*it;
//        out << "route: " << temp.S_addr.getIPv4().str() << "\n";
//    }

    std::list<address_list> temp;
    temp.assign(AddressRangeList.begin(), AddressRangeList.end());
    for (std::list<address_list>::iterator it = temp.begin(); it != temp.end();
            it++) {
        address_list tempList = (address_list) *it;
        out << "route: " << tempList.ipaddr.S_addr.getIPv4().str() << "\n";

        std::list<address_range> temp2;
        temp2.assign(tempList.range.begin(), tempList.range.end());
        for (std::list<address_range>::iterator it = temp2.begin();
                it != temp2.end(); it++) {
            address_range tempRange = (address_range) *it;
            out << "range: " << tempRange.ipaddr.S_addr.getIPv4().str()
                    << "\n";
        }
    }
    out << "metric.size : " << metricBetweenQueryingAndForw << "\n";
    return out.str();
}

u_int8_t * PASER_UU_RREP::toByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(destAddress_var.S_addr);
    len += sizeof(seq);
    len += sizeof(keyNr);

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
    len += sizeof(metricBetweenDestAndForw);
    len += sizeof(certForw.len);
    len += certForw.len;
    len += SHA256_DIGEST_LENGTH;
    len += sizeof(initVector);
    len += sizeof(geoDestination.lat);
    len += sizeof(geoDestination.lon);
    len += sizeof(geoForwarding.lat);
    len += sizeof(geoForwarding.lon);
    if (GFlag) {
//        len += sizeof(groupTransientKey.len);
//        len += groupTransientKey.len;
        len += sizeof(kdc_data.GTK.len);
        len += kdc_data.GTK.len;
        len += sizeof(kdc_data.nonce);
        len += sizeof(kdc_data.CRL.len);
        len += kdc_data.CRL.len;
        len += sizeof(kdc_data.cert_kdc.len);
        len += kdc_data.cert_kdc.len;
        len += sizeof(kdc_data.sign.len);
        len += kdc_data.sign.len;
        len += sizeof(kdc_data.key_nr);
        len += sizeof(kdc_data.sign_key.len);
        len += kdc_data.sign_key.len;
    }
    len += sizeof(timestamp);

//    //test
//    len += sizeof(len);

    //messageType
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    memset(data, 0xEE, len);
    buf = data;
  //********************************  Comment this section due to signature errors in the inet2.0 version for unkown reasons.********************************/
   /** //    //messageType
    data[0] = 0x01;
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


    // GFlag
  if (GFlag)
       buf[0] = 0x01;
     else
        buf[0] = 0x00;
 //  buf += sizeof(GFlag);
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
    // Metric
    memcpy(buf, (u_int8_t *) &metricBetweenDestAndForw,
            sizeof(metricBetweenDestAndForw));
    buf += sizeof(metricBetweenDestAndForw);
   // Cert of forwarding node
    memcpy(buf, (u_int8_t *) &certForw.len, sizeof(certForw.len));
    buf += sizeof(certForw.len);
    memcpy(buf, certForw.buf, certForw.len);
    buf += certForw.len;
    // root
    memcpy(buf, root, SHA256_DIGEST_LENGTH);
    buf += SHA256_DIGEST_LENGTH;
    // IV
    memcpy(buf, (u_int8_t *) &initVector, sizeof(initVector));
    buf += sizeof(initVector);
    // GEO of geoDestination node
    memcpy(buf, (u_int8_t *) &geoDestination.lat, sizeof(geoDestination.lat));
    buf += sizeof(geoDestination.lat);
    memcpy(buf, (u_int8_t *) &geoDestination.lon, sizeof(geoDestination.lon));
    buf += sizeof(geoDestination.lon);
    // GEO of forwarding node
    memcpy(buf, (u_int8_t *) &geoForwarding.lat, sizeof(geoForwarding.lat));
    buf += sizeof(geoForwarding.lat);
    memcpy(buf, (u_int8_t *) &geoForwarding.lon, sizeof(geoForwarding.lon));
    buf += sizeof(geoForwarding.lon);
    // GTK
    if (GFlag) {
        //memcpy(buf, (u_int8_t *)&groupTransientKey.len, sizeof(groupTransientKey.len));
        //buf += sizeof(groupTransientKey.len);
        //memcpy(buf, groupTransientKey.buf, groupTransientKey.len);
        //buf += groupTransientKey.len;
        memcpy(buf, (u_int8_t *) &kdc_data.GTK.len, sizeof(kdc_data.GTK.len));
        buf += sizeof(kdc_data.GTK.len);
        memcpy(buf, kdc_data.GTK.buf, kdc_data.GTK.len);
        buf += kdc_data.GTK.len;

        memcpy(buf, (u_int8_t *) &kdc_data.nonce, sizeof(kdc_data.nonce));
        buf += sizeof(kdc_data.nonce);

        memcpy(buf, (u_int8_t *) &kdc_data.CRL.len, sizeof(kdc_data.CRL.len));
        buf += sizeof(kdc_data.CRL.len);
        memcpy(buf, kdc_data.CRL.buf, kdc_data.CRL.len);
        buf += kdc_data.CRL.len;

        memcpy(buf, (u_int8_t *) &kdc_data.cert_kdc.len,
                sizeof(kdc_data.cert_kdc.len));
        buf += sizeof(kdc_data.cert_kdc.len);
        memcpy(buf, kdc_data.cert_kdc.buf, kdc_data.cert_kdc.len);
        buf += kdc_data.cert_kdc.len;

        memcpy(buf, (u_int8_t *) &kdc_data.sign.len, sizeof(kdc_data.sign.len));
        buf += sizeof(kdc_data.sign.len);
        memcpy(buf, kdc_data.sign.buf, kdc_data.sign.len);
        buf += kdc_data.sign.len;

        memcpy(buf, (u_int8_t *) &kdc_data.key_nr, sizeof(kdc_data.key_nr));
        buf += sizeof(kdc_data.key_nr);

        memcpy(buf, (u_int8_t *) &kdc_data.sign_key.len,
                sizeof(kdc_data.sign_key.len));
        buf += sizeof(kdc_data.sign_key.len);
        memcpy(buf, kdc_data.sign_key.buf, kdc_data.sign_key.len);
        buf += kdc_data.sign_key.len;
    }
//    //timestamp
  memcpy(buf, (u_int8_t *) &timestamp, sizeof(timestamp));
    buf += sizeof(timestamp);

////    //test
////    u_int32_t tempAdd = 0xFFFFFFFF;
////    memcpy(buf, (u_int8_t *)&tempAdd, sizeof(tempAdd));
////    buf += sizeof(tempAdd);*/
    *l = len;

    return data;
}

u_int8_t * PASER_UU_RREP::getCompleteByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(destAddress_var.S_addr);
    len += sizeof(seq);
    len += sizeof(keyNr);

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
    len += sizeof(metricBetweenQueryingAndForw);
    len += sizeof(metricBetweenDestAndForw);
    len += sizeof(certForw.len);
    len += certForw.len;
    len += 32;
    len += sizeof(initVector);
    len += sizeof(geoDestination.lat);
    len += sizeof(geoDestination.lon);
    len += sizeof(geoForwarding.lat);
    len += sizeof(geoForwarding.lon);
    if (GFlag) {
//        len += sizeof(groupTransientKey.len);
//        len += groupTransientKey.len;
        len += sizeof(kdc_data.GTK.len);
        len += kdc_data.GTK.len;
        len += sizeof(kdc_data.nonce);
        len += sizeof(kdc_data.CRL.len);
        len += kdc_data.CRL.len;
        len += sizeof(kdc_data.cert_kdc.len);
        len += kdc_data.cert_kdc.len;
        len += sizeof(kdc_data.sign.len);
        len += kdc_data.sign.len;
        len += sizeof(kdc_data.key_nr);
        len += sizeof(kdc_data.sign_key.len);
        len += kdc_data.sign_key.len;
    }
    len += sizeof(timestamp);

    len += sizeof(sign.len);
    len += sign.len;

//    //test
//    len += sizeof(len);
    //messageType
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    buf = data;
    //messageType
    data[0] = 0x01;
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
    // Metric
    memcpy(buf, (u_int8_t *) &metricBetweenDestAndForw,
            sizeof(metricBetweenDestAndForw));
    buf += sizeof(metricBetweenDestAndForw);
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
    // GEO of geoDestination node
    memcpy(buf, (u_int8_t *) &geoDestination.lat, sizeof(geoDestination.lat));
    buf += sizeof(geoDestination.lat);
    memcpy(buf, (u_int8_t *) &geoDestination.lon, sizeof(geoDestination.lon));
    buf += sizeof(geoDestination.lon);
    // GEO of forwarding node
    memcpy(buf, (u_int8_t *) &geoForwarding.lat, sizeof(geoForwarding.lat));
    buf += sizeof(geoForwarding.lat);
    memcpy(buf, (u_int8_t *) &geoForwarding.lon, sizeof(geoForwarding.lon));
    buf += sizeof(geoForwarding.lon);
    // GTK
    if (GFlag) {
//        memcpy(buf, (u_int8_t *)&groupTransientKey.len, sizeof(groupTransientKey.len));
//        buf += sizeof(groupTransientKey.len);
//        memcpy(buf, groupTransientKey.buf, groupTransientKey.len);
//        buf += groupTransientKey.len;
//        EV << "GTK = " << groupTransientKey.len << "\n";
        memcpy(buf, (u_int8_t *) &kdc_data.GTK.len, sizeof(kdc_data.GTK.len));
        buf += sizeof(kdc_data.GTK.len);
        memcpy(buf, kdc_data.GTK.buf, kdc_data.GTK.len);
        buf += kdc_data.GTK.len;

        memcpy(buf, (u_int8_t *) &kdc_data.nonce, sizeof(kdc_data.nonce));
        buf += sizeof(kdc_data.nonce);

        memcpy(buf, (u_int8_t *) &kdc_data.CRL.len, sizeof(kdc_data.CRL.len));
        buf += sizeof(kdc_data.CRL.len);
        memcpy(buf, kdc_data.CRL.buf, kdc_data.CRL.len);
        buf += kdc_data.CRL.len;

        memcpy(buf, (u_int8_t *) &kdc_data.cert_kdc.len,
                sizeof(kdc_data.cert_kdc.len));
        buf += sizeof(kdc_data.cert_kdc.len);
        memcpy(buf, kdc_data.cert_kdc.buf, kdc_data.cert_kdc.len);
        buf += kdc_data.cert_kdc.len;

        memcpy(buf, (u_int8_t *) &kdc_data.sign.len, sizeof(kdc_data.sign.len));
        buf += sizeof(kdc_data.sign.len);
        memcpy(buf, kdc_data.sign.buf, kdc_data.sign.len);
        buf += kdc_data.sign.len;

        memcpy(buf, (u_int8_t *) &kdc_data.key_nr, sizeof(kdc_data.key_nr));
        buf += sizeof(kdc_data.key_nr);

        memcpy(buf, (u_int8_t *) &kdc_data.sign_key.len,
                sizeof(kdc_data.sign_key.len));
        buf += sizeof(kdc_data.sign_key.len);
        memcpy(buf, kdc_data.sign_key.buf, kdc_data.sign_key.len);
        buf += kdc_data.sign_key.len;
    }
    //timestamp
    memcpy(buf, (u_int8_t *) &timestamp, sizeof(timestamp));
    buf += sizeof(timestamp);

    //sign
    memcpy(buf, (u_int8_t *) &sign.len, sizeof(sign.len));
    buf += sizeof(sign.len);
    memcpy(buf, sign.buf, sign.len);
    buf += sign.len;
    EV << "signlenge = " << sign.len << "\n";

//    //test
//    u_int32_t tempAdd = 0xFFFFFFFF;
//    memcpy(buf, (u_int8_t *)&tempAdd, sizeof(tempAdd));
//    buf += sizeof(tempAdd);*/
    *l = len;
    return data;
}
#endif
