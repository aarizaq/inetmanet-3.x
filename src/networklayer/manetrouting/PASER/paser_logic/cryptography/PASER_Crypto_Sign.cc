/*
 *\class       PASER_Crypto_Sign
 *@brief       Class provides functions to compute and check signatures of PASER messages
 *@ingroup     Cryptography
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
#include <openssl/pem.h>
#include "PASER_Crypto_Sign.h"
#include "PASER_Definitions.h"

#include <stdio.h>
#include <list>
#include <compatibility.h>

#ifndef __unix__
extern"C"
{
#include<openssl/applink.c>
}
#endif

//#define findCertError

void PASER_Crypto_Sign::init(char *certPath, char *keyPath, char *CAcertPath) {
//    return;
//justForTest();
    EV << "certfile: " << certPath << "\n";
    EV << "keyfile:  " << keyPath << "\n";
    FILE *fp;

    fp = fopen(keyPath, "r");
    if (fp == NULL) {
        printf("keyfile not found: %s", keyPath);
        exit(1);
    }
    pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    /* Read cert */
    fp = fopen(certPath, "r");
    if (fp == NULL) {
        printf("certfile not found: %s", certPath);
        exit(1);
    }
//    char cstring[256];
//    while ( ! feof (fp) ){
//        fgets (cstring , 256 , fp);
//        fputs (cstring , stdout);
//    }
//    fseek ( fp , 0 , SEEK_SET );

//    x509=X509_new();
    x509 = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);
    if (x509 == NULL) {
        ERR_print_errors_fp (stderr);
        exit(1);
    }

    //read CA_file
    fp = fopen(CAcertPath, "r");
    if (fp == NULL) {
        printf("CAcertPath not found: %s", CAcertPath);
        exit(1);
    }
    ca_cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);
    if (ca_cert == NULL) {
        ERR_print_errors_fp (stderr);
        exit(1);
    }

    crl = NULL;
//    ca_store = X509_STORE_new();
//    if(X509_STORE_add_cert(ca_store, ca_cert)!=1){
//        ERR_print_errors_fp (stderr);
//        exit (1);
//    }

}

PASER_Crypto_Sign::~PASER_Crypto_Sign() {
    if (x509) {
        X509_free(x509);
    }
    x509 = NULL;
    if (ca_cert) {
        X509_free(ca_cert);
    }
    ca_cert = NULL;
    if (pkey) {
        EVP_PKEY_free(pkey);
    }
    if (crl) {
        X509_CRL_free(crl);
    }
}

