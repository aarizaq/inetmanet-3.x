cplusplus {{
#include "PASER_Definitions.h"
#include "compatibility.h"

}}

struct In_addr;
//struct lv_block;
//struct kdc_block;

packet crl_message {
    In_addr src;
    In_addr gwAddr;
    In_addr nextHopAddr;
    int cert_len;
    char cert_array[];
    
    int kdc_gtk_len;
    char kdc_gtk_array[];
    int kdc_nonce;
    int kdc_crl_len;
    char kdc_crl_array[];
    int kdc_cert_len;
    char kdc_cert_array[];
    int kdc_sign_len;
    char kdc_sign_array[];
    int kdc_key_nr;
    int kdc_sign_key_len;
    char kdc_sign_key_array[];
    //lv_block cert;
    //kdc_block kdcData;
    //int nonce;
}
