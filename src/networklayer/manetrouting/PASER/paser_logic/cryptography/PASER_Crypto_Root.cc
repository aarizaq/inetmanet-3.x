/*
 *\class       PASER_Crypto_Root
 *@brief       Class provides functions to generate secrets, compute and check authentication trees
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
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <omnetpp.h>

#include "PASER_Crypto_Root.h"
#include "PASER_Definitions.h"
#include "compatibility.h"


PASER_Crypto_Root::PASER_Crypto_Root(PASER_Global *pGlobal) {
    paser_global = pGlobal;
}

PASER_Crypto_Root::~PASER_Crypto_Root() {
    clear_lists();
}

void PASER_Crypto_Root::clear_lists() {
    for (std::list<u_int8_t *>::iterator it = secret_list.begin();
            it != secret_list.end(); it++) {
        u_int8_t *data = (u_int8_t *) *it;
        free(data);
    }
    secret_list.clear();
    for (std::list<u_int8_t *>::iterator it = tree.begin(); it != tree.end();
            it++) {
        u_int8_t *data = (u_int8_t *) *it;
        free(data);
    }
    tree.clear();
}

int PASER_Crypto_Root::root_init(int n) {
    iv_nr = 0;
    if (n > 31)
        return 0;
    param = n;

//    return root_regenerate();
    return 0;
}

int PASER_Crypto_Root::root_regenerate() {
    clear_lists();
    int n = param;
    int b = 1;
    b = b << n;
    for (int i = 0; i < b; i++) {
        u_int8_t *buf = (u_int8_t *) malloc(
                (sizeof(u_int8_t) * PASER_SECRET_LEN));
        if (RAND_bytes(buf, PASER_SECRET_LEN) != 1) {
            EV << "Error: root_init - RAND_bytes";
            return 0;
        }
        int count = i << (32 - n);
        for (int j = 0; j < n; j++) {
            int block_nr = j / 8;
            int bit_nr = j % 8;
            u_int8_t del = 0x01;
            del = ~(del << (8 - bit_nr - 1));

            u_int8_t set_bit = 0x01 & (count >> (31 - j));
            set_bit = set_bit << (8 - bit_nr - 1);
            buf[block_nr] = buf[block_nr] & del;
            buf[block_nr] = buf[block_nr] | set_bit;
        }
        secret_list.push_back(buf);

    }

//    root_elem = root_getHashTreeValue(0, secret_list.size()-1);
//    printf("root_elem:0x");
//    for (int n = 0; n < SHA256_DIGEST_LENGTH; n++)
//        printf("%02x", root_elem[n]);
//    putchar('\n');

    calculateTree();
    return 1;
}

void PASER_Crypto_Root::calculateTree() {
    for (std::list<u_int8_t *>::iterator it = secret_list.begin();
            it != secret_list.end(); it++) {
        u_int8_t *data = root_getOneHash((u_int8_t *) *it, PASER_SECRET_LEN);
        tree.push_back(data);
    }

    for (int i = param; i > 0; i--) {
        int steps = 1 << (i - 1);
        std::list<u_int8_t *>::iterator it = tree.begin();
        std::list<u_int8_t *> temp;
        for (int k = 0; k < steps; k++) {
            u_int8_t *data1 = (u_int8_t *) *it;
            it++;
            u_int8_t *data2 = (u_int8_t *) *it;
            it++;
            temp.push_back(root_getHash(data1, data2));
        }
        tree.insert(tree.begin(), temp.begin(), temp.end());
    }

    root_elem = tree.front();
}

u_int8_t* PASER_Crypto_Root::root_get_root() {
    u_int8_t *buf = (u_int8_t *) malloc(
            (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    memcpy(buf, root_elem, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
    return buf;
}

std::list<u_int8_t *> PASER_Crypto_Root::root_get_next_secret(int *nr,
        u_int8_t *secret) {
    std::list<u_int8_t *> iv;
    std::list<u_int8_t *>::iterator it = tree.end();
    it--;
    int point = iv_nr;
    for (int i = param; i > 0; i--) {
        int steps = 1 << i;
        for (int k = steps - 1; k >= 0; k--) {
            if (k == point) {
                if (point % 2 == 0) {
                    it++;
                    u_int8_t *buf = (u_int8_t *) malloc(
                            (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
                    u_int8_t *buf2 = (u_int8_t *) *it;
                    memcpy(buf, buf2,
                            (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
                    iv.push_back(buf);
                    it--;
                } else {
                    it--;
                    u_int8_t *buf = (u_int8_t *) malloc(
                            (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
                    u_int8_t *buf2 = (u_int8_t *) *it;
                    memcpy(buf, buf2,
                            (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
                    iv.push_back(buf);
                    it++;
                }
            }
            it--;
        }
        point = point / 2;
    }
    u_int32_t count = 0;
    for (std::list<u_int8_t *>::iterator IT = secret_list.begin();
            IT != secret_list.end(); IT++) {
        if (count == iv_nr) {
            u_int8_t *temp = (u_int8_t *) *IT;
            memcpy(secret, temp, (sizeof(u_int8_t) * PASER_SECRET_LEN));
            break;
        }
        count++;
    }
//    std::list<u_int8_t *> iv = root_getIvTree(iv_nr, 0, secret_list.size()-1);
    *nr = iv_nr;
    iv_nr++;
    if (iv_nr >= secret_list.size()) {
//        root_regenerate();
        paser_global->getPASER_modul()->startGenNewRoot();
        iv_nr = 0;
    }

    return iv;
}

int PASER_Crypto_Root::root_get_iv() {
    return iv_nr;
}

u_int8_t *PASER_Crypto_Root::root_getOneHash(u_int8_t* h1, int len) {
    SHA256_CTX ctx;
    u_int8_t *results = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);

    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (u_int8_t *) h1,
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    SHA256_Final(results, &ctx);
    return results;
}

u_int8_t *PASER_Crypto_Root::root_getHash(u_int8_t* h1, u_int8_t* h2) {
    SHA256_CTX ctx;
    u_int8_t *results = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);

    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (u_int8_t *) h1,
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    SHA256_Update(&ctx, (u_int8_t *) h2,
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    SHA256_Final(results, &ctx);
    return results;
}

//// begin < iv < end
//std::list<u_int8_t *> PASER_Crypto_Root::root_getIvTree(int iv, int begin, int end){
//    std::list<u_int8_t *> temp;
//    if (begin == end){
//        std::list<u_int8_t *>::iterator it = secret_list.begin();
//        for (int i=0; i<begin; i++){
//            it++;
//        }
//        u_int8_t *buf1 = (u_int8_t *)*it;
//        if (begin != iv){
//            buf1 = root_getOneHash( buf1 );
//        }
//        else{
//            u_int8_t *buf = (u_int8_t *)malloc((sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
//            memcpy(buf, buf1, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH));
//            buf1 = buf;
//        }
//        temp.push_front( buf1 );
//        return temp;
//    }
//
//    if ( (((begin + (end-begin)/2)) < iv)  && (end>=iv)){
//        std::list<u_int8_t *> temp2 = root_getIvTree(iv, (begin + (end-begin)/2)+1 , end);
//        temp.splice( temp.begin(), temp2 );
//
//        temp.push_back( root_getHashTreeValue( begin , (begin + (end-begin)/2) ) );
//        return temp;
//    }
//    else{
//        std::list<u_int8_t *> temp2 = root_getIvTree(iv, begin , (begin + (end-begin)/2) );
//        temp.splice( temp.begin(), temp2 );
//        temp.push_back( root_getHashTreeValue( (begin + (end-begin)/2)+1 , end ));
//        return temp;
//    }
//}
//
//u_int8_t* PASER_Crypto_Root::root_getHashTreeValue(int begin, int end){
//    if(begin == end){
//        std::list<u_int8_t *>::iterator it = secret_list.begin();
//        for (int i=0; i<begin; i++){
//            it++;
//        }
//        u_int8_t *buf1 = (u_int8_t *)*it;
//        return root_getOneHash(buf1);
//    }
//    u_int8_t *buf1 = root_getHashTreeValue(begin, begin + (end-begin)/2);
//    u_int8_t *buf2 = root_getHashTreeValue(begin + (end-begin)/2 + 1, end);
//    u_int8_t *result = root_getHash(buf1, buf2);
//    free(buf1);
//    free(buf2);
//    return result;
//}

int PASER_Crypto_Root::root_check_root(u_int8_t* root, u_int8_t* secret,
        std::list<u_int8_t *> iv_list, u_int32_t iv, u_int32_t *newIV) {
    u_int32_t new_iv = 0;
    u_int8_t temp[4] = { 0x00, 0x00, 0x00, 0x00 };
    u_int8_t * temp1 = secret;
    for (int i = 0; i < 4; i++) {
        memcpy(&temp[3 - i], temp1, 1);
        temp1++;
    }
    memcpy(&new_iv, temp, 4);
    new_iv = new_iv >> (SHA256_DIGEST_LENGTH - param);
    EV << "iv= " << iv << ",new IV_nr = " << new_iv << ", param: " << param
            << "\n";
    if (iv > 0 && iv > new_iv) {
        EV << "old iv\n";
        return 0;
    }

    *newIV = new_iv + 1;

    u_int8_t *buf;
    buf = root_getOneHash(secret, PASER_SECRET_LEN);
    for (std::list<u_int8_t *>::iterator it = iv_list.begin();
            it != iv_list.end(); it++) {
        if (new_iv % 2 == 1) {
            u_int8_t * temp = root_getHash((u_int8_t *) *it, buf);
            free(buf);
            buf = temp;
        } else {
            u_int8_t * temp = root_getHash(buf, (u_int8_t *) *it);
            free(buf);
            buf = temp;
        }
        new_iv = new_iv / 2;
    }

    if (memcmp(root, buf, (sizeof(u_int8_t) * SHA256_DIGEST_LENGTH)) == 0) {
        free(buf);
        return 1;
    }

    free(buf);
    return 0;
}
#endif