int PASER_Crypto_Sign::getCert(lv_block *cert) {
    int len;
    unsigned char *buf;
    buf = NULL;
    len = i2d_X509(x509, &buf);
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

int PASER_Crypto_Sign::setCRL(lv_block in) {
    if (in.buf == NULL) {
        return 0;
    }
    X509_CRL *x;
    const u_int8_t *buf, *p;
    int len;
    buf = in.buf;
    len = in.len;
    p = buf;
    x = d2i_X509_CRL(NULL, &p, len);
    if (x == NULL) {
        ERR_print_errors_fp(stderr);
        return 0;
    }
    crl = x;
    return 1;
}

int PASER_Crypto_Sign::signUBRREQ(PASER_UB_RREQ * message) {
    u_int32_t sig_len = PASER_sign_len;
    u_int8_t *sign = (u_int8_t *) malloc(sizeof(u_int8_t) * sig_len);
    EVP_MD_CTX *md_ctx;
    md_ctx = EVP_MD_CTX_create();
    EVP_SignInit(md_ctx, EVP_sha1());

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    EV << "message.len = " << len << "\n";
    EVP_SignUpdate(md_ctx, data, len);
    free(data);

    int err = EVP_SignFinal(md_ctx, sign, &sig_len, pkey);
    EVP_MD_CTX_destroy(md_ctx);
    if (err != 1) {
        ERR_print_errors_fp(stderr);
#ifdef findCertError
        opp_error("signUBRREQ");
#endif
        return 0;
    }
    if (message->sign.buf != NULL) {
        free(message->sign.buf);
    }
    message->sign.buf = sign;
    message->sign.len = sig_len;
    EV << "sign.len = " << sig_len << "\n";
    return 1;
}

int PASER_Crypto_Sign::checkSignUBRREQ(PASER_UB_RREQ * message) {
    X509 *x;
    const u_int8_t *buf, *p;
    int len;
    buf = message->certForw.buf;
    len = message->certForw.len;
    p = buf;
    x = d2i_X509(NULL, &p, len);
    if (x == NULL) {
        ERR_print_errors_fp(stderr);
        return 0;
    }
    if (checkOneCert(x) != 1) {
        X509_free(x);
        EV << "Certificate ERROR!";
        return 0;
    }
//    EV << "found cert name:\n " << x->name << "\n";
    EVP_PKEY *pubKey = X509_get_pubkey(x);
    X509_free(x);
    u_int32_t sig_len = message->sign.len;
    EV << "sign.len = " << sig_len << "\n";
    u_int8_t *sign = message->sign.buf;
    EVP_MD_CTX *md_ctx;
    md_ctx = EVP_MD_CTX_create();
    EVP_VerifyInit(md_ctx, EVP_sha1());

    int message_len = 0;
    u_int8_t *data = message->toByteArray(&message_len);
    EV << "message.len = " << message_len << "\n";
    EVP_VerifyUpdate(md_ctx, data, message_len);
    free(data);

    int err = EVP_VerifyFinal(md_ctx, sign, sig_len, pubKey);
    EV << " verifyFinal"<<err<< "\n";
    EVP_PKEY_free(pubKey);
    EVP_MD_CTX_destroy(md_ctx);

    if (err != 1) {
        EV << "EVP_VerifyFinal error\n";
        ERR_print_errors_fp (stderr);
#ifdef findCertError
        opp_error("checkSignUBRREQ");
#endif
        return 0;
    }
    return 1;
}

int PASER_Crypto_Sign::signUURREP(PASER_UU_RREP * message) {
    u_int32_t sig_len = PASER_sign_len;
    u_int8_t *sign = (u_int8_t *) malloc(sizeof(u_int8_t) * sig_len);
    EVP_MD_CTX *md_ctx;
    md_ctx = EVP_MD_CTX_create();
    EVP_SignInit(md_ctx, EVP_sha1());

    int message_len = 0;
    u_int8_t *data = message->toByteArray(&message_len);
    EVP_SignUpdate(md_ctx, data, message_len);
  //    u_int8_t *data = (u_int8_t *) malloc(4);
 //      memset(data, 0xEE, 4);
  //    EVP_SignUpdate(md_ctx, data, 4);

    int err = EVP_SignFinal(md_ctx, sign, &sig_len, pkey);
    EVP_MD_CTX_destroy(md_ctx);
    if (err != 1) {
        ERR_print_errors_fp(stderr);
#ifdef findCertError
        opp_error("signUURREP");
#endif
        return 0;
    }
    if (message->sign.buf != NULL) {
        free(message->sign.buf);
    }
    message->sign.buf = sign;
    message->sign.len = sig_len;

////printf("SING:signleng: %d\nsign:0x",sig_len);
//printf("Sign-sign:0x");
//for (int n = 0; n < sig_len; n++)
//    printf("%02x", sign[n]);
//putchar('\n');
////printf("pack-ver: %d\nsign:0x",message_len);
//printf("sign-pack:0x");
//for (int n = 0; n < message_len; n++)
//    printf("%02x", data[n]);
//putchar('\n');
//int message_len_full = 0;
//u_int8_t *data_full = message->getCompleteByteArray(&message_len_full);
////printf("FULL-gen: %d\nsign:0x",message_len_full);
//printf("sign-full:0x");
//for (int n = 0; n < message_len_full; n++)
//    printf("%02x", data_full[n]);
//putchar('\n');
//free(data_full);
    free(data);
    return 1;
}

int PASER_Crypto_Sign::checkSignUURREP(PASER_UU_RREP * message) {
    X509 *x;
    const u_int8_t *buf, *p;
    int len;
    buf = message->certForw.buf;
    len = message->certForw.len;
    p = buf;
    x = d2i_X509(NULL, &p, len);
    if (x == NULL) {
        ERR_print_errors_fp(stderr);
        return 0;
    }
    if (checkOneCert(x) != 1) {
        X509_free(x);
        EV << "Certificate ERROR!";
        return 0;
    }
//    EV << "found cert name:\n " << x->name << "\n";
    EVP_PKEY *pubKey = X509_get_pubkey(x);
    X509_free(x);
    u_int32_t sig_len = message->sign.len;
    u_int8_t *sign = message->sign.buf;
    EVP_MD_CTX *md_ctx;
    md_ctx = EVP_MD_CTX_create();
    EVP_VerifyInit(md_ctx, EVP_sha1());

     int message_len = 0;
     u_int8_t *data = message->toByteArray(&message_len);
    // EVP_SignUpdate(md_ctx, data, message_len);
     //u_int8_t *data = (u_int8_t *) malloc(4);
    //memset(data, 0xEE, 4);
    EVP_VerifyUpdate(md_ctx, data, message_len);

////printf("VERI:signleng: %d\nveri:0x",sig_len);
//printf("VERI-sign:0x");
//for (int n = 0; n < sig_len; n++)
//    printf("%02x", sign[n]);
//putchar('\n');
////printf("pack-ver: %d\nveri:0x",message_len);
//printf("veri-pack:0x");
//for (int n = 0; n < message_len; n++)
//    printf("%02x", data[n]);
//putchar('\n');
//int message_len_full = 0;
//u_int8_t *data_full = message->getCompleteByteArray(&message_len_full);
////printf("FULL-ver: %d\nveri:0x",message_len_full);
//printf("veri-full:0x");
//for (int n = 0; n < message_len_full; n++)
//    printf("%02x", data_full[n]);
//putchar('\n');
//free(data_full);
    free(data);
    int err = EVP_VerifyFinal(md_ctx, sign, sig_len, pubKey);
    EVP_PKEY_free(pubKey);
    EVP_MD_CTX_destroy(md_ctx);

    if (err != 1) {
//printf("EVP_VerifyFinal error\n");
        EV << "EVP_VerifyFinal error\n";
        ERR_print_errors_fp (stderr);
#ifdef findCertError
        opp_error("checkSignUURREP");
#endif
        return 0;
    }
    return 1;
}

int PASER_Crypto_Sign::signB_ROOT(PASER_UB_Root_Refresh * message) {
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
#ifdef findCertError
        opp_error("signB_ROOT");
#endif
        return 0;
    }
    if (message->sign.buf != NULL) {
        free(message->sign.buf);
    }
    message->sign.buf = sign;
    message->sign.len = sig_len;
    free(data);
    return 1;
}

