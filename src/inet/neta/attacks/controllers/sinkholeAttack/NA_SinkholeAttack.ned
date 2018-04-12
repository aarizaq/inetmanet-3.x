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

package inet.neta.attacks.controllers.sinkholeAttack;
import inet.neta.attacks.controllers.NA_Attack;

//
// Sinkhole attack controller simple module. Three parameter controls the attack behavior:
// the attack probability, if the route requested is knowed by the attacker or not and the
// the sequence number generated.
//
// @see NA_Attack, NA_AODVUU
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
simple NA_SinkholeAttack extends NA_Attack
{
    parameters:
        @class(neta::NA_SinkholeAttack);
        @display("i=misc/cloud3,red,100;is=l");

        attackType = "sinkhole";  //Should correspond with the tag located in the hackedModules.ned. e.g. @sinkhole for this attack.       
        double sinkholeAttackProbability = default(0); //Probability of answering a RREQ message with a fake route. Range [0,1].
        bool sinkOnlyWhenRouteInTable = default(false); //TRUE: Send fake answers to RREQ messages only when requested routes are really in routing table of sinkhole node.
														//FALSE: Send fake answers for any RREQ messages arriving at the sinkhole node.

        volatile double seqnoAdded = default(uniform(20, 30)); 	// Fake sequence number generated.
        volatile int numHops = default(1);						// Fake number of hops.
}
