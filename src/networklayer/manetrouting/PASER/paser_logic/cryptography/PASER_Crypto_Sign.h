/**
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
class PASER_Crypto_Sign;

#ifndef PASER_CRYPTO_SIGN_H_
#define PASER_CRYPTO_SIGN_H_


#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include "PASER_Definitions.h"
#include "PASER_UB_RREQ.h"
#include "PASER_UU_RREP.h"
#include "PASER_UB_Root_Refresh.h"
#include "PASER_UB_Key_Refresh.h"
#include "PASER_Routing_Table.h"
#include "PASER_Neighbor_Table.h"
#include "PASER_Timer_Queue.h"

#include "compatibility.h"

#include <map>

class PASER_Crypto_Sign {
private:
    EVP_PKEY *pkey; ///< Asymmetric private key
    X509 *x509; ///< Own certificate
    X509 *ca_cert; ///< CA certificate
//    X509_STORE *ca_store;   //
    X509_CRL *crl; ///< Certificate Revocation List

public:
    ~PASER_Crypto_Sign();

    /**
     * Initialization of PASER_Crypto_Sign Object. The function loads
     * own certificate, asymmetric private key and CA certificate.!!!
     *
     *@param certPath Path to the own certificate
     *@param keyPath Path to the asymmetric private key
     *@param CAcertPath Path to the CA certificate
     *
     */
    void init(char *certPath, char *keyPath, char *CAcertPath);

    /**
     * Get own certificate as char array
     *
     *@param cert Pointer to the lv_block which will be written
     *
     *@return 1 if successful or 0 on error
     */
    int getCert(lv_block *cert);

    /**
     * Extract a X509 certificate from acchar array
     *
     *@param cert lv_block which contains both, the certificate and the char array
     *
     *@return Pointer to certificate if successful or NULL on error
     */
    X509* extractCert(lv_block cert);

    /**
     * Compute a signature of a PASER_UB_RREQ message and
     * append the signature to the message
     *
     *@param message Pointer to the PASER_TU_RREQ message
     *
     *@return 1 if successful or 0 on error
     */
    int signUBRREQ(PASER_UB_RREQ * message);

    /**
     * Check the signature of a PASER_UB_RREQ message
     *
     *@param message Pointer to the PASER_TU_RREQ message
     *
     *@return 1 if successful or 0 on error
     */
    int checkSignUBRREQ(PASER_UB_RREQ * message);

    /**
     * Check the signature of a KDC_block message
     *
     *@param data KDC_block
     *
     *@return 1 if successful or 0 on error
     */
    int checkSignKDC(kdc_block data);

    /**
     * Compute the signature of a PASER_UU_RREP message and
     * append the signature to the message
     *
     *@param message Pointer to the PASER_UU_RREP message
     *@return 1 if successful or 0 on error
     */
    int signUURREP(PASER_UU_RREP * message);
    /**
     * Check the signature of a PASER_UU_RREP message
     *
     *@param message Pointer to the PASER_UU_RREP message
     *
     *@return 1 if successful or 0 on error
     */
    int checkSignUURREP(PASER_UU_RREP * message);

    /**
     * Compute the signature of a PASER_UB_Root_Refresh message and
     * append the signature to the message
     *
     *@param message Pointer to the PASER_UB_Root_Refresh message
     *
     *@return 1 if successful or 0 on error
     */
    int signB_ROOT(PASER_UB_Root_Refresh * message);
    /**
     * Check the signature of a PASER_UB_Root_Refresh message
     *
     *@param message Pointer to the PASER_UB_Root_Refresh message
     *
     *@return 1 if successful or 0 on error
     */
    int checkSignB_ROOT(PASER_UB_Root_Refresh * message);

    /**
     * Check the signature of a PASER_UB_Key_Refresh message
     *
     *@param message Pointer to the PASER_UB_Key_Refresh message
     *
     *@return 1 if successful or 0 on error
     */
    int checkSignRESET(PASER_UB_Key_Refresh * message);

    /**
     * Check if a certificate belongs to the Gateway
     *
     *@param cert Pointer to the certificate
     *
     *@return 1 if successful or 0 on error
     */
    u_int8_t isGwCert(X509 *cert);

    /**
     * Check if a certificate belongs to KDC
     *
     *@param cert Pointer to the certificate
     *
     *@return 1 if successful or 0 on error
     */
    u_int8_t isKdcCert(X509 *cert);

    /**
     * Encrypt a char array with a public key of given certificate
     *
     *@param in Char array to be encrypted
     *@param out Pointer to the encrypted char array
     *@param cert Pointer to the certificate
     *
     *@return 1 on successful or 0 on error
     */
    int rsa_encrypt(lv_block in, lv_block *out, X509 *cert);

    /**
     * Decrypt a char array with a own asymmetric private key
     *
     *@param in Char array to be decrypted
     *@param out Pointer to the decrypted char array!!!
     *
     *@return 1 on successful or 0 on error
     */
    int rsa_dencrypt(lv_block in, lv_block *out);

    /**
     * Set CRL from char array!!!
     *
     *@param in Char array which contains CRL
     *
     *@return 1 if successful or 0 on error
     */
    int setCRL(lv_block in);

    /**
     * Check whether a certificate is valid
     *
     *@param cert Pointer to the certificate
     *
     *@return 1 if successful or 0 on error
     */
    int checkOneCert(X509 *cert);

    /**
     * The function checks whether all certificates in routing and neighbor tables are valid.
     * All routes that correspond to an invalid certificate will be deleted.
     *
     *@param routing_table Pointer to the routing table object
     *@param neighbor_table Pointer to the neighbor table object
     *@param timer_queue Pointer to the timer queue
     *
     *@return 1 if a valid route to a gateway exist. Else 0.
     */
    int checkAllCertInRoutingTable(PASER_Routing_Table *routing_table,
            PASER_Neighbor_Table *neighbor_table,
            PASER_Timer_Queue *timer_queue);

};

#endif /* PASER_CRYPTO_SIGN_H_ */
#endif
