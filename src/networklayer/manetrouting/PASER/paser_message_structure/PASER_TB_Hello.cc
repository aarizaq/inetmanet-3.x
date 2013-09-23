/*
 *\class       PASER_TB_Hello
 *@brief       Class is an implementation of PASER_TB_Hello messages
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
#include "PASER_TB_Hello.h"

#include <openssl/sha.h>
#include <openssl/x509.h>
#include "PASER_Definitions.h"

PASER_TB_Hello::PASER_TB_Hello(const PASER_TB_Hello &m) {
    setName(m.getName());
    operator=(m);
}

PASER_TB_Hello::PASER_TB_Hello(struct in_addr src, u_int32_t seqNr) {
    type = B_HELLO;
    srcAddress_var = src;
    destAddress_var = src;
    seq = seqNr;

    secret = NULL;
    hash = NULL;
}

PASER_TB_Hello::~PASER_TB_Hello() {
    if (secret != NULL) {
        free(secret);
    }
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        u_int8_t *data = (u_int8_t *) *it;
        free(data);
    }
    auth.clear();
    if (hash != NULL) {
        free(hash);
    }
}

PASER_TB_Hello& PASER_TB_Hello::operator =(const PASER_TB_Hello &m) {
    if (this == &m)
        return *this;
    cPacket::operator=(m);

    // PASER_MSG
    type = m.type;
    srcAddress_var.S_addr = m.srcAddress_var.S_addr;
    destAddress_var.S_addr = m.destAddress_var.S_addr;
    seq = m.seq;

    // PASER_TB_Hello
    std::list<address_list> tempList(m.AddressRangeList);
    for (std::list<address_list>::iterator it = tempList.begin();
            it != tempList.end(); it++) {
        AddressRangeList.push_back((address_list) *it);
    }

    geoQuerying.lat = m.geoQuerying.lat;
    geoQuerying.lon = m.geoQuerying.lon;

    // Secret
    secret = (u_int8_t *) malloc((sizeof(u_int8_t) * PASER_SECRET_LEN));
    memcpy(secret, m.secret, (sizeof(u_int8_t) * PASER_SECRET_LEN));
    // Auth
    std::list<u_int8_t *> temp(m.auth);
    for (std::list<u_int8_t *>::iterator it = temp.begin(); it != temp.end();
            it++) {
        u_int8_t *data = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
        memcpy(data, (u_int8_t *) *it,
                (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
        auth.push_back(data);
    }
    // hash
    hash = (u_int8_t *) malloc((sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(hash, m.hash, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));

    return *this;
}

std::string PASER_TB_Hello::detailedInfo() const {
    std::stringstream out;
    out << "Type: PASER_TB_Hello \n";
    out << "Querying node: " << srcAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Sequence: " << seq << "\n";
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
    return out.str();
}

u_int8_t * PASER_TB_Hello::toByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(seq);

    len += sizeof(len); // groesse der Adl!!!
    for (std::list<address_list>::iterator it = AddressRangeList.begin();
            it != AddressRangeList.end(); it++) {
        address_list temp = (address_list) *it;
        len += sizeof(temp.ipaddr.S_addr);
        len += sizeof(len); // groesse der add_r!!!
        for (std::list<address_range>::iterator it2 = temp.range.begin();
                it2 != temp.range.end(); it2++) {
            struct in_addr temp_addr;
            temp_addr.S_addr = ((address_range) *it2).ipaddr.S_addr;
            len += sizeof(temp_addr.S_addr);
            temp_addr.S_addr = ((address_range) *it2).mask.S_addr;
            len += sizeof(temp_addr.S_addr);
        }
    }
    len += sizeof(geoQuerying.lat);
    len += sizeof(geoQuerying.lon);

    // Secret
    len += PASER_SECRET_LEN;

    // Auth
    len += sizeof(auth.size());
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        len += SHA256_DIGEST_LENGTH;
    }

    //Message Type
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    memset(data, 0xEE, len);
    buf = data;
    //********************************  Comment this section due to signature errors in the inet2.0 version for unkown reasons.********************************/
           /*
    //Message Type
    data[0] = 0x06;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);
    // Size of ADL !!!
    int tempListSize = AddressRangeList.size();
    memcpy(buf, (u_int8_t *) &tempListSize, sizeof(tempListSize));
    buf += sizeof(tempListSize);
    for (std::list<address_list>::iterator it = AddressRangeList.begin();
            it != AddressRangeList.end(); it++) {
        address_list temp = (address_list) *it;
        memcpy(buf, &temp.ipaddr.S_addr,
                sizeof(temp.ipaddr.S_addr));
        buf += sizeof(temp.ipaddr.S_addr);
        // Size of the address_range !!!
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
    // Geo of Querying node
    memcpy(buf, (u_int8_t *) &geoQuerying.lat, sizeof(geoQuerying.lat));
    buf += sizeof(geoQuerying.lat);
    memcpy(buf, (u_int8_t *) &geoQuerying.lon, sizeof(geoQuerying.lon));
    buf += sizeof(geoQuerying.lon);

    // Secret
    memcpy(buf, (u_int8_t *) secret, (sizeof(u_int8_t) * PASER_SECRET_LEN));
    buf += sizeof(u_int8_t) * PASER_SECRET_LEN;
    // Auth
    int authLen = auth.size();
    memcpy(buf, &authLen, sizeof(authLen));
    buf += sizeof(authLen);
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        memcpy(buf, (u_int8_t *) *it,
                (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
        buf += sizeof(u_int8_t) * SHA256_DIGEST_LENGTH;
    }*/

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}