int PASER_Crypto_Sign::checkSignB_ROOT(PASER_UB_Root_Refresh * message) {
    X509 *x;
    const u_int8_t *buf, *p;
    int len;
    buf = message->cert.buf;
    len = message->cert.len;
    p = buf;
    x = d2i_X509(NULL, &p, len);
    if (x == NULL) {
        ERR_print_errors_fp(stderr);
        return 0;
    }
    if (checkOneCert(x) != 1) {
        X509_free(x);
        EV << "Certificate ERROR!";
        return 0;
    }
//    EV << "found cert name:\n " << x->name << "\n";
    EVP_PKEY *pubKey = X509_get_pubkey(x);
    X509_free(x);
    u_int32_t sig_len = message->sign.len;
    u_int8_t *sign = message->sign.buf;
    EVP_MD_CTX *md_ctx;
    md_ctx = EVP_MD_CTX_create();
    EVP_VerifyInit(md_ctx, EVP_sha1());

    int message_len = 0;
    u_int8_t *data = message->toByteArray(&message_len);
    EVP_VerifyUpdate(md_ctx, data, message_len);
    free(data);
    int err = EVP_VerifyFinal(md_ctx, sign, sig_len, pubKey);
    EVP_PKEY_free(pubKey);
    EVP_MD_CTX_destroy(md_ctx);

    if (err != 1) {
//printf("EVP_VerifyFinal error\n");
        EV << "EVP_VerifyFinal error\n";
        ERR_print_errors_fp (stderr);
#ifdef findCertError
        opp_error("checkSignB_ROOT");
#endif
        return 0;
    }
    return 1;
}

