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

#ifndef NA_HACKEDMODULE_H_
#define NA_HACKEDMODULE_H_

#include "inet/neta/common/log/NA_NesgLog.h"

/**
 * @brief Dropping attack hacked module
 *
 * @details This hacked module is in charge of implement the dropping behavior on
 * IP layer. When this module receive a dropping control message from the controller
 * this activate or deactivate the dropping behavior. The packets are discarded randomly
 * following a normal distribution with a @verbatim droppingAttackProbability @endverbatim
 * probability.
 *
 * @see NA_HackedModule, NA_DroppingAttack
 *
 * @author Gabriel Maciá Fernández, gmacia@ugr.es
 * @date 01/22/2013
 *
 */


namespace inet {

namespace neta {


class NA_HackedModule {

private:
    /**
     * Log reference
     */
    NA_NesgLog log;

public:
    /**
     * This method is in charge of to receive the control message from the attack controller.
     * Depending of the message information, this method sets the specific hacked module properties
     * to allow the attack behavior or not.
     *
     * This method must be overridden for all hacked modules.
     *
     * @param msg cMessage the received message from attack controller
     */
    virtual void handleMessageFromAttackController(omnetpp::cMessage *msg);

};

}
}
#endif /* NA_HACKEDMODULE_H_ */
