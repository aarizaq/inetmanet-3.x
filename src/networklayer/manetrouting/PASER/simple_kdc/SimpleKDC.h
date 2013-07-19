#ifndef __SimpleKDC_H__
#define __SimpleKDC_H__

/**
 *\class       SimpleKDC
 *@ingroup KDC
 *@brief       Class handles all Key Distribution Center (KDC) actions
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
//#define PORT_for_CRL_in 653
//#define PORT_for_CRL_out 653
#include "Configuration.h"
#ifdef OPENSSL_IS_LINKED
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/err.h"
#include "PASER_Definitions.h"
#include "PASER_UB_Key_Refresh.h"

#include "crl_message_m.h"

#include <omnetpp.h>
#include <list>
#include "csimplemodule.h"
#include "UDPSocket.h"
//#include "UDPBasicApp.h"

#define CRL_FILE "cert/crl.pem"

//typedef struct{
//    unsigned char *buf;
//    int len;
//} lv_block;
//
//typedef struct{
//    lv_block GTK;
//    u_int32_t nonce;
//    lv_block CRL;
//    lv_block cert_kdc;
//    lv_block sign;
//} kdc_block;


class SimpleKDC : public cSimpleModule
{


private:
    X509_CRL *crl;
    lv_block crl_als_block;

    X509 *ca_cert;

    EVP_PKEY *pkey;
    X509 *x509;
    lv_block cert_als_block;

    lv_block GTK;
    int key_nr;

    double message_dellay;

    cMessage * resetMessage;

    std::list<struct in_addr> nextHopList;

    UDPSocket socket;
public:
    SimpleKDC();
    virtual ~SimpleKDC();

protected:
    int numInitStages() const {return 4;}
    void initialize(int stage);
    void handleMessage(cMessage *msg);
    void setSocketOptions();

    int checkOneCert(X509 *cert);
    X509* extractCert(lv_block cert);
    int getCRL(lv_block *cert);

    int saveCertAlsBlock();
    int rsa_encrypt(lv_block in, lv_block *out, X509 *cert);
};
#endif
#endif
