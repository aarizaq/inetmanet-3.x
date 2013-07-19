/**
 *\class       PASER_RERR_List
 *@brief       Class represents a map of IP addresses of invalid destinations regarding which error messages have been recently sent
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
class PASER_RERR_List;

#ifndef PASER_BLACKLIST_H_
#define PASER_BLACKLIST_H_

#include "ManetAddress.h"
#include "PASER_Definitions.h"

#include "compatibility.h"


#include <map>


class PASER_RERR_List {
private:
    /**
     * Map of IP addresses.
     * Key   - IP Address
     * Value - Time when a RERR message was send
     */
    std::map<ManetAddress, struct timeval> rerr_list;
public:
    /**
     * @brief Add or edit an entry in container
     *
     *@param addr IP Address
     *@param time Timestamp
     *
     *@return True if entry is successfully added or false if entry has been already added/edited
     *in the last <b>PASER_rerr_limit</b> seconds. Only one RERR message is sent during this interval.
     */
    bool setRerrTime(struct in_addr addr, struct timeval time);

    /**
     * @brief Delete all entries in container
     */
    void clearRerrList();
};

#endif /* PASER_BLACKLIST_H_ */
#endif
