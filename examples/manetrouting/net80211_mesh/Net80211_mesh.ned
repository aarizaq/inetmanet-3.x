//
// Copyright (C) 2010 Alfonso Ariza
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

package inet.examples.manetrouting.net80211_mesh;


import inet.linklayer.ieee80211mesh.nodes.BasicMobileManetMesh;
import inet.linklayer.ieee80211mesh.nodes.HostMesh;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

network Net80211_mesh
{
    parameters:
        int numHosts;
        int numFixHosts;
        int numBasic;
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=60,50;i=misc/sun");
        }        
        fixhost[numFixHosts]: HostMesh {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
        host[numHosts]: HostMesh {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
        basic[numBasic]: BasicMobileManetMesh {
            parameters:
                @display("i=device/pocketpc_s;r=,,#707070");
        }
    connections allowunconnected:
}


