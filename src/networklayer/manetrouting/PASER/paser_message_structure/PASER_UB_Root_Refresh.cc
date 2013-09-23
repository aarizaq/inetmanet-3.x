/*
 *\class       PASER_UB_Root_Refresh
 *@brief       Class is an implementation of PASER_UB_Root_Refresh messages
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
#include "PASER_UB_Root_Refresh.h"
#include <openssl/sha.h>
#include <openssl/x509.h>


PASER_UB_Root_Refresh::PASER_UB_Root_Refresh(const PASER_UB_Root_Refresh &m) {
    setName(m.getName());
    operator=(m);
}



PASER_UB_Root_Refresh::PASER_UB_Root_Refresh(struct in_addr src, u_int32_t seqNr) {
    type = B_ROOT;
    srcAddress_var = src;
    destAddress_var = src;
    seq = seqNr;

    sign.buf = NULL;
    sign.len = 0;

    timestamp = 0;
}


PASER_UB_Root_Refresh::~PASER_UB_Root_Refresh() {
//    X509 *x;
//    x = (X509*) cert.buf;
    free(cert.buf);
    free(root);
    if (sign.len > 0) {
        free(sign.buf);
        sign.len = 0;
    }
}



PASER_UB_Root_Refresh& PASER_UB_Root_Refresh::operator =(const PASER_UB_Root_Refresh &m) {
    if (this == &m)
        return *this;
    cPacket::operator=(m);

    // PASER_MSG
    type = m.type;
    srcAddress_var.S_addr = m.srcAddress_var.S_addr;
    destAddress_var.S_addr = m.destAddress_var.S_addr;
    seq = m.seq;

    // PASER_UB_Root_Refresh
    timestamp = m.timestamp;

    cert.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.cert.len));
    memcpy(cert.buf, m.cert.buf, (sizeof(u_int8_t) * m.cert.len));
    cert.len = m.cert.len;

    root = (u_int8_t *) malloc((sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(root, m.root, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));

    initVector = m.initVector;

    geoQuerying.lat = m.geoQuerying.lat;
    geoQuerying.lon = m.geoQuerying.lon;

    sign.buf = (u_int8_t *) malloc((sizeof(u_int8_t) * m.sign.len));
    memcpy(sign.buf, m.sign.buf, (sizeof(u_int8_t) * m.sign.len));
    sign.len = m.sign.len;
    return *this;
}


std::string PASER_UB_Root_Refresh::detailedInfo() const {
    std::stringstream out;
    out << "Type: PASER_UB_Root_Refresh \n";
    out << "Querying node: " << srcAddress_var.S_addr.getIPv4().str()
            << "\n";
    out << "Sequence: " << seq << "\n";
    out << "initVector: " << (int32_t) initVector << " .\n";
    return out.str();
}



u_int8_t * PASER_UB_Root_Refresh::toByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(seq);

    len += sizeof(cert.len);
    len += cert.len;

    len += 32;
    len += sizeof(initVector);
    len += sizeof(geoQuerying.lat);
    len += sizeof(geoQuerying.lon);
    len += sizeof(timestamp);

    //Message Type
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    memset(data, 0xEE, len);
    buf = data;
    //Message Type
    data[0] = 0x07;
    buf++;
  //********************************  Comment this section due to signature errors in the inet2.0 version for unkown reasons.********************************/
  /*  //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);

    // Cert of querying node
    memcpy(buf, (u_int8_t *) &cert.len, sizeof(cert.len));
    buf += sizeof(cert.len);
    memcpy(buf, cert.buf, cert.len);
    buf += cert.len;

    // ROOT
    memcpy(buf, root, 32);
    buf += 32;
    // IV
    memcpy(buf, (u_int8_t *) &initVector, sizeof(initVector));
    buf += sizeof(initVector);
    // Geo of querying node
    memcpy(buf, (u_int8_t *) &geoQuerying.lat, sizeof(geoQuerying.lat));
    buf += sizeof(geoQuerying.lat);
    memcpy(buf, (u_int8_t *) &geoQuerying.lon, sizeof(geoQuerying.lon));
    buf += sizeof(geoQuerying.lon);
    // Timestamp
    memcpy(buf, (u_int8_t *) &timestamp, sizeof(timestamp));
    buf += sizeof(timestamp);*/

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}



u_int8_t * PASER_UB_Root_Refresh::getCompleteByteArray(int *l) {
    int len = 0;
    len += 1;
    len += sizeof(srcAddress_var.S_addr);
    len += sizeof(seq);

    len += sizeof(cert.len);
    len += cert.len;

    len += 32;
    len += sizeof(initVector);
    len += sizeof(geoQuerying.lat);
    len += sizeof(geoQuerying.lon);
    len += sizeof(timestamp);

    len += sizeof(sign.len);
    len += sign.len;

    //Message Type
    u_int8_t *buf;
    u_int8_t *data = (u_int8_t *) malloc(len);
    buf = data;
    //Message Type
    data[0] = 0x07;
    buf++;

    //Querying node
    memcpy(buf, &srcAddress_var.S_addr,
            sizeof(srcAddress_var.S_addr));
    buf += sizeof(srcAddress_var.S_addr);
    // Sequence number
    memcpy(buf, (u_int8_t *) &seq, sizeof(seq));
    buf += sizeof(seq);

    // Cert of querying node
    memcpy(buf, (u_int8_t *) &cert.len, sizeof(cert.len));
    buf += sizeof(cert.len);
    memcpy(buf, cert.buf, cert.len);
    buf += cert.len;

    // ROOT
    memcpy(buf, root, 32);
    buf += 32;
    // IV
    memcpy(buf, (u_int8_t *) &initVector, sizeof(initVector));
    buf += sizeof(initVector);
    // Geo of querying node
    memcpy(buf, (u_int8_t *) &geoQuerying.lat, sizeof(geoQuerying.lat));
    buf += sizeof(geoQuerying.lat);
    memcpy(buf, (u_int8_t *) &geoQuerying.lon, sizeof(geoQuerying.lon));
    buf += sizeof(geoQuerying.lon);
    //Timestamp
    memcpy(buf, (u_int8_t *) &timestamp, sizeof(timestamp));
    buf += sizeof(timestamp);

    //Sign
    memcpy(buf, (u_int8_t *) &sign.len, sizeof(sign.len));
    buf += sizeof(sign.len);
    memcpy(buf, sign.buf, sign.len);
    buf += sign.len;
    EV << "signlenge = " << sign.len << "\n";

    *l = len;
    EV << "len = " << len << "\n";
    return data;
}
#endif
