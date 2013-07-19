/**
 *\class       PASER_Crypto_Hash
 *@brief       Class provides functions to compute and check the hash value of PASER messages
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
#ifndef PASER_CRYPTO_HASH_H_
#define PASER_CRYPTO_HASH_H_

#include <openssl/engine.h>
#include "PASER_Definitions.h"
#include <omnetpp.h>


#include "PASER_TU_RREQ.h"
#include "PASER_TU_RREP_ACK.h"
#include "PASER_TU_RREP.h"
#include "PASER_TB_RERR.h"
#include "PASER_TB_Hello.h"

class PASER_Crypto_Hash {
    /**
     * @brief Initialize Object
     */
    void init();

public:
    /**
     * @brief Compute a hash value of the PASER_TU_RREQ message using GTK and
     * append this value to the message
     *
     *@param message Pointer to the PASER_TU_RREQ message the hash value of which will be computed
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int computeHmacTURREQ(PASER_TU_RREQ * message, lv_block GTK);

    /**
     * @brief Check the hash value of a PASER_TU_RREQ value
     *
     *@param message Pointer to the PASER_TU_RREQ the hash value of which will be verified
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int checkHmacTURREQ(PASER_TU_RREQ * message, lv_block GTK);

    /**
     * @brief Compute a hash value of a PASER_TU_RREP message using GTK and
     * append this value to the message
     *
     *@param message Pointer to the PASER_TU_RREP message the hash value will be computed
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int computeHmacTURREP(PASER_TU_RREP * message, lv_block GTK);

    /**
     * @brief Check a hash value of a PASER_TU_RREP message
     *
     *@param message Pointer to the PASER_TU_RREP message the hash value of which will be verified
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int checkHmacTURREP(PASER_TU_RREP * message, lv_block GTK);

    /**
     * @brief Compute the hash value of a PASER_TU_RREP_ACK message using GTK and
     * append the hash value to the message
     *
     *@param message Pointer to the PASER_TU_RREP_ACK message the hash value of which will be computed
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int computeHmacTURREPACK(PASER_TU_RREP_ACK * message, lv_block GTK);

    /**
     * @brief Check the hash value of a PASER_TU_RREP_ACK message
     *
     *@param message Pointer to the PASER_TU_RREP_ACK message the hash value of which will be verified
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int checkHmacTURREPACK(PASER_TU_RREP_ACK * message, lv_block GTK);

    /**
     * @brief Compute the hash value of a PASER_TB_RERR message using GTK and
     * append the hash value to the message
     *
     *@param message Pointer to the PASER_TB_RERR message the hash value of which which will be computed
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int computeHmacRERR(PASER_TB_RERR * message, lv_block GTK);

    /**
     * @brief Check the hash value of a PASER_TB_RERR message
     *
     *@param message Pointer to the PASER_TB_RERR message the hash value of which will be verified
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int checkHmacRERR(PASER_TB_RERR * message, lv_block GTK);

    /**
     * @brief Compute the hash value of a PASER_TB_Hello message using GTK and
     * append the hash value to the message
     *
     *@param message Pointer to the PASER_TB_Hello message the hash value of which will be computed
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int computeHmacHELLO(PASER_TB_Hello * message, lv_block GTK);

    /**
     * @brief Verify the hash value of a PASER_TB_Hello message
     *
     *@param message Pointer to the PASER_TB_Hello message the hash value of which will be verified
     *@param GTK Current GTK
     *
     *@return 1 if successful or 0 on error
     */
    int checkHmacHELLO(PASER_TB_Hello * message, lv_block GTK);
};

#endif /* PASER_CRYPTO_HASH_H_ */
#endif
