/*
 *\class       PASER_TU_RREP_ACK
 *@brief       Class is an implementation of PASER_TU_RREP_ACK messages
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
#include "PASER_TU_RREP_ACK.h"
#include <openssl/sha.h>


PASER_TU_RREP_ACK::PASER_TU_RREP_ACK(const PASER_TU_RREP_ACK &m) {
    setName(m.getName());
    operator=(m);
}


PASER_TU_RREP_ACK::PASER_TU_RREP_ACK(struct in_addr src, struct in_addr dest,
        u_int32_t seqNr) {
    type = TU_RREP_ACK;
    srcAddress_var = src;
    destAddress_var = dest;
    seq = seqNr;

    keyNr = 0;
}


PASER_TU_RREP_ACK::~PASER_TU_RREP_ACK() {
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        u_int8_t *temp = (u_int8_t *) *it;
        free(temp);
    }
    free(secret);
    auth.clear();
    free(hash);
}


PASER_TU_RREP_ACK& PASER_TU_RREP_ACK::operator =(const PASER_TU_RREP_ACK &m) {
    if (this == &m)
        return *this;
    cPacket::operator=(m);

    // PASER_MSG
    type = m.type;
    srcAddress_var.S_addr = m.srcAddress_var.S_addr;
    destAddress_var.S_addr = m.destAddress_var.S_addr;
    seq = m.seq;

    keyNr = m.keyNr;

    // PASER_TU_RREP_ACK
    // secret
    secret = (u_int8_t *) malloc((sizeof(u_int8_t) * PASER_SECRET_LEN));
    memcpy(secret, m.secret, (sizeof(u_int8_t) * PASER_SECRET_LEN));
    // auth
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


std::string PASER_TU_RREP_ACK::detailedInfo() const {
    std::stringstream out;
    out << "Type : TU_RREP_ACK \n";
    out << "Querying node : " << srcAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Destination node : " << destAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Sequence : " << seq << "\n";
    out << "KeyNr : " << keyNr << "\n";
    out << "AuthTreeLength : " << auth.size();
    return out.str();
}


u_int8_t * PASER_TU_RREP_ACK::toByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(destAddress_var.S_addr);
    len += sizeof(seq);
    len += sizeof(keyNr);

    len += PASER_SECRET_LEN;
    len += sizeof(auth.size());
    len += auth.size() * SHA256_DIGEST_LENGTH;

    //messageType
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    memset(data, 0xEE, len);
    buf = data;
    //********************************  Comment this section due to signature errors in the inet2.0 version for unkown reasons.********************************/
           /*
    //messageType
    data[0] = 0x04;
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

    // secret
    memcpy(buf, secret, PASER_SECRET_LEN);
    buf += PASER_SECRET_LEN;
    // authentication path
    int authLen = auth.size();
    memcpy(buf, &authLen, sizeof(authLen));
    buf += sizeof(authLen);
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        u_int8_t * temp = (u_int8_t *) *it;
        memcpy(buf, temp, SHA256_DIGEST_LENGTH);
        buf += SHA256_DIGEST_LENGTH;
    }*/

    *l = len;
    return data;
}


u_int8_t * PASER_TU_RREP_ACK::getCompleteByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(destAddress_var.S_addr);
    len += sizeof(seq);
    len += sizeof(keyNr);

    len += PASER_SECRET_LEN;
    len += sizeof(auth.size());
    len += auth.size() * SHA256_DIGEST_LENGTH;

    len += SHA256_DIGEST_LENGTH;

    //messageType
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    buf = data;
    //messageType
    data[0] = 0x04;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    //Dest node
    memcpy(buf, &destAddress_var,
            sizeof(destAddress_var.S_addr));
    buf += sizeof(destAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);
    // Key number
    memcpy(buf, (u_int8_t *) &keyNr, sizeof(keyNr));
    buf += sizeof(keyNr);

    // secret
    memcpy(buf, secret, PASER_SECRET_LEN);
    buf += PASER_SECRET_LEN;
    // authentication path
    int authLen = auth.size();
    memcpy(buf, &authLen, sizeof(authLen));
    buf += sizeof(authLen);
    for (std::list<u_int8_t *>::iterator it = auth.begin(); it != auth.end();
            it++) {
        u_int8_t * temp = (u_int8_t *) *it;
        memcpy(buf, temp, SHA256_DIGEST_LENGTH);
        buf += SHA256_DIGEST_LENGTH;
    }

    //hash
    memcpy(buf, (u_int8_t *) hash, SHA256_DIGEST_LENGTH);
    buf += SHA256_DIGEST_LENGTH;

    *l = len;
    return data;
}
#endif
