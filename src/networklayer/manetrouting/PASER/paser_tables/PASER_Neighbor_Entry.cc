/*
 *\class       PASER_Neighbor_Table
 *@brief       Class represents an entry in the neighbor table
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
#include <openssl/x509.h>
#include "PASER_Neighbor_Entry.h"


PASER_Neighbor_Entry::~PASER_Neighbor_Entry() {
    if (root) {
        free(root);
    }
    root = NULL;

    if (Cert) {
        X509_free((X509*) Cert);
    }
    Cert = NULL;
}

void PASER_Neighbor_Entry::setValidTimer(PASER_Timer_Message *_validTimer) {
    validTimer = _validTimer;
}
#endif