//int PASER_Crypto_Sign::signRESET(PASER_UB_Key_Refresh * message){
//    u_int32_t sig_len = PASER_sign_len;
//    u_int8_t *sign = (u_int8_t *)malloc(sizeof(u_int8_t) * sig_len);
//    EVP_MD_CTX *md_ctx;
//    md_ctx = EVP_MD_CTX_create();
//    EVP_SignInit   (md_ctx, EVP_sha1());
//
//    int message_len = 0;
//    u_int8_t *data = message->toByteArray(&message_len);
//    EVP_SignUpdate(md_ctx, data, message_len);
//
//    int err = EVP_SignFinal (md_ctx, sign, &sig_len, pkey);
//    EVP_MD_CTX_destroy(md_ctx);
//    if (err != 1) {
//        ERR_print_errors_fp(stderr);
//#ifdef findCertError
//        opp_error("signB_ROOT");
//#endif
//        return 0;
//    }
//    if(message->sign.buf != NULL){
//        free(message->sign.buf);
//    }
//    message->sign.buf = sign;
//    message->sign.len = sig_len;
//    free(data);
//    return 1;
//}

int PASER_Crypto_Sign::checkSignRESET(PASER_UB_Key_Refresh * message) {
    X509 *x;
    const u_int8_t *buf, *p;
    int len;
    buf = message->cert.buf;
    len = message->cert.len;
    p = buf;
    x = d2i_X509(NULL, &p, len);
    if (x == NULL) {
        ERR_print_errors_fp(stderr);
        return 0;
    }
    if (checkOneCert(x) != 1) {
        X509_free(x);
        EV << "Certificate ERROR!";
        return 0;
    }
//    EV << "found cert name:\n " << x->name << "\n";
    EVP_PKEY *pubKey = X509_get_pubkey(x);
    X509_free(x);
    u_int32_t sig_len = message->sign.len;
    u_int8_t *sign = message->sign.buf;
    EVP_MD_CTX *md_ctx;
    md_ctx = EVP_MD_CTX_create();
    EVP_VerifyInit(md_ctx, EVP_sha1());
//printf("sign_getRE_Message:");
//for (int n = 0; n < sig_len; n++)
//    printf("%02x", sign[n]);
//putchar('\n');

    int message_len = 0;
    u_int8_t *data = message->toByteArray(&message_len);
    EVP_VerifyUpdate(md_ctx, data, message_len);
    free(data);
    int err = EVP_VerifyFinal(md_ctx, sign, sig_len, pubKey);
    EVP_PKEY_free(pubKey);
    EVP_MD_CTX_destroy(md_ctx);

    if (err != 1) {
//printf("EVP_VerifyFinal error\n");
        EV << "EVP_VerifyFinal error\n";
        ERR_print_errors_fp (stderr);
#ifdef findCertError
        opp_error("checkSignB_ROOT");
#endif
        return 0;
    }
    return 1;
}

int PASER_Crypto_Sign::checkSignKDC(kdc_block data) {
    //read CRL
    if (data.CRL.buf == NULL) {
        return 0;
    }
    X509_CRL *crl_x;
    const u_int8_t *buf, *p;
    int len;
    buf = data.CRL.buf;
    len = data.CRL.len;
    p = buf;
    crl_x = d2i_X509_CRL(NULL, &p, len);
    if (crl_x == NULL) {
        ERR_print_errors_fp(stderr);
        return 0;
    }

    //read KDC cert
    X509* kdc_cert = extractCert(data.cert_kdc);
    if (kdc_cert == NULL) {
        EV << "kein KDC Zertifikat\n";
        X509_CRL_free(crl_x);
        return 0;
    }

    //check KDC cert
    EV << "try to verify Cert\n";
    X509_STORE *ca_store;
    ca_store = X509_STORE_new();
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
    if (crl_x) {
        EV << "verify Cert with CRL\n";
        if (X509_STORE_add_crl(ca_store, crl_x) != 1) {
            ERR_print_errors_fp (stderr);
            EV << "Error: X509_STORE_add_crl\n";
            return 0;
        }
#if (OPENSSL_VERSION_NUMBER > 0x00907000L)
        //set the flag of the store so that CRLs are consulted
        X509_STORE_set_flags(ca_store,
                X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#endif
    }

    X509_STORE_CTX *verify_ctx;
    //create a verification context and initialize it
    if (!(verify_ctx = X509_STORE_CTX_new())) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_CTX_new\n";
        return 0;
    }
    //X509_STORE_CTX_init did not return an error condition in prior versions
#if (OPENSSL_VERSION_NUMBER > 0x00907000L)
    if (X509_STORE_CTX_init(verify_ctx, ca_store, kdc_cert, NULL) != 1) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_CTX_init\n";
        return 0;
    }
