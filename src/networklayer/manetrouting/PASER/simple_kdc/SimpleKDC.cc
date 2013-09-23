/*
 *class       PASER_KDC
 *brief       Class handles all KDC actions
 *
 *Authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
 *
 *copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
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
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <omnetpp.h>
#include "UDPControlInfo_m.h"
#include "IPvXAddressResolver.h"
#include "InterfaceTableAccess.h"
#include "compatibility.h"
#include "ManetAddress.h"
#include <string.h>
#include "SimpleKDC.h"
#define MSGKIND_CONNECT  0
#define MSGKIND_SEND     1

Define_Module(SimpleKDC);

SimpleKDC::SimpleKDC() {
}

SimpleKDC::~SimpleKDC() {
    free(GTK.buf);
    free(crl_als_block.buf);
    X509_CRL_free(crl);
    EVP_PKEY_free(pkey);
    X509_free(ca_cert);
    X509_free(x509);
    free(cert_als_block.buf);

    if (resetMessage) {
        if (resetMessage->isScheduled()) {
            cancelEvent(resetMessage);
        }
        delete resetMessage;
    }
}

void SimpleKDC::initialize(int stage) {
    if (stage != 3)
        return;

    // Init OPENSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    char *crlfile = new char[strlen(CRL_FILE) + 1];
    strcpy(crlfile, CRL_FILE);

    FILE *fp;

    fp = fopen(crlfile, "r");
    if (fp == NULL) {
        printf("cafile not found: %s", crlfile);
        exit(1);
    }
    delete[] crlfile;
    crl = PEM_read_X509_CRL(fp, NULL, NULL, NULL);
    fclose(fp);
    if (crl == NULL) {
        ERR_print_errors_fp (stderr);
        exit(1);
    }

    if ((getCRL(&crl_als_block)) != 1) {
        printf("cann't decode CRL\n");
        exit(1);
    }
    EV << "crl.len = " << crl_als_block.len << "\n";

    key_nr = 1;
    char *gtkKey = (char *) malloc(6);
    strcpy(gtkKey, "12345");
    GTK.buf = (u_int8_t *) gtkKey;
    GTK.len = 6;

    //read cert
    char *certfile = new char[strlen("cert/kdccert.pem") + 1];
    strcpy(certfile, "cert/kdccert.pem");
    fp = fopen(certfile, "r");
    if (fp == NULL) {
        printf("certfile not found: %s", certfile);
        exit(1);
    }
    x509 = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);
    if (x509 == NULL) {
        ERR_print_errors_fp (stderr);
        exit(1);
    }
    saveCertAlsBlock();
    delete[] certfile;

    // Read Own Private Key
    char *keyfile = new char[strlen("cert/kdckey.key") + 1];
    strcpy(keyfile, "cert/kdckey.key");
    fp = fopen(keyfile, "r");
    if (fp == NULL) {
        printf("keyfile not found: %s", keyfile);
        exit(1);
    }
    pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);
    delete[] keyfile;

    // Read CA_file
    char * cafile = new char[strlen(PASER_CA_cert_file) + 1];
    strcpy(cafile, PASER_CA_cert_file);
    fp = fopen(cafile, "r");
    if (fp == NULL) {
        printf("CAcertPath not found: %s", cafile);
        exit(1);
    }
    ca_cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);
    if (ca_cert == NULL) {
        ERR_print_errors_fp (stderr);
        exit(1);
    }
    delete[] cafile;

//    bindToPort((int)par("port").longValue());
    socket.setOutputGate(gate("udpOut"));
    socket.bind(((int) par("port").longValue()));
    setSocketOptions();

    message_dellay = 0;

    resetMessage = 0;
    if (par("resetMessageTime").doubleValue() > 0) {
        resetMessage = new cMessage("resetMessage");
        simtime_t timer;
        timer = simTime() + par("resetMessageTime");
        scheduleAt(timer, resetMessage);
    }
}

void SimpleKDC::handleMessage(cMessage *msg) {
    message_dellay = 0;
    if (msg == resetMessage) {
        key_nr++;
        char *gtkKey = (char *) malloc(6);
        strcpy(gtkKey, "54321");
        free(GTK.buf);
        GTK.buf = (u_int8_t *) gtkKey;
        GTK.len = 6;

        EV << "list.size = " << nextHopList.size() << "\n";
        for (std::list<struct in_addr>::iterator it = nextHopList.begin();
                it != nextHopList.end(); it++) {
            struct in_addr tempAddr = (struct in_addr) *it;
            EV << "nextHopAddr: " << tempAddr.S_addr.getIPv4().str()
                    << "\n";
//            kdcReset *message = new kdcReset();
//            message->setKdc_key_nr(key_nr);
            struct in_addr tempSrc;
            tempSrc.S_addr = PASER_BROADCAST;
            PASER_UB_Key_Refresh * message = new PASER_UB_Key_Refresh(tempSrc);
            message->keyNr = key_nr;
            message->cert.len = cert_als_block.len;
            message->cert.buf = (u_int8_t *) malloc(
                    (sizeof(u_int8_t) * message->cert.len));
            memcpy(message->cert.buf, cert_als_block.buf,
                    (sizeof(u_int8_t) * message->cert.len));

            u_int32_t sig_len = PASER_sign_len;
            u_int8_t *sign = (u_int8_t *) malloc(sizeof(u_int8_t) * sig_len);
            EVP_MD_CTX *md_ctx;
            md_ctx = EVP_MD_CTX_create();
            EVP_SignInit(md_ctx, EVP_sha1());

            int message_len = 0;
            u_int8_t *data = message->toByteArray(&message_len);
            EVP_SignUpdate(md_ctx, data, message_len);

            int err = EVP_SignFinal(md_ctx, sign, &sig_len, pkey);
            EVP_MD_CTX_destroy(md_ctx);
            if (err != 1) {
                ERR_print_errors_fp(stderr);
                opp_error("signRESET");
            }
            message->sign.buf = sign;
            message->sign.len = sig_len;
//printf("sign_KDCnewRESET_:");
//for (int n = 0; n < sig_len; n++)
//    printf("%02x", sign[n]);
//putchar('\n');
            free(data);
            socket.sendTo(message, tempAddr.S_addr.getIPv4(),
                    (int) par("port").longValue());
        }
        return;
    }
    if (msg->isSelfMessage()) {
        if (dynamic_cast<crl_message *>(msg)) {
            crl_message *message = check_and_cast<crl_message *>(msg);
            bool found = false;
            for (std::list<struct in_addr>::iterator it = nextHopList.begin();
                    it != nextHopList.end(); it++) {
                struct in_addr tempAddr = (struct in_addr) *it;
                if (tempAddr.S_addr == message->getGwAddr().S_addr) {
                    found = true;
                }
            }
            if (!found) {
                nextHopList.push_back(message->getGwAddr());
            }
            socket.sendTo(message, message->getGwAddr().S_addr.getIPv4(),
                    (int) par("port").longValue());
//            sendToUDP(message, (int)par("port").longValue(), message->getGwAddr().S_addr.getIPv4(), (int)par("port").longValue());
        } else {
            delete msg;
        }
        return;
    }
    if (dynamic_cast<crl_message *>(msg)) {
        struct in_addr dest_Addr;
        crl_message *message = check_and_cast<crl_message *>(msg);
        EV << "message->getGwAddr(): "
                << message->getGwAddr().S_addr.getIPv4().str() << "\n";
        struct in_addr src = message->getSrc();
        EV << "src: " << src.S_addr.getIPv4().str() << "\n";
        //Verify certificate of incoming message
        lv_block tempCert;
        tempCert.len = message->getCert_len();
        tempCert.buf = (u_int8_t*) malloc(tempCert.len);
        for (u_int32_t i = 0; i < tempCert.len; i++) {
            tempCert.buf[i] = message->getCert_array(i);
        }
        X509* clientCert = extractCert(tempCert);
        EV << "certLen: " << tempCert.len << "\n";
        free(tempCert.buf);
        if (clientCert == NULL) {
            EV << "kein Cert\n";
            return;
        }
        if (checkOneCert(clientCert) != 1) {
            EV << "Falsches cert\n";
            X509_free(clientCert);
            delete message;
            return;
        }
        message_dellay += par("kdc_verify_mess_delay").doubleValue();
        EV << "cert OK\n";

        kdc_block kdcData;
        // GTK
        rsa_encrypt(GTK, &kdcData.GTK, clientCert);
        message_dellay += par("kdc_rsa_enc_delay").doubleValue();
        EV << "GTK OK\n";
        X509_free(clientCert);

        // Nonce
        kdcData.nonce = message->getKdc_nonce();
        EV << "nonce: " << kdcData.nonce << "\n";

        // CRL
        kdcData.CRL.len = crl_als_block.len;
        kdcData.CRL.buf = (u_int8_t *) malloc(crl_als_block.len);
        memcpy(kdcData.CRL.buf, crl_als_block.buf, crl_als_block.len);
        EV << "CRL OK\n";

        // Certificate of KDC
        kdcData.cert_kdc.len = cert_als_block.len;
        kdcData.cert_kdc.buf = (u_int8_t *) malloc(cert_als_block.len);
        memcpy(kdcData.cert_kdc.buf, cert_als_block.buf, cert_als_block.len);

        // Current key number
        kdcData.key_nr = key_nr;

        // Sign
        u_int32_t sig_len = PASER_sign_len;
        u_int8_t *sign = (u_int8_t *) malloc(sizeof(u_int8_t) * sig_len);
        EVP_MD_CTX *md_ctx;
        md_ctx = EVP_MD_CTX_create();
        EVP_SignInit(md_ctx, EVP_sha1());
        int kdc_data_len = kdcData.GTK.len + sizeof(kdcData.nonce)
                + sizeof(kdcData.key_nr) + kdcData.CRL.len
                + kdcData.cert_kdc.len;
        u_int8_t *temp = (u_int8_t*) malloc(kdc_data_len);
        u_int8_t *tempData = temp;
        memcpy(temp, (u_int8_t *) kdcData.GTK.buf, kdcData.GTK.len);
        temp += kdcData.GTK.len;
        memcpy(temp, (u_int8_t *) &kdcData.nonce, sizeof(kdcData.nonce));
        temp += sizeof(kdcData.nonce);
        memcpy(temp, (u_int8_t *) &kdcData.key_nr, sizeof(kdcData.key_nr));
        temp += sizeof(kdcData.key_nr);
        memcpy(temp, (u_int8_t *) kdcData.CRL.buf, kdcData.CRL.len);
        temp += kdcData.CRL.len;
        memcpy(temp, (u_int8_t *) kdcData.cert_kdc.buf, kdcData.cert_kdc.len);
        temp += kdcData.cert_kdc.len;
        EVP_SignUpdate(md_ctx, tempData, kdc_data_len);
        free(tempData);
        int err = EVP_SignFinal(md_ctx, sign, &sig_len, pkey);
        EVP_MD_CTX_destroy(md_ctx);
        if (err != 1) {
            ERR_print_errors_fp(stderr);
#ifdef findCertError
            opp_error("signKDC");
#endif
            delete message;
            return;
        }
        kdcData.sign.buf = sign;
        kdcData.sign.len = sig_len;
        message_dellay += par("kdc_sign_mess_delay").doubleValue();


        u_int32_t sig_key_len = PASER_sign_len;
        u_int8_t *sign_key = (u_int8_t *) malloc(
                sizeof(u_int8_t) * sig_key_len);
        EVP_MD_CTX *md_key_ctx;
        md_key_ctx = EVP_MD_CTX_create();
        EVP_SignInit(md_key_ctx, EVP_sha1());
        int kdc_key_data_len = sizeof(kdcData.key_nr)
                + sizeof(kdcData.cert_kdc.len) + kdcData.cert_kdc.len;
        u_int8_t *temp_key = (u_int8_t*) malloc(kdc_key_data_len);
        u_int8_t *tempData_key = temp_key;
        memcpy(temp_key, (u_int8_t *) &kdcData.key_nr, sizeof(kdcData.key_nr));
        temp_key += sizeof(kdcData.key_nr);
        memcpy(temp_key, (u_int8_t *) &kdcData.cert_kdc.len,
                sizeof(kdcData.cert_kdc.len));
        temp_key += sizeof(kdcData.cert_kdc.len);
        memcpy(temp_key, (u_int8_t *) kdcData.cert_kdc.buf,
                kdcData.cert_kdc.len);
        temp_key += kdcData.cert_kdc.len;
        EVP_SignUpdate(md_key_ctx, tempData_key, kdc_key_data_len);
        free(tempData_key);
        int err_key = EVP_SignFinal(md_key_ctx, sign_key, &sig_key_len, pkey);
        EVP_MD_CTX_destroy(md_key_ctx);
        if (err_key != 1) {
            ERR_print_errors_fp(stderr);
#ifdef findCertError
            opp_error("signUURREP");
#endif
            delete message;
            return;
        }
        kdcData.sign_key.buf = sign_key;
        kdcData.sign_key.len = sig_key_len;
        message_dellay += par("kdc_sign_mess_delay").doubleValue();
//printf("sign_KDColdRESET:");
//for (int n = 0; n < sig_key_len; n++)
//    printf("%02x", sign_key[n]);
//putchar('\n');

        crl_message *message_to_send = new crl_message();
        message_to_send->setSrc(message->getSrc());
        // Initialize message
        // GTK
        message_to_send->setKdc_gtk_len(kdcData.GTK.len);
        message_to_send->setKdc_gtk_arrayArraySize(kdcData.GTK.len);
        for (u_int32_t i = 0; i < kdcData.GTK.len; i++) {
            message_to_send->setKdc_gtk_array(i, kdcData.GTK.buf[i]);
        }
        // Nonce
        message_to_send->setKdc_nonce(kdcData.nonce);
        // CRL
        message_to_send->setKdc_crl_len(kdcData.CRL.len);
        message_to_send->setKdc_crl_arrayArraySize(kdcData.CRL.len);
        for (u_int32_t i = 0; i < kdcData.CRL.len; i++) {
            message_to_send->setKdc_crl_array(i, kdcData.CRL.buf[i]);
        }
        // Certificate of KDC
        message_to_send->setKdc_cert_len(kdcData.cert_kdc.len);
        message_to_send->setKdc_cert_arrayArraySize(kdcData.cert_kdc.len);
        for (u_int32_t i = 0; i < kdcData.cert_kdc.len; i++) {
            message_to_send->setKdc_cert_array(i, kdcData.cert_kdc.buf[i]);
        }
        // Sign
        message_to_send->setKdc_sign_len(kdcData.sign.len);
        message_to_send->setKdc_sign_arrayArraySize(kdcData.sign.len);
        for (u_int32_t i = 0; i < kdcData.sign.len; i++) {
            message_to_send->setKdc_sign_array(i, kdcData.sign.buf[i]);
        }
        // Current key number
        message_to_send->setKdc_key_nr(kdcData.key_nr);
        // Signature private key
        message_to_send->setKdc_sign_key_len(kdcData.sign_key.len);
        message_to_send->setKdc_sign_key_arrayArraySize(kdcData.sign_key.len);
        for (u_int32_t i = 0; i < kdcData.sign_key.len; i++) {
            message_to_send->setKdc_sign_key_array(i, kdcData.sign_key.buf[i]);
        }
        message_to_send->setSrc(src);
        message_to_send->setGwAddr(message->getGwAddr());
        message_to_send->setNextHopAddr(message->getNextHopAddr());

//        sendToUDP(message_to_send, PORT_for_CRL_out, message->getGwAddr().S_addr.getIPv4(), PORT_for_CRL_out);
        scheduleAt(message_dellay + simTime().dbl(), message_to_send);

        free(kdcData.GTK.buf);
        free(kdcData.CRL.buf);
        free(kdcData.cert_kdc.buf);
        free(kdcData.sign.buf);
        free(kdcData.sign_key.buf);
        delete message;
    }
}

void SimpleKDC::setSocketOptions() {
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int typeOfService = par("typeOfService");
    if (typeOfService != -1)
        socket.setTypeOfService(typeOfService);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = InterfaceTableAccess().get(this);
        InterfaceEntry *ie = ift->getInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError(
                    "Wrong multicastInterface setting: no interface named \"%s\"",
                    multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups)
        socket.joinLocalMulticastGroups();
}

int SimpleKDC::checkOneCert(X509 *cert) {
    EV << "try to verify Cert\n";
    X509_STORE * ca_store = X509_STORE_new();
    if (X509_STORE_add_cert(ca_store, ca_cert) != 1) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_add_cert\n";
        return 0;
    }
    if (X509_STORE_set_default_paths(ca_store) != 1) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_set_default_paths\n";
        return 0;
    }
    if (crl) {
        EV << "verify Cert with CRL\n";
        if (X509_STORE_add_crl(ca_store, crl) != 1) {
            ERR_print_errors_fp (stderr);
            EV << "Error: X509_STORE_add_crl\n";
            return 0;
        }
#if (OPENSSL_VERSION_NUMBER > 0x00907000L)
        // Set the store flag so that CRLs are considered by the verification of certificates
        X509_STORE_set_flags(ca_store,
                X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#endif
    }

    X509_STORE_CTX *verify_ctx;
    // Create a verification context and initialize it
    if (!(verify_ctx = X509_STORE_CTX_new())) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_CTX_new\n";
        return 0;
    }
    // X509_STORE_CTX_init did not return an error condition in prior versions
#if (OPENSSL_VERSION_NUMBER > 0x00907000L)
    if (X509_STORE_CTX_init(verify_ctx, ca_store, cert, NULL) != 1) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_CTX_init\n";
        return 0;
    }
#else
    X509_STORE_CTX_init(verify_ctx, ca_store, cert, NULL);
#endif

    // Verify the certificate
    if (X509_verify_cert(verify_ctx) != 1) {
        ERR_print_errors_fp (stderr);
        EV << "cert: " << cert->name << "\n";
        EV << "Error: X509_verify_cert: "
                << X509_STORE_CTX_get_error(verify_ctx) << "\n";
        X509_STORE_free(ca_store);
        X509_STORE_CTX_free(verify_ctx);
        return 0;
    } else {
        X509_STORE_free(ca_store);
        X509_STORE_CTX_free(verify_ctx);
        return 1;
    }

    EV << "Error: \n";
    return 0;
}

X509* SimpleKDC::extractCert(lv_block cert) {
    if (cert.buf == NULL) {
        return NULL;
    }
    X509 *x;
    const u_int8_t *buf, *p;
    int len;
    buf = cert.buf;
    len = cert.len;
    p = buf;
    x = d2i_X509(NULL, &p, len);
    if (x == NULL) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return x;
}

int SimpleKDC::getCRL(lv_block *cert) {
    int len;
    unsigned char *buf;
    buf = NULL;
    len = i2d_X509_CRL(crl, &buf);
    if (len < 0) {
        return 0;
    }
    u_int8_t *buf_cert = (u_int8_t *) malloc(sizeof(u_int8_t) * len);
    memcpy(buf_cert, buf, (sizeof(u_int8_t) * len));
    cert->buf = buf_cert;
    cert->len = len;
#ifdef __unix__
    free(buf);
#endif
    return 1;
}

int SimpleKDC::saveCertAlsBlock() {
    int len;
    unsigned char *buf;
    buf = NULL;
    len = i2d_X509(x509, &buf);
    if (len < 0) {
        return 0;
    }
    u_int8_t *buf_cert = (u_int8_t *) malloc(sizeof(u_int8_t) * len);
    memcpy(buf_cert, buf, (sizeof(u_int8_t) * len));
    cert_als_block.buf = buf_cert;
    cert_als_block.len = len;
#ifdef __unix__
    free(buf);
#endif
    return 1;
}

int SimpleKDC::rsa_encrypt(lv_block in, lv_block *out, X509 *cert) {
    EVP_PKEY *pubKey;
    if (cert != NULL) {
        pubKey = X509_get_pubkey(cert);
    } else {
        pubKey = X509_get_pubkey(x509);
    }

    RSA *rsa = EVP_PKEY_get1_RSA(pubKey);
    out->buf = (u_int8_t *) malloc(RSA_size(rsa));
    int i = RSA_public_encrypt(in.len, in.buf, out->buf, rsa,
            RSA_PKCS1_PADDING);
    out->len = i;
    EVP_PKEY_free(pubKey);
    RSA_free(rsa);
    if (i < 0) {
        return 0;
    }
//            printf("rsa_encrypt%d : \nWERT GTK:0x",i);
//            for (int n = 0; n < out->len; n++)
//                printf("%02x", out->buf[n]);
//            putchar('\n');
    return 1;
}
#endif
