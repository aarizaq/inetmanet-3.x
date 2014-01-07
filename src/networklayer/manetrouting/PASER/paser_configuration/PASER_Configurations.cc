/*
 *class       PASER_Configurations
 *brief       Class is a parser of the PASER configuration in the NED file
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
#include "PASER_Configurations.h"
#include "IPv4InterfaceData.h"
#include <InterfaceEntry.h>

PASER_Configurations::PASER_Configurations(char *configFile, PASER_Socket *pModul) {
    paser_modul = pModul;
    if (configFile[0] == '1') {
        isGW = true;
        EV << "GATEWAY\n";
    } else {
        isGW = false;
        EV << "CLIENT\n";
    }

    setGWsearch = pModul->par("isGWsearch");
    root_repetitions_timeout = pModul->par("secret_repeat_broadcast_timeout");
    root_repetitions = pModul->par("secret_repeat_broadcast");

//isBlackHole
    if (pModul->getParentModule()->par("isBlackHole")) { // BlackHole
        if (isGW) {
            certfile = new char[strlen(PASER_gateway_old_cert_file) + 1];
            strcpy(certfile, PASER_gateway_old_cert_file);
            keyfile = new char[strlen(PASER_gateway_old_cert_key_file) + 1];
            strcpy(keyfile, PASER_gateway_old_cert_key_file);
        } else {
            certfile = new char[strlen(PASER_router_old_cert_file) + 1];
            strcpy(certfile, PASER_router_old_cert_file);
            keyfile = new char[strlen(PASER_router_old_cert_key_file) + 1];
            strcpy(keyfile, PASER_router_old_cert_key_file);
        }
    } else {
        if (isGW) {
            certfile = new char[strlen(PASER_gateway_cert_file) + 1];
            strcpy(certfile, PASER_gateway_cert_file);
            keyfile = new char[strlen(PASER_gateway_cert_key_file) + 1];
            strcpy(keyfile, PASER_gateway_cert_key_file);
        } else {
            certfile = new char[strlen(PASER_router_cert_file) + 1];
            strcpy(certfile, PASER_router_cert_file);
            keyfile = new char[strlen(PASER_router_cert_key_file) + 1];
            strcpy(keyfile, PASER_router_cert_key_file);
        }
    }

    cafile = new char[strlen(PASER_CA_cert_file) + 1];
    strcpy(cafile, PASER_CA_cert_file);

    GTK.len = 0;
    GTK.buf = NULL;
    KDC_cert.len = 0;
    KDC_cert.buf = NULL;
    RESET_sign.len = 0;
    RESET_sign.buf = NULL;
    key_nr = 0;
//    if(isGW){
//        char *gtkKey = (char *)malloc(6);
//        strcpy( gtkKey, "12345" );
//        GTK.buf = (u_int8_t *)gtkKey;
//        GTK.len = 6;
//    }
    netDeviceNumber = 1;

    EV << "getNumInterfaces() = " << pModul->MYgetNumInterfaces() << "\n";

    netDevice = NULL;
    if (netDeviceNumber > 0) {
        netDevice = new network_device[netDeviceNumber];
        netDevice[0].enabled = 1;
        netDevice[0].sock = -1;
        netDevice[0].icmp_sock = -1;
        netDevice[0].bcast.S_addr = PASER_BROADCAST;
        netDevice[0].ipaddr.S_addr.set(pModul->PUBLIC_getInterfaceEntry(0)->ipv4Data()->getIPAddress());
        netDevice[0].ifindex = 0;
        strcpy(netDevice[0].ifname,
                pModul->PUBLIC_getInterfaceEntry(0)->getName());
    }

    netEthDevice = NULL;
    for (int i = 0; i < pModul->MYgetNumInterfaces(); i++) {
        InterfaceEntry * tempIf = pModul->PUBLIC_getInterfaceEntry(i);
        if (memcmp(tempIf->getName(), "eth0", 4) == 0) {
            EV << "netEthDevice found!\n";
            netEthDevice = new network_device[1];
            netEthDevice[0].enabled = 1;
            netEthDevice[0].sock = -1;
            netEthDevice[0].icmp_sock = -1;
            netEthDevice[0].bcast.S_addr = PASER_BROADCAST;
            netEthDevice[0].ipaddr.S_addr.set(pModul->PUBLIC_getInterfaceEntry(i)->ipv4Data()->getIPAddress());
            netEthDevice[0].ifindex = i;
            strcpy(netEthDevice[0].ifname,
                    pModul->PUBLIC_getInterfaceEntry(i)->getName());
            netEthDeviceNumber = 1;
            break;
        }
    }

//    for(int i=0; i<pModul->MYgetNumInterfaces(); i++){
//        InterfaceEntry * tempIf = pModul->PUBLIC_getInterfaceEntry(i);
//        EV << "ClassName: " << tempIf->getClassName() << "\n";
//        EV << "Name: " << tempIf->getName() << "\n";
//        EV << "FullName: " << tempIf->getFullName() << "\n";
//    }

//    netDevice[1].enabled = 1;
//    netDevice[1].sock = -1;
//    netDevice[1].icmp_sock = -1;
//    netDevice[1].bcast.S_addr = PASER_BROADCAST;
//    netDevice[1].ipaddr.S_addr = pModul->PUBLIC_getInterfaceEntry(3)->ipv4Data()->getIPAddress().getInt();
//    netDevice[1].ifindex = 3;
//    strcpy(netDevice[1].ifname, pModul->PUBLIC_getInterfaceEntry(3)->getName());

    netAddDevice = NULL;
    intAddlList();

    isRootReady = false;

    isLinkLayerFeeback = PASER_linkLayerFeeback;

    isLocalRepair = pModul->par("PASER_local_repair").boolValue();
    maxHopCountForLocalRepair =
            (int) pModul->par("PASER_local_repair_hopCount").longValue();

    addressOfKDC.S_addr.set(IPv4Address((in_addr_t) 0x0A0A0001));

}

PASER_Configurations::~PASER_Configurations() {
    delete[] certfile;
    delete[] keyfile;
    delete[] cafile;

    if (netDevice) {
        delete[] netDevice;
    }
    if (netEthDevice) {
        delete[] netEthDevice;
    }
    if (netAddDevice) {
        delete[] netAddDevice;
    }

    if (GTK.len > 0) {
        free(GTK.buf);
        GTK.len = 0;
    }

    if (KDC_cert.len > 0) {
        free(KDC_cert.buf);
        KDC_cert.len = 0;
    }

    if (RESET_sign.len > 0) {
        free(RESET_sign.buf);
        RESET_sign.len = 0;
    }
}

bool PASER_Configurations::getIsGW() {
    return isGW;
}

char *PASER_Configurations::getCertfile() {
    return certfile;
}

char *PASER_Configurations::getKeyfile() {
    return keyfile;
}

char *PASER_Configurations::getCAfile() {
    return cafile;
}

bool PASER_Configurations::isGWsearch() {
    return setGWsearch;
}

lv_block PASER_Configurations::getGTK() {
    return GTK;
}

void PASER_Configurations::setGTK(lv_block _GTK) {
    if (GTK.len > 0) {
        free(GTK.buf);
    }
    if (_GTK.len == 0) {
        GTK.len = 0;
        GTK.buf = NULL;
        return;
    }
    GTK.buf = (u_int8_t *) malloc(_GTK.len);
    memcpy(GTK.buf, _GTK.buf, _GTK.len);
    GTK.len = _GTK.len;
}

lv_block PASER_Configurations::getKDC_cert() {
    return KDC_cert;
}

void PASER_Configurations::getKDCCert(lv_block *t) {
    t->len = KDC_cert.len;
    t->buf = (u_int8_t *) malloc((sizeof(u_int8_t) * KDC_cert.len));
    memcpy(t->buf, KDC_cert.buf, (sizeof(u_int8_t) * KDC_cert.len));
}

void PASER_Configurations::setKDC_cert(lv_block _KDC_cert) {
    if (KDC_cert.len > 0) {
        free(KDC_cert.buf);
    }
    if (_KDC_cert.len == 0) {
        KDC_cert.len = 0;
        KDC_cert.buf = NULL;
        EV << "_KDC_cer = NULLt\n";
        return;
    }
    KDC_cert.buf = (u_int8_t *) malloc(_KDC_cert.len);
    memcpy(KDC_cert.buf, _KDC_cert.buf, _KDC_cert.len);
    KDC_cert.len = _KDC_cert.len;
//    printf("cert_set__KDC:");
//    for (int n = 0; n < _KDC_cert.len; n++)
//        printf("%02x", _KDC_cert.buf[n]);
//    putchar('\n');
//    printf("cert__me__KDC:");
//    for (int n = 0; n < KDC_cert.len; n++)
//        printf("%02x", KDC_cert.buf[n]);
//    putchar('\n');
}

lv_block PASER_Configurations::getRESET_sign() {
    return RESET_sign;
}

void PASER_Configurations::getRESETSign(lv_block *s) {
    s->len = RESET_sign.len;
    s->buf = (u_int8_t *) malloc((sizeof(u_int8_t) * RESET_sign.len));
    memcpy(s->buf, RESET_sign.buf, (sizeof(u_int8_t) * RESET_sign.len));
//printf("sign_getRESETSign:");
//for (int n = 0; n < RESET_sign.len; n++)
//    printf("%02x", RESET_sign.buf[n]);
//putchar('\n');
}

void PASER_Configurations::setRESET_sign(lv_block _RESET_sign) {
    if (RESET_sign.len > 0) {
        free(RESET_sign.buf);
    }
    if (_RESET_sign.len == 0) {
        RESET_sign.len = 0;
        RESET_sign.buf = NULL;
        return;
    }
    RESET_sign.buf = (u_int8_t *) malloc(_RESET_sign.len);
    memcpy(RESET_sign.buf, _RESET_sign.buf, _RESET_sign.len);
    RESET_sign.len = _RESET_sign.len;

//printf("sign_setRESETSign:");
//for (int n = 0; n < RESET_sign.len; n++)
//    printf("%02x", RESET_sign.buf[n]);
//putchar('\n');
}

void PASER_Configurations::setKeyNr(u_int32_t k) {
    key_nr = k;
}

u_int32_t PASER_Configurations::getKeyNr() {
    return key_nr;
}

u_int32_t PASER_Configurations::getNetDeviceNumber() {
    return netDeviceNumber;
}

network_device *PASER_Configurations::getNetDevice() {
    return netDevice;
}

u_int32_t PASER_Configurations::getNetEthDeviceNumber() {
    return netEthDeviceNumber;
}

network_device *PASER_Configurations::getNetEthDevice() {
    return netEthDevice;
}

u_int32_t PASER_Configurations::getNetAddDeviceNumber() {
    return netAddDeviceNumber;
}

network_device *PASER_Configurations::getNetAddDevice() {
    return netAddDevice;
}

bool PASER_Configurations::isSetLinkLayerFeeback() {
    return isLinkLayerFeeback;
}

bool PASER_Configurations::isSetLocalRepair() {
    return isLocalRepair;
}

u_int32_t PASER_Configurations::getMaxLocalRepairHopCount() {
    return maxHopCountForLocalRepair;
}

u_int32_t PASER_Configurations::getRootRepetitionsTimeout() {
    return root_repetitions_timeout;
}

u_int32_t PASER_Configurations::getRootRepetitions() {
    return root_repetitions;
}

/*
 * Initialize the Array of node's subnetworks
 */
