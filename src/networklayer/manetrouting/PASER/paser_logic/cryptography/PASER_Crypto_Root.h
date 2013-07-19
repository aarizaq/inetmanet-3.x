/**
 *\class       PASER_Crypto_Root
 *@brief       Class provides functions to generate secrets, compute and check authentication trees
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
class PASER_Crypto_Root;

#ifndef PASER_ROOT_H_
#define PASER_ROOT_H_

#include <openssl/crypto.h>
#include <list>

#include "PASER_Global.h"

#include "compatibility.h"

class PASER_Crypto_Root {
private:
    PASER_Global *paser_global; ///< Pointer to PASER global object
    int param; ///< Number of generated secrets 2^param
//  int length;                         //(< Length of secret
    std::list<u_int8_t *> secret_list; ///< List of all generated secrets
    std::list<u_int8_t *> tree; ///< List of computed authentication tree
    u_int32_t iv_nr; ///< IV
    u_int8_t* root_elem; ///< ROOt element

public:
    PASER_Crypto_Root(PASER_Global *pGlobal);
    ~PASER_Crypto_Root();

    /**
     * @brief Initialize the object
     *
     *@param n Number of generated secrets 2^param
     */
    int root_init(int n);

    /**
     * @brief Generate secrets and compute authentication tree
     *
     *@return 1 if successful or 0 on error
     */
    int root_regenerate();

    /**
     * @brief Get root element
     *
     *@return Root element
     */
    u_int8_t* root_get_root();

    /**
     * @brief Get next secret
     *
     *@param nr Pointer to next IV
     *@param secret Pointer to next secret
     *
     *@return Authentication path of current secret
     */
    std::list<u_int8_t *> root_get_next_secret(int *nr, u_int8_t *secret);

    /**
     * @brief Get IV
     *
     *@return IV
     */
    int root_get_iv();

    /**
     * @brief Check authentication path of a given secret
     *
     *@param root Pointer to the root element corresponding to the authentication path
     *@param secret Pointer to the secret corresponding to the authentication path
     *@param iv_list Authentication path
     *@param iv Last IV of the node
     *@param newIV New IV of the node
     *@return 1 if successful or 0 on error
     */
    int root_check_root(u_int8_t* root, u_int8_t* secret,
            std::list<u_int8_t *> iv_list, u_int32_t iv, u_int32_t *newIV);

private:
    /**
     * @brief Calculate authentication tree. This function assumes that all secrets have been already generated
     */
    void calculateTree();

    /**
     *@brief Compute hash of a char array
     *
     *@param h1 Pointer to the char array which will be hashed
     *@param len Length of the char array
     *
     *@return hash
     */
    u_int8_t* root_getOneHash(u_int8_t* h1, int len);

    /**
     *@brief Compute hash of two char arrays having the same length as the hash output
     *
     *@param h1 Pointer to the first char array which will be hashed
     *@param h2 Pointer to the second char array which will be hashed
     *
     *@return hash
     */
    u_int8_t* root_getHash(u_int8_t* h1, u_int8_t* h2);

//    std::list<u_int8_t *> root_getIvTree(int iv, int begin, int end);
//    u_int8_t* root_getHashTreeValue(int begin, int end);

    /**
     * @brief Clear the list of all generated secrets and the list of computed authentication tree
     */
    void clear_lists();
};

#endif /* PASER_ROOT_H_ */
#endif
