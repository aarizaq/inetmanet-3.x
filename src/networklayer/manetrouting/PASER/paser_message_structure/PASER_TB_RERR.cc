/*
 *\class       PASER_TB_RERR
 *@brief       Class is an implementation of PASER_TB_RERR messages
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
#include "PASER_TB_RERR.h"

#include <openssl/sha.h>
#include <openssl/x509.h>
#include "PASER_Definitions.h"

PASER_TB_RERR::PASER_TB_RERR(const PASER_TB_RERR &m) {
    setName(m.getName());
    operator=(m);
}

PASER_TB_RERR::PASER_TB_RERR(struct in_addr src, u_int32_t seqNr) {
    type = B_RERR;
    srcAddress_var = src;
    destAddress_var = src;
    seq = seqNr;

    keyNr = 0;
}

PASER_TB_RERR::~PASER_TB_RERR() {
    free(secret);
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        u_int8_t *data = (u_int8_t *) *it;
        free(data);
    }
    auth.clear();
    free(hash);
}

PASER_TB_RERR& PASER_TB_RERR::operator =(const PASER_TB_RERR &m) {
    if (this == &m)
        return *this;
    cPacket::operator=(m);

    // PASER_MSG
    type = m.type;
    srcAddress_var.S_addr = m.srcAddress_var.S_addr;
    destAddress_var.S_addr = m.destAddress_var.S_addr;
    seq = m.seq;

    keyNr = m.keyNr;

    // PASER_TB_RERR
//    lastUnreachableSeq = m.lastUnreachableSeq;
    std::list<unreachableBlock> tempList(m.UnreachableAdressesList);
    for (std::list<unreachableBlock>::iterator it = tempList.begin();
            it != tempList.end(); it++) {
        unreachableBlock temp;
        temp.addr.S_addr = ((unreachableBlock) *it).addr.S_addr;
        temp.seq = ((unreachableBlock) *it).seq;
        UnreachableAdressesList.push_back(temp);
    }
//    std::list<u_int32_t> tempSeqList (m.SeqAdressesList);
//    for(std::list<u_int32_t>::iterator it = tempSeqList.begin(); it!= tempSeqList.end(); it++){
//        SeqAdressesList.push_back( (u_int32_t)*it );
//    }
//    std::list<address_list> tempList (m.UnreachableAdressesList);
//    for(std::list<address_list>::iterator it = tempList.begin(); it!= tempList.end(); it++){
//        UnreachableAdressesList.push_back( (address_list)*it );
//    }

    geoForwarding.lat = m.geoForwarding.lat;
    geoForwarding.lon = m.geoForwarding.lon;

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
    // Hash
    hash = (u_int8_t *) malloc((sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(hash, m.hash, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));

    return *this;
}

std::string PASER_TB_RERR::detailedInfo() const {
    std::stringstream out;
    out << "Type: RERR \n";
    out << "Querying node: " << srcAddress_var.S_addr.getIPv4().str()
            << "\n";
//    out << "First unreachable node: "<< UnreachableAdressesList.front().S_addr.getIPv4().str() << "\n";
    out << "Sequence: " << seq << "\n";
    out << "KeyNR: " << keyNr << "\n";
//    out << "Route.size: "<< routeFromQueryingToForwarding.size() << "\n";
//    std::list<struct in_addr> temp;
//    temp.assign(routeFromQueryingToForwarding.begin(), routeFromQueryingToForwarding.end());
//    for(std::list<struct in_addr>::iterator it=temp.begin(); it!=temp.end(); it++){
//        struct in_addr temp = (struct in_addr)*it;
//        out << "route: " << temp.S_addr.getIPv4().str() << "\n";
//    }

    return out.str();
}

u_int8_t * PASER_TB_RERR::toByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(seq);
//    len += sizeof(lastUnreachableSeq);
    len += sizeof(keyNr);

    len += sizeof(len); // groesse der UnreachableAdressesList
    for (std::list<unreachableBlock>::iterator it =
            UnreachableAdressesList.begin();
            it != UnreachableAdressesList.end(); it++) {
        unreachableBlock temp = (unreachableBlock) *it;
        len += sizeof(temp.addr.S_addr);
        len += sizeof(temp.seq);
    }
//    len += sizeof(len); // groesse der SeqAdressesList
//    for (std::list<u_int32_t>::iterator it=SeqAdressesList.begin(); it!=SeqAdressesList.end(); it++){
//        u_int32_t temp = (u_int32_t)*it;
//        len += sizeof(temp);
//    }
//    for (std::list<address_list>::iterator it=UnreachableAdressesList.begin(); it!=UnreachableAdressesList.end(); it++){
//        address_list temp = (address_list)*it;
//        len += sizeof(temp.ipaddr.S_addr);
//        len += sizeof(len); // groesse der add_r
//        for (std::list<address_range>::iterator it2=temp.range.begin(); it2!=temp.range.end(); it2++){
//            struct in_addr temp_addr ;
//            temp_addr.S_addr = ((address_range)*it2).ipaddr.S_addr;
//            len += sizeof(temp_addr.S_addr);
//            temp_addr.S_addr = ((address_range)*it2).mask.S_addr;
//            len += sizeof(temp_addr.S_addr);
//        }
//    }

    len += sizeof(geoForwarding.lat);
    len += sizeof(geoForwarding.lon);

    // Secret
    len += PASER_SECRET_LEN;

    //Auth
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
    data[0] = 0x05;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);

    // Key number
    memcpy(buf, (u_int8_t *) &keyNr, sizeof(keyNr));
    buf += sizeof(keyNr);
//    // last unreachable Sequence number
//    memcpy(buf, (u_int8_t *)&lastUnreachableSeq, sizeof(lastUnreachableSeq));
//    buf += sizeof(lastUnreachableSeq);

    // UnreachableAdressesList
    // Groesse der UnreachableAdressesList
    int tempListSize = UnreachableAdressesList.size();
    memcpy(buf, (u_int8_t *) &tempListSize, sizeof(tempListSize));
    buf += sizeof(tempListSize);
    for (std::list<unreachableBlock>::iterator it =
            UnreachableAdressesList.begin();
            it != UnreachableAdressesList.end(); it++) {
        unreachableBlock temp = (unreachableBlock) *it;
        memcpy(buf, &temp.addr.S_addr, sizeof(temp.addr.S_addr));
        buf += sizeof(temp.addr.S_addr);
        memcpy(buf, &temp.seq, sizeof(temp.seq));
        buf += sizeof(temp.seq);
    }
//    // SeqAdressesList
//    // Groesse der SeqAdressesList
//    int tempSeqListSize = SeqAdressesList.size();
//    memcpy(buf, (u_int8_t *)&tempSeqListSize, sizeof(tempSeqListSize));
//    buf += sizeof(tempSeqListSize);
//    for (std::list<u_int32_t>::iterator it=SeqAdressesList.begin(); it!=SeqAdressesList.end(); it++){
//        u_int32_t temp = (u_int32_t)*it;
//        memcpy(buf, (u_int8_t *)&temp, sizeof(temp));
//        buf += sizeof(temp);
//    }
//    for (std::list<address_list>::iterator it=UnreachableAdressesList.begin(); it!=UnreachableAdressesList.end(); it++){
//        address_list temp = (address_list)*it;
//        memcpy(buf, temp.ipaddr.S_addr.toString(10), sizeof(temp.ipaddr.S_addr));
//        buf += sizeof(temp.ipaddr.S_addr);
//        // Groesse der address_range
//        int tempAdd = temp.range.size();
//        memcpy(buf, (u_int8_t *)&tempAdd, sizeof(tempAdd));
//        buf += sizeof(tempAdd);
//        for (std::list<address_range>::iterator it2=temp.range.begin(); it2!=temp.range.end(); it2++){
//            struct in_addr temp_addr;
//            temp_addr.S_addr = ((address_range)*it2).ipaddr.S_addr;
//            memcpy(buf, temp_addr.S_addr.toString(10), sizeof(temp_addr.S_addr));
//            buf += sizeof(temp_addr.S_addr);
//            temp_addr.S_addr = ((address_range)*it2).mask.S_addr;
//            memcpy(buf, temp_addr.S_addr.toString(10), sizeof(temp_addr.S_addr));
//            buf += sizeof(temp_addr.S_addr);
//        }
//    }

    // Geo of forwarding node
    memcpy(buf, (u_int8_t *) &geoForwarding.lat, sizeof(geoForwarding.lat));
    buf += sizeof(geoForwarding.lat);
    memcpy(buf, (u_int8_t *) &geoForwarding.lon, sizeof(geoForwarding.lon));
    buf += sizeof(geoForwarding.lon);

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

//printf("small:0x");
//for (int n = 0; n < len; n++)
//    printf("%02x", data[n]);
//putchar('\n');*/
    *l = len;
    EV << "len = " << len << "\n";
    return data;
}

