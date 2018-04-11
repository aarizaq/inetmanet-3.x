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

package inet.neta.attacks.controllers;

//
// Base simple module inherited from all attacks modules.
// Is in charge of set the schedulling parameters, attack activation or not, and what
// is the type of the attack.
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
simple NA_Attack
{
    parameters:
        @display("i=misc/cloud3,red,100;is=l");
        string attackType @enum("dropping","sinkhole","delay") = default("");  //String with the type of attack
        bool active = default(false);  //Indicates if this attack is active or not in the node
        double startTime @unit("s") = default(0s); // Timestamp to start the attack
        double endTime @unit("s") = default(0s); //Time instant at which the attack ends, 0 means that the attack will not end during the simulation.
}
