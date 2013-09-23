/*
 *\class       PASER_RERR_List
 *@brief       Class represents a list of IP addresses of invalid destinations regarding which error messages have been recently sent
 *@ingroup     MP
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
#include "PASER_RERR_List.h"

bool PASER_RERR_List::setRerrTime(struct in_addr addr, struct timeval time) {
    EV << "Suche ein Eintrag in blacklist fuer IP: "
            << addr.S_addr.getIPv4().str() << "\n";
    std::map<ManetAddress, struct timeval>::iterator it = rerr_list.find(
            addr.S_addr);
    if (it != rerr_list.end()) {
        struct timeval last = it->second;
        EV << "Eintrag fuer IP: " << addr.S_addr.getIPv4().str()
                << " exestiert in blacklist\n";
        EV << "Zeit in blackList sec: " << last.tv_sec << " usec: "
                << last.tv_usec << "\n";
        EV << "Zeil jetzt sec: " << time.tv_sec << " usec: " << time.tv_usec
                << "\n";
        if (time.tv_sec - last.tv_sec > 1) {
            it->second.tv_sec = time.tv_sec;
            it->second.tv_usec = time.tv_usec;
            EV << "1\n";
            return true;
        } else if (time.tv_sec - last.tv_sec == 1
                && time.tv_usec - last.tv_usec + 1000000
                        > PASER_rerr_limit * 1000) {
            it->second.tv_sec = time.tv_sec;
            it->second.tv_usec = time.tv_usec;
            EV << "2\n";
            return true;
        } else if (time.tv_sec - last.tv_sec == 0
                && time.tv_usec - last.tv_usec > PASER_rerr_limit * 1000) {
            it->second.tv_sec = time.tv_sec;
            it->second.tv_usec = time.tv_usec;
            EV << "3\n";
            return true;
        }
        EV << "diff < PASER_rerr_limit\n";
        return false;
    }
    rerr_list.insert(std::make_pair(addr.S_addr, time));
    return true;
}

void PASER_RERR_List::clearRerrList() {
    rerr_list.clear();
}
#endif
