//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//



package inet.linklayer.ieee80211mesh.ethernet;

import inet.linklayer.IWiredNic;
import inet.linklayer.IEtherMAC;
import inet.base.IHook;
import inet.linklayer.ethernet.EtherQoSQueue;

//
// Ethernet network interface. Complements EtherMAC and EtherEncap
// with an output queue for QoS and RED support.
//
// @see EthernetInterface2, EthernetInterfaceNoQueue
//
module EthernetInterfaceMesh like IWiredNic
{
    parameters:
        bool csmacdSupport = default(false);  // by default no CSMA/CD so only only point-to-point duplex links are supported.
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string queueType = default("DropTailQueue"); // ~DropTailQueue, or a Diffserv queue; set to "" for use of an internal queue
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        @display("i=block/ifcard;bgb=273,285;bgl=2");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(EtherFrame);
        input wifiMeshIn @labels(Ieee80211Frame);   // to Ethernet MAC
        output wifiMeshOut @labels(Ieee80211Frame); // to Ethernet MAC
    submodules:
        outputHook[numOutputHooks]: <default("Nop")> like IHook if numOutputHooks>0 {
            @display("p=185,94");
        }
        inputHook[numInputHooks]: <default("Nop")> like IHook if numInputHooks>0 {
            @display("p=43,79");
        }
        queue: EtherQoSQueue if queueType != "" {
            parameters:
                dataQueueType = queueType;
                @display("p=165,174;q=l2queue");
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = (queueType == "" ? "" : "queue");
                txQueueLimit = (queueType == "NoQueue" ? 10000 : 1); // queue sends one packet at a time
                @display("p=98,230");
        }
        encap: EtherEncapMesh {
            parameters:
                @display("p=98,124");
        }
    connections:

        encap.lowerLayerOut --> queue.in if queueType != "";
        queue.out --> mac.upperLayerIn if queueType != "";
        encap.lowerLayerOut --> mac.upperLayerIn if queueType == "";
        encap.lowerLayerIn <-- mac.upperLayerOut;
        mac.phys <--> { @display("m=s"); } <--> phys;

        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if numOutputHooks == 0;
        upperLayerOut <-- { @display("m=n"); } <-- encap.upperLayerOut if numInputHooks == 0;

        encap.upperLayerOut --> inputHook[0].in if numInputHooks > 0;
        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }
        inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut if numInputHooks > 0;

        upperLayerIn --> { @display("m=n"); } --> outputHook[0].in if numOutputHooks > 0;
        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }
        outputHook[numOutputHooks-1].out --> encap.upperLayerIn if numOutputHooks > 0;

        wifiMeshIn --> encap.wifiMeshIn;   // from wifi mesh
        wifiMeshOut <-- encap.wifiMeshOut; // to wifi mesh
}

