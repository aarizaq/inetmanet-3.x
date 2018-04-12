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

package inet.neta.attacks.controllers.delayAttack;
import inet.neta.attacks.controllers.NA_Attack;

//
// Delay attack controller simple module. Two parameters control the attack behavior:
// the attack probability and the delay time.
//
// @see NA_Attack, NA_IPv4
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
simple NA_DelayAttack extends NA_Attack
{
    parameters:
        @class(neta::NA_DelayAttack);
        @display("i=misc/cloud3,red,100;is=l");

        attackType = "delay";  //Should correspond with the tag located in the hackedModules.ned. e.g. @delay for this attack.
        double delayAttackProbability = default(0); //Probability of delaying a packet. Range [0,1].
        volatile double delayAttackValue @unit(s) = default(normal(1s, 0.1s));//Random delay time
}