#else
    X509_STORE_CTX_init(verify_ctx, ca_store, kdc_cert, NULL);
#endif

    //verify the certificate
    if (X509_verify_cert(verify_ctx) != 1) {
        ERR_print_errors_fp (stderr);
        EV << "cert: " << kdc_cert->name << "\n";
        EV << "Error: X509_verify_cert: "
                << X509_STORE_CTX_get_error(verify_ctx) << "\n";
        X509_STORE_free(ca_store);
        X509_STORE_CTX_free(verify_ctx);
        return 0;
    } else {
        X509_STORE_free(ca_store);
        X509_STORE_CTX_free(verify_ctx);
    }

    //check KDC Sign
    EVP_PKEY *pubKey = X509_get_pubkey(kdc_cert);
    X509_free(kdc_cert);
    u_int32_t sig_len = data.sign.len;
    u_int8_t *sign = data.sign.buf;
    EVP_MD_CTX *md_ctx;
    md_ctx = EVP_MD_CTX_create();
    EVP_VerifyInit(md_ctx, EVP_sha1());

    EV << "nonce: " << data.nonce << "\n";
    int kdc_data_len = data.GTK.len + sizeof(data.nonce) + sizeof(data.key_nr)
            + data.CRL.len + data.cert_kdc.len;
    u_int8_t *temp = (u_int8_t*) malloc(kdc_data_len);
    u_int8_t *tempData = temp;
    memcpy(temp, (u_int8_t *) data.GTK.buf, data.GTK.len);
    temp += data.GTK.len;
    memcpy(temp, (u_int8_t *) &data.nonce, sizeof(data.nonce));
    temp += sizeof(data.nonce);
    memcpy(temp, (u_int8_t *) &data.key_nr, sizeof(data.key_nr));
    temp += sizeof(data.key_nr);
    memcpy(temp, (u_int8_t *) data.CRL.buf, data.CRL.len);
    temp += data.CRL.len;
    memcpy(temp, (u_int8_t *) data.cert_kdc.buf, data.cert_kdc.len);
    temp += data.cert_kdc.len;
    EVP_VerifyUpdate(md_ctx, tempData, kdc_data_len);

//printf("sign-check:0x");
//for (int n = 0; n < kdc_data_len; n++)
//    printf("%02x", tempData[n]);
//putchar('\n');
    free(tempData);

    int err = EVP_VerifyFinal(md_ctx, sign, sig_len, pubKey);
    EVP_PKEY_free(pubKey);
    EVP_MD_CTX_destroy(md_ctx);

    if (err != 1) {
        EV << "checkKDC: EVP_VerifyFinal error\n";
        ERR_print_errors_fp (stderr);
#ifdef findCertError
        opp_error("checkSignUBRREQ");
#endif
        return 0;
    }
    //aktualisiere CRL
    if (crl) {
        X509_CRL_free(crl);
    }
    crl = crl_x;
    return 1;
}