u_int8_t * PASER_TB_Hello::getCompleteByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(seq);

    len += sizeof(len); // Size of Adl !!!
    for (std::list<address_list>::iterator it = AddressRangeList.begin();
            it != AddressRangeList.end(); it++) {
        address_list temp = (address_list) *it;
        len += sizeof(temp.ipaddr.S_addr);
        len += sizeof(len); // Size of add_r !!!
        for (std::list<address_range>::iterator it2 = temp.range.begin();
                it2 != temp.range.end(); it2++) {
            struct in_addr temp_addr;
            temp_addr.S_addr = ((address_range) *it2).ipaddr.S_addr;
            len += sizeof(temp_addr.S_addr);
            temp_addr.S_addr = ((address_range) *it2).mask.S_addr;
            len += sizeof(temp_addr.S_addr);
        }
    }
    len += sizeof(geoQuerying.lat);
    len += sizeof(geoQuerying.lon);

    //Secret
    len += PASER_SECRET_LEN;

    //Auth !!!!
    len += sizeof(auth.size());
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        len += SHA256_DIGEST_LENGTH;
    }
    // MAC
    len += SHA256_DIGEST_LENGTH;

    //Message Type
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    buf = data;
    //Message Type
    data[0] = 0x06;
    buf++;
    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);
    // Size of the address list!!!
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
    // Geo of querying node
    memcpy(buf, (u_int8_t *) &geoQuerying.lat, sizeof(geoQuerying.lat));
    buf += sizeof(geoQuerying.lat);
    memcpy(buf, (u_int8_t *) &geoQuerying.lon, sizeof(geoQuerying.lon));
    buf += sizeof(geoQuerying.lon);

    // Secret
    memcpy(buf, (u_int8_t *) secret, (sizeof(u_int8_t) * PASER_SECRET_LEN));
    buf += sizeof(u_int8_t) * PASER_SECRET_LEN;
    // Auth
    int authLen = auth.size();
    memcpy(buf, &authLen, sizeof(authLen));
    buf += sizeof(authLen);
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        memcpy(buf, (u_int8_t *) *it,
                (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
        buf += sizeof(u_int8_t) * SHA256_DIGEST_LENGTH;
    }
    // Hash
    memcpy(buf, (u_int8_t *) hash, SHA256_DIGEST_LENGTH);
    buf += SHA256_DIGEST_LENGTH;

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}
#endif
