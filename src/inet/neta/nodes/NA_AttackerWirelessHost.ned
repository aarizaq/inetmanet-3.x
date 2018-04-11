//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

//
// Copyright (C) 2013 and modified by NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//

package inet.neta.nodes;
import inet.neta.attacks.controllers.droppingAttack.NA_DroppingAttack;

//
// Attacker wireless host. Modificated from WirelessHost module. 
// Redefined from INET framework and used as attacker host.
//
// To implement an attack you must include the corresponding attack controller module in the attacker host.
//
// This attack could be include N attacks controllers to implement several simultaneous attacks.
// The current attack controllers included are:
// - NA_DroppingAttack (dropping attack controller)
//
// @see NA_StandardHost, WirelessHost, NA_DroppingAttack
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
module NA_AttackerWirelessHost extends NA_StandardHost
{
    @display("i=device/wifilaptop;bgb=754,448;i2=status/cross,white");
    numRadios = default(1);
    submodules:
        //# Include dropping attack controller 
        droppingAttack: NA_DroppingAttack {
            @display("p=665,54");
        }
}