u_int8_t * PASER_TB_RERR::getCompleteByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(seq);
//    len += sizeof(lastUnreachableSeq);
    len += sizeof(keyNr);
    len += sizeof(len); // groesse der UnreachableAdressesList
    for (std::list<unreachableBlock>::iterator it =
            UnreachableAdressesList.begin();
            it != UnreachableAdressesList.end(); it++) {
        unreachableBlock temp = (unreachableBlock) *it;
        len += sizeof(temp.addr.S_addr);
        len += sizeof(temp.seq);
    }
//    len += sizeof(len); // groesse der SeqAdressesList
//    for (std::list<u_int32_t>::iterator it=SeqAdressesList.begin(); it!=SeqAdressesList.end(); it++){
//        u_int32_t temp = (u_int32_t)*it;
//        len += sizeof(temp);
//    }
//    for (std::list<address_list>::iterator it=UnreachableAdressesList.begin(); it!=UnreachableAdressesList.end(); it++){
//        address_list temp = (address_list)*it;
//        len += sizeof(temp.ipaddr.S_addr);
//        len += sizeof(len); // groesse der add_r
//        for (std::list<address_range>::iterator it2=temp.range.begin(); it2!=temp.range.end(); it2++){
//            struct in_addr temp_addr ;
//            temp_addr.S_addr = ((address_range)*it2).ipaddr.S_addr;
//            len += sizeof(temp_addr.S_addr);
//            temp_addr.S_addr = ((address_range)*it2).mask.S_addr;
//            len += sizeof(temp_addr.S_addr);
//        }
//    }

    len += sizeof(geoForwarding.lat);
    len += sizeof(geoForwarding.lon);

    //Secret
    len += PASER_SECRET_LEN;

    //Auth
    len += sizeof(auth.size());
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        len += SHA256_DIGEST_LENGTH;
    }
    //MAC
    len += SHA256_DIGEST_LENGTH;

    //Message Type
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    buf = data;
    //Message Type
    data[0] = 0x05;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);
    // Key number
    memcpy(buf, (u_int8_t *) &keyNr, sizeof(keyNr));
    buf += sizeof(keyNr);
