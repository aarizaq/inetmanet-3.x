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

import inet.linklayer.contract.IWiredNic;
import inet.linklayer.contract.IEtherMAC;
import inet.common.IHook;
import inet.linklayer.ethernet.EtherQoSQueue;
import inet.linklayer.contract.ITrafficConditioner;
//
// Ethernet network interface. Complements EtherMAC and EtherEncap
// with an output queue for QoS and RED support.
//
// @see EthernetInterface2, EthernetInterfaceNoQueue
//
module EthernetInterfaceMesh like IWiredNic
{
    parameters:
        string interfaceTableModule;
        bool csmacdSupport = default(false);  // by default no CSMA/CD so only only point-to-point duplex links are supported.
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex
        string queueType = default("DropTailQueue"); // ~DropTailQueue, or a Diffserv queue; set to "" for use of an internal queue
        int numOutputHooks = default(0);
        int numInputHooks = default(0);
        string ingressTCType = default(""); // a module type implementing ~ITrafficConditioner for optional traffic conditioning of incoming traffic
        string egressTCType = default(""); // a module type implementing ~ITrafficConditioner for optional traffic conditioning of outgoing traffic
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/ifcard;bgb=324,285;bgl=2");
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
        ingressTC: <ingressTCType> like ITrafficConditioner if ingressTCType != "" {
            @display("p=137,52");
        }
        egressTC: <egressTCType> like ITrafficConditioner if egressTCType != "" {
            @display("p=242,35");
        }
        queue: EtherQoSQueue if queueType != "" {
            parameters:
                dataQueueType = queueType;
                @display("p=165,174;q=l2queue");
        }
        mac: <macType> like IEtherMAC {
            parameters:
                queueModule = (queueType == "" ? "" : "^.queue");
                txQueueLimit = (queueType == "" ? 10000 : 1); // queue sends one packet at a time
                @display("p=98,230");
        }
        encap: EtherEncapMesh {
            parameters:
                @display("p=98,124");
        }
    connections:

        mac.upperLayerOut --> encap.lowerLayerIn;
        mac.phys <--> { @display("m=s"); } <--> phys;

        if queueType != "" {
            encap.lowerLayerOut --> queue.in;
            queue.out --> mac.upperLayerIn;
        }

        encap.lowerLayerOut --> mac.upperLayerIn if queueType == "";

        // no input hooks, no ingressTC
        encap.upperLayerOut --> { @display("m=n"); } --> upperLayerOut if numInputHooks == 0 && ingressTCType == "";

        // no input hooks, there is ingressTC
        if numInputHooks == 0 && ingressTCType != "" {
            encap.upperLayerOut --> ingressTC.in;
            ingressTC.out --> { @display("m=n"); } --> upperLayerOut;
        }

        // there are input hooks
        if numInputHooks > 0 {
            inputHook[numInputHooks-1].out --> { @display("m=n"); } --> upperLayerOut;
        }
        for i=0..numInputHooks-2 {
            inputHook[i].out --> inputHook[i+1].in;
        }

        // there are input hooks, no ingressTC
        if numInputHooks > 0 && ingressTCType == "" {
            encap.upperLayerOut --> inputHook[0].in;
        }

        // there are input hooks, there is ingressTC
        if numInputHooks > 0 && ingressTCType != "" {
            encap.upperLayerOut --> ingressTC.in;
            ingressTC.out --> inputHook[0].in;
        }

        // chain output hooks
        for i=0..numOutputHooks-2 {
            outputHook[i].out --> outputHook[i+1].in;
        }

        // no output hooks, no egressTC
        upperLayerIn --> { @display("m=n"); } --> encap.upperLayerIn if numOutputHooks == 0 && egressTCType == "";

        // there are output hooks
        if numOutputHooks > 0 {
            upperLayerIn --> { @display("m=n"); } --> outputHook[0].in;
        }

        // no output hooks, there is egressTC
        if numOutputHooks == 0 && egressTCType != "" {
            upperLayerIn --> { @display("m=n"); } --> egressTC.in;
        }

        // there is egressTC
        if egressTCType != "" {
            egressTC.out --> encap.upperLayerIn;
        }

        // there are output hooks, no egressTC
        if numOutputHooks > 0 && egressTCType == "" {
            outputHook[numOutputHooks-1].out --> encap.upperLayerIn;
        }

        // there are output hooks, there is egressTC
        if numOutputHooks > 0 && egressTCType != "" {
            outputHook[numOutputHooks-1].out --> egressTC.in;
        }


        wifiMeshIn --> encap.wifiMeshIn;   // from wifi mesh
        wifiMeshOut <-- encap.wifiMeshOut; // to wifi mesh

}

