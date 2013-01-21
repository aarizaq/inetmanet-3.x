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
        int maxQueueSize = default(255);
        int dot11MeshHWMPmaxPREQretries = default(3);
        double dot11MeshHWMPnetDiameterTraversalTime @unit("s") = default(102400us);
        double dot11MeshHWMPpreqMinInterval @unit("s") = default(102400us);
        double dot11MeshHWMPperrMinInterval @unit("s") = default(102400us);
        double dot11MeshHWMPactiveRootTimeout @unit("s") = default(1024*5000 us);
        double dot11MeshHWMPactivePathTimeout @unit("s") = default(1024*5000 us);
        double dot11MeshHWMPpathToRootInterval @unit("s") = default(1024*2000 us);
        double dot11MeshHWMPrannInterval @unit("s") = default(1024*5000 us);
        double dot11MeshHWMPgannInterval @unit("s") = default(1024*5000 us);
        bool   isGan=default(false);
        double neighborLive @unit("s")=default(1024*5000 us);  // dot11MeshHWMPactivePathTimeout, maximun time to considere a neighbor live without receive a message
        int    lostThreshold = default(1); // number of consecutive lost packet to consider break path and lost the neighbor
        bool concurrentReactive = default(false); // it allows that a reactive can be search at the same time that the proactive route is used 
        bool isRoot = default(false); // this node is root, only must be one per network
        bool minHopCost = default(true); // use the min-hop like cost, if false use the airlink metric.
        int maxTtl = default(32);
        bool updateLifetimeInFrowarding = default(true); // refress the route life time every time that this route is used
        int unicastPerrThreshold = default(32);
        int unicastPreqThreshold = default(1);
        int unicastDataThreshold = default(1);
        volatile double randomStart = default(uniform(0,0.1));
        bool ToFlag=default(false);
        double timeLimitInQueue @unit("s") = default(20s);
        bool gratuitousReverseRoute = default (false);// use the data packet to create the reverse route if this route doesn't exist
        volatile double broadcastDelay @unit("s") = default (uniform(0s,0.01s));  // the delay added to broadcast operations if EqualDelay is set (used to model processing time) 
        volatile double unicastDelay @unit("s") = default (uniform(0s,0.001s));  // a delay added to unicast messaged (i.e. data packet forwarding) (used to model processing time)
        bool checkNextHop=default(false);
        bool PublicRoutingTables = default(false);
        bool propagateProactive = default(true);
    gates:
        input from_ip;
        output to_ip;
}

 