//    // last unreachable Sequence number
//    memcpy(buf, (u_int8_t *)&lastUnreachableSeq, sizeof(lastUnreachableSeq));
//    buf += sizeof(lastUnreachableSeq);

    // UnreachableAdressesList
    // Groesse der UnreachableAdressesList
    int tempListSize = UnreachableAdressesList.size();
    memcpy(buf, (u_int8_t *) &tempListSize, sizeof(tempListSize));
    buf += sizeof(tempListSize);
    for (std::list<unreachableBlock>::iterator it =
            UnreachableAdressesList.begin();
            it != UnreachableAdressesList.end(); it++) {
        unreachableBlock temp = (unreachableBlock) *it;
        memcpy(buf, &temp.addr.S_addr, sizeof(temp.addr.S_addr));
        buf += sizeof(temp.addr.S_addr);
        memcpy(buf, &temp.seq, sizeof(temp.seq));
        buf += sizeof(temp.seq);
    }
//    // SeqAdressesList
//    // Groesse der SeqAdressesList
//    int tempSeqListSize = SeqAdressesList.size();
//    memcpy(buf, (u_int8_t *)&tempSeqListSize, sizeof(tempSeqListSize));
//    buf += sizeof(tempSeqListSize);
//    for (std::list<u_int32_t>::iterator it=SeqAdressesList.begin(); it!=SeqAdressesList.end(); it++){
//        u_int32_t temp = (u_int32_t)*it;
//        memcpy(buf, (u_int8_t *)&temp, sizeof(temp));
//        buf += sizeof(temp);
//    }

//    for (std::list<address_list>::iterator it=UnreachableAdressesList.begin(); it!=UnreachableAdressesList.end(); it++){
//        address_list temp = (address_list)*it;
//        memcpy(buf, temp.ipaddr.S_addr.toString(10), sizeof(temp.ipaddr.S_addr));
//        buf += sizeof(temp.ipaddr.S_addr);
//        // Groesse der address_range
//        int tempAdd = temp.range.size();
//        memcpy(buf, (u_int8_t *)&tempAdd, sizeof(tempAdd));
//        buf += sizeof(tempAdd);
//        for (std::list<address_range>::iterator it2=temp.range.begin(); it2!=temp.range.end(); it2++){
//            struct in_addr temp_addr;
//            temp_addr.S_addr = ((address_range)*it2).ipaddr.S_addr;
//            memcpy(buf, temp_addr.S_addr.toString(10), sizeof(temp_addr.S_addr));
//            buf += sizeof(temp_addr.S_addr);
//            temp_addr.S_addr = ((address_range)*it2).mask.S_addr;
//            memcpy(buf, temp_addr.S_addr.toString(10), sizeof(temp_addr.S_addr));
//            buf += sizeof(temp_addr.S_addr);
//        }
//    }

    // Geo of forwarding node
    memcpy(buf, (u_int8_t *) &geoForwarding.lat, sizeof(geoForwarding.lat));
    buf += sizeof(geoForwarding.lat);
    memcpy(buf, (u_int8_t *) &geoForwarding.lon, sizeof(geoForwarding.lon));
    buf += sizeof(geoForwarding.lon);

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

    //hash
    memcpy(buf, (u_int8_t *) hash, SHA256_DIGEST_LENGTH);
    buf += SHA256_DIGEST_LENGTH;

//printf("big  :0x");
//for (int n = 0; n < len; n++)
//    printf("%02x", data[n]);
//putchar('\n');
//opp_error("rrrwrwrwr");

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}
#endif
