/*
 *\class       PASER_Crypto_Hash
 *@brief       Class provides functions to compute and check the hash value of PASER messages
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
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include "PASER_Crypto_Hash.h"
#include <compatibility.h>



void PASER_Crypto_Hash::init() {

}

int PASER_Crypto_Hash::computeHmacTURREQ(PASER_TU_RREQ * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
//printf("compuDATA:0x");
//for (int n = 0; n < len; n++)
//    printf("%02x", data[n]);
//putchar('\n');
//printf("GTK:0x");
//for (int n = 0; n < GTK.len; n++)
//    printf("%02x", GTK.buf[n]);
//putchar('\n');
//printf("-----\n");
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    message->hash = result;
    return 1;
}

int PASER_Crypto_Hash::checkHmacTURREQ(PASER_TU_RREQ * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
//printf("checkDATA:0x");
//for (int n = 0; n < len; n++)
//    printf("%02x", data[n]);
//putchar('\n');
//printf("GTK:0x");
//for (int n = 0; n < GTK.len; n++)
//    printf("%02x", GTK.buf[n]);
//putchar('\n');
//printf("-----\n");
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    if (memcmp(message->hash, result, (sizeof(u_int8_t) * result_len)) == 0) {
        free(result);
        return 1;
    }
    free(result);
    return 0;
}

int PASER_Crypto_Hash::computeHmacTURREP(PASER_TU_RREP * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    message->hash = result;
    return 1;
}

int PASER_Crypto_Hash::checkHmacTURREP(PASER_TU_RREP * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    if (memcmp(message->hash, result, (sizeof(u_int8_t) * result_len)) == 0) {
        free(result);
        return 1;
    }
    free(result);
    return 0;
}

int PASER_Crypto_Hash::computeHmacTURREPACK(PASER_TU_RREP_ACK * message,
        lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    EV << "len: " << len << "\n";
//printf("DATA:0x");
//for (int n = 0; n < len; n++)
//    printf("%02x", data[n]);
//putchar('\n');
//printf("GTK:0x");
//for (int n = 0; n < GTK.len; n++)
//    printf("%02x", GTK.buf[n]);
//putchar('\n');
//printf("-----\n");
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    message->hash = result;
    return 1;
}

int PASER_Crypto_Hash::checkHmacTURREPACK(PASER_TU_RREP_ACK * message,
        lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    EV << "len: " << len << "\n";
//printf("DATA:0x");
//for (int n = 0; n < len; n++)
//    printf("%02x", data[n]);
//putchar('\n');
//printf("GTK:0x");
//for (int n = 0; n < GTK.len; n++)
//    printf("%02x", GTK.buf[n]);
//putchar('\n');
//printf("-----\n");
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);
//
//    printf("GTK:0x");
//    for (int n = 0; n < GTK.len; n++)
//        printf("%02x", GTK.buf[n]);
//    putchar('\n');
//    printf("hash  :0x");
//    for (int n = 0; n < SHA256_DIGEST_LENGTH; n++)
//        printf("%02x", message->hash[n]);
//    putchar('\n');
//    printf("result:0x");
//    for (int n = 0; n < SHA256_DIGEST_LENGTH; n++)
//        printf("%02x", result[n]);
//    putchar('\n');

    if (memcmp(message->hash, result, (sizeof(u_int8_t) * result_len)) == 0) {
        free(result);
        return 1;
    }
    free(result);
    return 0;
}

int PASER_Crypto_Hash::computeHmacRERR(PASER_TB_RERR * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    EV << "len: " << len << "\n";
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    message->hash = result;
    return 1;
}

int PASER_Crypto_Hash::checkHmacRERR(PASER_TB_RERR * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    EV << "len: " << len << "\n";
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    if (memcmp(message->hash, result, (sizeof(u_int8_t) * result_len)) == 0) {
        free(result);
        return 1;
    }
    free(result);
    return 0;
}

int PASER_Crypto_Hash::computeHmacHELLO(PASER_TB_Hello * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    EV << "len: " << len << "\n";
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    message->hash = result;
    return 1;
}

int PASER_Crypto_Hash::checkHmacHELLO(PASER_TB_Hello * message, lv_block GTK) {
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, GTK.buf, GTK.len, EVP_sha256(), NULL);

    int len = 0;
    u_int8_t *data = message->toByteArray(&len);
    HMAC_Update(&ctx, data, len);
    EV << "len: " << len << "\n";
    free(data);

    u_int8_t *result = (u_int8_t *) malloc(
            sizeof(u_int8_t) * SHA256_DIGEST_LENGTH);
    u_int32_t result_len = SHA256_DIGEST_LENGTH;
    HMAC_Final(&ctx, result, &result_len);
    HMAC_CTX_cleanup(&ctx);

    if (memcmp(message->hash, result, (sizeof(u_int8_t) * result_len)) == 0) {
        free(result);
        return 1;
    }
    free(result);
    return 0;
}
#endif