void PASER_Configurations::intAddlList() {
    int wlanNumber = paser_modul->MYgetNumWlanInterfaces();
    netAddDeviceNumber = wlanNumber - netDeviceNumber;
    EV << "netAddDeviceNumber = " << netAddDeviceNumber << "\n";
    if (netAddDeviceNumber <= 0) {
        return;
    }
    netAddDevice = new network_device[netAddDeviceNumber];

    for (u_int32_t i = 0; i < netAddDeviceNumber; i++) {
        netAddDevice[i].enabled = 1;
        netAddDevice[i].sock = -1;
        netAddDevice[i].icmp_sock = -1;
//        netAddDevice[i].bcast.S_addr = (in_addr_t) 0xFFFFFF00;
        netAddDevice[i].mask.S_addr.set(paser_modul->PUBLIC_getInterfaceEntry(i + netDeviceNumber )->ipv4Data()->getNetmask());
        netAddDevice[i].ipaddr.S_addr.set(IPv4Address(paser_modul->PUBLIC_getInterfaceEntry(i + netDeviceNumber)->ipv4Data()->getIPAddress().getInt()
                        & netAddDevice[i].mask.S_addr.getIPv4().getInt()));/*(in_addr_t) 0xFFFFFF00*/;
        EV << "net Mask: "
                << paser_modul->PUBLIC_getInterfaceEntry(
                        i + netDeviceNumber)->ipv4Data()->getNetmask().str()
                << "\n";
        netAddDevice[i].ifindex = i + netDeviceNumber + 1;
        strcpy(
                netAddDevice[i].ifname,
                paser_modul->PUBLIC_getInterfaceEntry(i + netDeviceNumber)->getName());
    }
}

bool PASER_Configurations::isAddInMySubnetz(struct in_addr Addr) {
    for (u_int32_t i = 0; i < netAddDeviceNumber; i++) {
        EV << "(netAddDevice[i].mask.S_addr & Addr.S_addr) = "
                << (netAddDevice[i].mask.S_addr.getIPv4().getInt() & Addr.S_addr.getIPv4().getInt()) << "\n";
        if ((netAddDevice[i].mask.S_addr.getIPv4().getInt() & Addr.S_addr.getIPv4().getInt())
                == (netAddDevice[i].mask.S_addr.getIPv4().getInt() & netAddDevice[i].ipaddr.S_addr.getIPv4().getInt())) {
            return true;
        }
    }
    return false;
}

struct in_addr PASER_Configurations::getAddressOfKDC() {
    return addressOfKDC;
}
#endif
