//
// Copyright (C) 2013, NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//
// This file is part of NETA.
//
//    NETA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NETA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NETA.  If not, see <http://www.gnu.org/licenses/>.
//

#include "inet/neta/attacks/controllers/sinkholeAttack/NA_SinkholeAttack.h"
#include "inet/neta/attacks/controlmessages/sinkholeAttack/NA_SinkholeMessage_m.h"

/**
 * Type cPar. For allow functions into parameters from the *.ini file
 */
typedef cPar* ParPtr;

namespace inet {

namespace neta {

Define_Module (NA_SinkholeAttack);

cMessage *NA_SinkholeAttack::generateAttackMessage(const char *name) {

    LOG << "NA_SinkholeAttack: generateAttackMessage\n";

    // Generates the specific message with the specific parameters for hacked module
    NA_SinkholeMessage *msg = new NA_SinkholeMessage(name);
    msg->setSinkholeAttackProbability(
            par("sinkholeAttackProbability").doubleValue());
    msg->setSinkOnlyWhenRouteInTable(
            par("sinkOnlyWhenRouteInTable").boolValue());
    msg->setSeqnoAdded(&par("seqnoAdded"));
    msg->setNumHops(par("numHops"));
    return msg;
}

}
}
