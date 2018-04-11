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

#ifndef NA_DELAYATTACK_H_
#define NA_DELAYATTACK_H_

#include "inet/neta/attacks/controllers/NA_Attack.h"
#include "inet/neta/attacks/controlmessages/delayAttack/NA_DelayMessage_m.h"

/**
 * @brief Delay attack controller implementation
 *
 * @details This controller generates the specific control message (delay message) containing
 * the relevant information for the hacked modules affected by the attack.
 * In this case NA_IPv4.
 *
 * @see NA_Attack, NA_IPv4
 *
 * @author Gabriel Maciá Fernández, gmacia@ugr.es
 * @date 01/22/2013
 *
 */
namespace inet {

namespace neta {


class NA_DelayAttack: public NA_Attack {

protected:

    /**
     * Overridden function
     */
    virtual cMessage *generateAttackMessage(const char* name);
};

};
};
#endif /* NA_DELAYATTACK_H_ */