X509* PASER_Crypto_Sign::extractCert(lv_block cert) {
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

u_int8_t PASER_Crypto_Sign::isGwCert(X509 *cert) {
    EV << "isGwCert\n";
    if (cert == NULL) {
        EV << "cert = NULL\n";
        return 0x00;
    }
    ASN1_IA5STRING *nscomment;
    nscomment = (ASN1_IA5STRING *) X509_get_ext_d2i(cert, NID_netscape_comment,
            NULL, NULL);
    if (nscomment == NULL) {
        EV << "nscomment = NULL\n";
        return 0x00;
    }
    EV << "comment: " << nscomment->data << "\n";
    if (memcmp(nscomment, "gateway:true", 13)) {
        EV << "isGwCert: true\n";
        ASN1_IA5STRING_free(nscomment);
        return 0x01;
    }
    EV << "isGwCert: false\n";

    return 0x00;
}

u_int8_t PASER_Crypto_Sign::isKdcCert(X509 *cert) {
    EV << "isKdcCert\n";
    if (cert == NULL) {
        EV << "cert = NULL\n";
        return 0x00;
    }
    ASN1_IA5STRING *nscomment;
    nscomment = (ASN1_IA5STRING *) X509_get_ext_d2i(cert, NID_netscape_comment,
            NULL, NULL);
    if (nscomment == NULL) {
        EV << "nscomment = NULL\n";
        return 0x00;
    }
    EV << "comment: " << nscomment->data << "\n";
    if (memcmp(nscomment, "kdc:true", 9)) {
        EV << "isKdcCert: true\n";
        ASN1_IA5STRING_free(nscomment);
        return 0x01;
    }
    EV << "isKdcCert: false\n";

    return 0x00;
}

int PASER_Crypto_Sign::rsa_encrypt(lv_block in, lv_block *out, X509 *cert) {
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

int PASER_Crypto_Sign::rsa_dencrypt(lv_block in, lv_block *out) {
    if (out->buf != NULL && out->len > 0) {
        free(out->buf);
        out->len = 0;
    }
    RSA *rsa = EVP_PKEY_get1_RSA(pkey);
    u_int8_t *buf = (u_int8_t *) malloc(RSA_size(rsa));
    int i = RSA_private_decrypt(in.len, in.buf, buf, rsa, RSA_PKCS1_PADDING);
    if (i < 0) {
        free(buf);
        RSA_free(rsa);
        return 0;
    }
    out->len = i;
    out->buf = (u_int8_t *) malloc(i);
    memcpy(out->buf, buf, i);
    free(buf);
    RSA_free(rsa);
    return 1;
}

int PASER_Crypto_Sign::checkOneCert(X509 *cert) {
    EV << "try to verify Cert\n";
    X509_STORE *ca_store;
    ca_store = X509_STORE_new();
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
        //set the flag of the store so that CRLs are consulted
        X509_STORE_set_flags(ca_store,
                X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#endif
    }

    X509_STORE_CTX *verify_ctx;
    //create a verification context and initialize it
    if (!(verify_ctx = X509_STORE_CTX_new())) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_CTX_new\n";
        return 0;
    }
    //X509_STORE_CTX_init did not return an error condition in prior versions
#if (OPENSSL_VERSION_NUMBER > 0x00907000L)
    if (X509_STORE_CTX_init(verify_ctx, ca_store, cert, NULL) != 1) {
        ERR_print_errors_fp (stderr);
        EV << "Error: X509_STORE_CTX_init\n";
        return 0;
    }
#else
    X509_STORE_CTX_init(verify_ctx, ca_store, cert, NULL);
#endif

    //verify the certificate
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

int PASER_Crypto_Sign::checkAllCertInRoutingTable(
        PASER_Routing_Table *routing_table,
        PASER_Neighbor_Table *neighbor_table, PASER_Timer_Queue *timer_queue) {
    int i = 0;
    bool found = true;
    while (found) {
        found = false;
        int j = 0;
        for (std::map<ManetAddress, PASER_Neighbor_Entry *>::iterator it =
                neighbor_table->neighbor_table_map.begin();
                it != neighbor_table->neighbor_table_map.end(); it++) {
            if (j >= i) {
                PASER_Neighbor_Entry *nEntry = it->second;
                if (nEntry->Cert) {
                    if (checkOneCert((X509*) nEntry->Cert) == 0) {
                        found = true;
                        //delete neighbor
                        neighbor_table->delete_entry(nEntry);
                        if (nEntry->deleteTimer) {
                            EV << "delete nEntry->deleteTimer\n";
                            timer_queue->timer_remove(nEntry->deleteTimer);
                            delete nEntry->deleteTimer;
                        }
                        if (nEntry->validTimer) {
                            EV << "delete nEntry->validTimer\n";
                            timer_queue->timer_remove(nEntry->validTimer);
                            delete nEntry->validTimer;
                        }
//                        //delete route
//                        PASER_Routing_Entry *rEntry = routing_table->findDest(nEntry->neighbor_addr);
//                        if(rEntry){
//                            routing_table->delete_entry(rEntry);
//                            if(rEntry->deleteTimer){
//                                EV << "delete rEntry->deleteTimer\n";
//                                timer_queue->timer_remove(rEntry->deleteTimer);
//                                delete rEntry->deleteTimer;
//                            }
//                            if(rEntry->validTimer){
//                                EV << "delete rEntry->validTimer\n";
//                                timer_queue->timer_remove(rEntry->validTimer);
//                                delete rEntry->validTimer;
//                            }
//                            delete rEntry;
//                        }
                        //delete all routes
                        std::list<PASER_Routing_Entry*> routeList =
                                routing_table->getListWithNextHop(
                                        nEntry->neighbor_addr);
                        for (std::list<PASER_Routing_Entry*>::iterator it2 =
                                routeList.begin(); it2 != routeList.end();
                                it2++) {
                            PASER_Routing_Entry *tempEntry =
                                    (PASER_Routing_Entry*) *it2;
                            if (tempEntry) {
                                routing_table->delete_entry(tempEntry);
                                if (tempEntry->deleteTimer) {
                                    EV << "delete rEntry->deleteTimer\n";
                                    timer_queue->timer_remove(
                                            tempEntry->deleteTimer);
                                    delete tempEntry->deleteTimer;
                                }
                                if (tempEntry->validTimer) {
                                    EV << "delete rEntry->validTimer\n";
                                    timer_queue->timer_remove(
                                            tempEntry->validTimer);
                                    delete tempEntry->validTimer;
                                }
                                delete tempEntry;
                            }
                        }
                        delete nEntry;
                        break;
                    }
                }
            }
            i++;
            j++;
        }
    }
    PASER_Routing_Entry *routeToGW = routing_table->getRouteToGw();
    if (routeToGW) {
        return 1;
    }
    return 0;
}

//void PASER_Crypto_Sign::justForTest(){
//    OpenSSL_add_all_algorithms();
//    ERR_load_crypto_strings();
//
//    FILE *fp;
//
//    X509_STORE * CAcerts;
//    X509 * cert;
//
//    X509_STORE_CTX ca_ctx;
//    char *strerr;
//
//    /* load CA cert store */
//    if (!(CAcerts = X509_STORE_new())) {
//        EV << "MEGAERROR: X509_STORE_new\n";
//    }
//    if (X509_STORE_load_locations(CAcerts, "cert/cacert.pem", NULL) != 1) {
//        EV << "MEGAERROR: X509_STORE_load_locations\n";
//    }
//    if (X509_STORE_set_default_paths(CAcerts) != 1) {
//        EV << "MEGAERROR: X509_STORE_set_default_paths\n";
//    }
//
//    /* load X509 certificate */
//    if (!(fp = fopen("cert/router.pem", "r"))) {
//        EV << "MEGAERROR: fopen\n";
//    }
//    if (!(cert = PEM_read_X509(fp, NULL, NULL, NULL))) {
//        EV << "MEGAERROR: PEM_read_X509\n";
//    }
//    fclose(fp);
//
//    /* verify */
//    if (X509_STORE_CTX_init(&ca_ctx, CAcerts, cert, NULL) != 1) {
//        EV << "MEGAERROR: X509_STORE_CTX_init\n";
//    }
//
//    if (X509_verify_cert(&ca_ctx) != 1) {
//        strerr = (char *) X509_verify_cert_error_string(ca_ctx.error);
//        printf("Verification error: %s\n", strerr);
//        EV << "MEGAERROR: X509_verify_cert " << strerr << "\n";
//    }
//    else{
//        EV << "Verification OK\n";
//    }
//
//    X509_STORE_free(CAcerts);
//    X509_free(cert);
//}
#endif
