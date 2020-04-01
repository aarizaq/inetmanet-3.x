//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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


import inet.neta.nodes.NA_WirelessHost;
import inet.networklayer.contract.IManetRouting;
import inet.neta.attacks.controllers.NA_Attack;
import inet.neta.attacks.controllers.delayAttack.NA_DelayAttack;
import inet.neta.attacks.controllers.droppingAttack.NA_DroppingAttack;
import inet.neta.attacks.controllers.sinkholeAttack.NA_SinkholeAttack;


//
// Wireless ad hoc host. Modificated from WirelessHost module. 
// Redefined from INET framework and used as normal host.
// 
// It is used as a simple AdhocHost like in INET framework. These hosts never
// behaves as attacker host. The only differente regards AdhocHost is that in
// this case they are using the MA framework arquitecture.
//
// @see AdhocHost, WirelessHost
//
// @author Gabriel Maciá Fernández, gmacia@ugr.es
// @date 01/22/2013
module NA_AdhocHost extends NA_WirelessHost
{
    parameters:
        @display("i=device/cellphone,white;bgb=803,448;i2=status/green");
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");  // use adhoc management
        string routingProtocol @enum("AODVUU","DYMOUM","DYMO","DSRUU","OLSR","OLSR_ETX","DSDV_2","Batman") = default("AODVUU");

    submodules:
        manetrouting: <routingProtocol> like IManetRouting if routingProtocol != "" {
            @display("p=522,307");
        }
    connections:
        networkLayer.transportOut++ --> manetrouting.ipIn if routingProtocol != "";
        networkLayer.transportIn++ <-- manetrouting.ipOut if routingProtocol != "";
        
}
