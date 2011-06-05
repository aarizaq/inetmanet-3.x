//
//   Copyright (C) 2009 by Alfonso Ariza
//   Malaga University 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.linklayer.ieee80211mesh.hwmp;
import inet.networklayer.manetrouting.base.BaseRouting;
import inet.networklayer.IManetRouting;

//
// Dynamic MANET On-demand (DYMO) Routing
//
// Implements draft-ietf-manet-dymo-05.
//
// NOTE:
//  - The initialization of the routing component is done in stage=4
//  - This implementation is a bit outdated. As of 2010-09-16 the 
//    current draft number is 21 
//
simple HwmpProtocol extends BaseRouting like IManetRouting
{
    parameters:
        @class(HwmpProtocol);
        int maxQueueSize=default(255);
        int dot11MeshHWMPmaxPREQretries=default(3);
        double dot11MeshHWMPnetDiameterTraversalTime @unit("s")=default(102400us);
        double dot11MeshHWMPpreqMinInterval @unit("s")=default(102400us);
        double dot11MeshHWMPperrMinInterval @unit("s")=default(102400us);
        double dot11MeshHWMPactiveRootTimeout @unit("s")=default(1024*5000 us);
        double dot11MeshHWMPactivePathTimeout @unit("s")=default(1024*5000 us);
        double dot11MeshHWMPpathToRootInterval @unit("s")=default(1024*2000 us);
        double dot11MeshHWMPrannInterval @unit("s")=default(1024*5000 us);
        double neighborLive @unit("s")=default(1024*5000 us);  // dot11MeshHWMPactivePathTimeout, maximun time to considere a neighbor live without receive a message
        int    lostThreshold = default(1); // number of consecutive lost packet to consider break path and lost the neighbor
        bool concurrentReactive=default(false);
        bool isRoot=default(false);
        int maxTtl=default(32);
        bool updateLifetimeInFrowarding=default(true);
        int unicastPerrThreshold = default(32);
        int unicastPreqThreshold = default(1);
        int unicastDataThreshold = default(1);
        double randomStart = default(uniform(0,0.1));
        bool ToFlag=default(false);
        volatile double broadCastDelay @unit("s") = default (uniform(0s,0.01s));  // the delay added to broadcast operations if EqualDelay is set (used to model processing time) 
        volatile double uniCastDelay @unit("s") = default (uniform(0s,0.001s));  // a delay added to unicast messaged (i.e. data packet forwarding) (used to model processing time)
    gates:
        input from_ip;
        output to_ip;
}

 