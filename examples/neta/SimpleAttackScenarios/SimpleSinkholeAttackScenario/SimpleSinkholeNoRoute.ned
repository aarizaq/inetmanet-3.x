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
// @author: Gabriel Macia
// @date: 09/12/2012

package inet.examples.neta.SimpleAttackScenarios.SimpleSinkholeAttackScenario;

import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;


import inet.neta.nodes.NA_AttackerAdhocHost;
import inet.neta.nodes.NA_AdhocHost;

network SimpleSinkholeNoRoute
{
    @display("bgb=600,600,grey75");
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=50,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=50,100");
        }        
        nodeA: NA_AdhocHost {
            @display("p=100,300");
        }
        nodeB: NA_AdhocHost {
            @display("p=150,500");
        }
        nodeC: NA_AdhocHost {
            @display("p=400,300");
        }
        nodeD: NA_AdhocHost {
            @display("p=350,500");
        }
        attacker: NA_AttackerAdhocHost {
            @display("p=250,150");
        }   
}
