/*
 *\class       PASER_UB_Key_Refresh
 *@brief       Class is an implementation of PASER_UB_Key_Refresh messages
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
#include "PASER_UB_Key_Refresh.h"

#include <openssl/sha.h>
#include <openssl/x509.h>

PASER_UB_Key_Refresh::PASER_UB_Key_Refresh(const PASER_UB_Key_Refresh &m) {
    setName(m.getName());
    operator=(m);
}

PASER_UB_Key_Refresh::PASER_UB_Key_Refresh(struct in_addr src) {
    type = B_RESET;
    srcAddress_var = src;
    destAddress_var = src;
    seq = 0;

    sign.buf = NULL;
    sign.len = 0;

}

/**
 * Destructor
 */
PASER_UB_Key_Refresh::~PASER_UB_Key_Refresh() {
    free(cert.buf);
    free(sign.buf);
}

PASER_UB_Key_Refresh& PASER_UB_Key_Refresh::operator =(const PASER_UB_Key_Refresh &m) {
    if (this == &m)
        return *this;
    cPacket::operator=(m);

    // PASER_MSG
    type = m.type;
    srcAddress_var.S_addr = m.srcAddress_var.S_addr;
    destAddress_var.S_addr = m.destAddress_var.S_addr;
    seq = m.seq;

    keyNr = m.keyNr;

    // PASER_UB_Key_Refresh
    cert.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.cert.len));
    memcpy(cert.buf, m.cert.buf, (sizeof(u_int8_t) * m.cert.len));
    cert.len = m.cert.len;

    sign.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.sign.len));
    memcpy(sign.buf, m.sign.buf, (sizeof(u_int8_t) * m.sign.len));
    sign.len = m.sign.len;

    return *this;
}

std::string PASER_UB_Key_Refresh::detailedInfo() const {
    std::stringstream out;
    out << "Type: PASER_UB_Key_Refresh \n";
    out << "Querying node: " << srcAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "keyNr: " << keyNr << "\n";
    return out.str();
}

u_int8_t * PASER_UB_Key_Refresh::toByteArray(int *l) {
    int len = 0;
//    len += 1;
//    len += sizeof(srcAddress_var.S_addr);

    len += sizeof(keyNr);
    len += sizeof(cert.len);
    len += cert.len;

    //Message type
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    memset(data, 0xEE, len);
    buf = data;
    //********************************  Comment this section due to signature errors in the inet2.0 version for unkown reasons.********************************/
    /*
    //Message type
//    data[0] = 0x08;
//    buf ++;

//    //Querying node
//    memcpy(buf, srcAddress_var.S_addr.toString(10), sizeof(srcAddress_var.S_addr));
//    buf += sizeof(srcAddress_var.S_addr);

// Key number

    memcpy(buf, (u_int8_t *) &keyNr, sizeof(keyNr));
    buf += sizeof(keyNr);
    memcpy(buf, (u_int8_t *) &cert.len, sizeof(cert.len));
    buf += sizeof(cert.len);
    memcpy(buf, cert.buf, cert.len);
    buf += cert.len;*/

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}

u_int8_t * PASER_UB_Key_Refresh::getCompleteByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);

    len += sizeof(keyNr);
    len += sizeof(cert.len);
    len += cert.len;

    len += sizeof(sign.len);
    len += sign.len;

    //Message type
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    buf = data;
    //Message type
    data[0] = 0x08;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);

    //Key number
    memcpy(buf, (u_int8_t *) &keyNr, sizeof(keyNr));
    buf += sizeof(keyNr);

    //Certificate
    memcpy(buf, (u_int8_t *) &cert.len, sizeof(cert.len));
    buf += sizeof(cert.len);
    memcpy(buf, cert.buf, cert.len);
    buf += cert.len;

    //Sign
    memcpy(buf, (u_int8_t *) &sign.len, sizeof(sign.len));
    buf += sizeof(sign.len);
    memcpy(buf, sign.buf, sign.len);
    buf += sign.len;

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}
#endif
