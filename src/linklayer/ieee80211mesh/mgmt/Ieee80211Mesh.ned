//
// Copyright (C) 2008 Alfonso Ariza
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
//

package inet.linklayer.ieee80211mesh.mgmt;

import inet.linklayer.ieee80211.mgmt.IIeee80211Mgmt;

//simple Ieee80211Mesh like IIeee80211Mgmt
simple Ieee80211Mesh
{
    parameters:
        @macRouting;  // Mac layer with routing protocol working in the level 2
        string meshReactiveRoutingProtocol = default("inet.networklayer.manetrouting.DYMOUM");
        int frameCapacity = default(100);
        bool useHwmp = default(false); // If active automatically deactivate useProactive and useReactive and activate ETXEstimate
        bool useProactive = default(false);
        bool useReactive = default(true);
        bool FixNode = default(false); // used by routing protocol to indicate no mobility node
        bool UseLwMpls = default(false);
        double maxDelay = default(0.1);
        int maxTTL = default(32); // the same that IP
        bool ETXEstimate = default(false);
        bool IsGateWay = default(false);
        double GateWayAnnounceInterval @unit("s") = default(100s);
        volatile double MacBroadcastDelay @unit("s") = default(uniform(0s,0.01s));
        bool forceRelayUnit  = default(false);
        bool floodingConfirmation = default(false);
        // pending of remove variables
        //         bool ProactiveFeedback = default(true);
        //        int maxHopProactiveFeedback = default(-1); // Maximun number of hops for to use the proactive feedback, not used yet, TO-DO
        //        int maxHopProactive = default(-1); // Maximun number of hops in the fix part of the network with the proactive feedback
        //        int maxHopReactive = default(-1); // Maximun number of hops by the reactive part for to use the proactive feedback, not used yet, TO-DO
        bool locatorActive  = default(false);
        double coverageArea @unit(m) = default(-1m);
        bool inteligentForward = default(false);
        double lifeTimeForInterface @unit(s) = default(5s);
        string selectionCriteria @enum("Etx","MinQueue","LastUsed","MinQueueLastUsed","LastUsedMinQueue") = default("MinQueue");// if ETX active the ETX proc
        bool inteligentBroadcastRouting = default(false);
        @signal[enqueuePk](type=cMessage);
        @signal[dequeuePk](type=cMessage);
        @signal[dropPkByQueue](type=cMessage);
        @signal[queueingTime](type=simtime_t; unit=s);
        @signal[dataQueueLen](type=long);  // number of stored packets
        @signal[numHopsSignal](type=long);
        @signal[numFixHopsSignal](type=long);
        @statistic[numHops](title="numHops"; source=numHopsSignal; record=histogram ; interpolationmode=none);
        @statistic[numFixHops](title="numHops fix"; source=numFixHopsSignal; record=histogram ; interpolationmode=none);        
        @statistic[dataQueueLen](title="data queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[dropPkByQueue](title="packets dropped by queue"; source=dropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input  ETXProcIn @loose;
        output ETXProcOut @loose;
        input  routingInProactive @loose;
        output routingOutProactive @loose;
        input  routingInReactive @loose;
        output routingOutReactive @loose;
        input  routingInHwmp @loose;
        output routingOutHwmp @loose;
        input  upperLayerIn @labels(Ieee802Ctrl/down);
        output upperLayerOut @labels(Ieee802Ctrl/up);
        input  fromEthernet @labels(Ieee80211Frame);
        output toEthernet @labels(Ieee80211Frame);
        input  macIn[];
        output macOut[];
        input  agentIn @loose;   // optional gates used for the agent in stations
        output agentOut @loose; // optional gates used for the agent in stations
        input  locatorIn @loose;
        output  locatorOut @loose;
}
