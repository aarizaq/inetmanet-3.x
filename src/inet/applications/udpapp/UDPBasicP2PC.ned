//
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

package inet.applications.udpapp;
import inet.applications.contract.IUDPApp;
//
// TODO auto-generated type
//
simple UDPBasicP2P2C like IUDPApp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module        
        string fuzzyFisFile = default("P2Pprueba.fis");
        int totalSegments = default(100);
        int segmentSize @unit(B) = default(1000B);
        int numInitialSegments = default(0);
        int requestPerPacket = default(10); // max sub segment request
        int maxPacketSize  @unit(B) = default(1000B);
        double packetRate @unit(s) = default(0.5s); // flow
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        int destPort = default(-1);  // local port (-1: use ephemeral port)
        double startTime @unit(s) = default(uniform(30s,50s));  // start timer
        volatile double waitTime  @unit(s) = default(uniform(2s,3s)); //timer that the node wait after received succesfully a packet before request other 
        volatile double requestTimer  @unit(s) = default(uniform(1s,2s)); //
        bool changeSugSegment = default(true);
        int numVarFuzzy = default(3);
        string listNodes = default("");
        int numParallelRequest = default(1); // pending
        bool forceMinHop = default(true);
        bool fuzzy = default(false);
        bool initNodesRand = default(false);
        int numInitNodesRand = default(1);
        bool appRouting = default(false);
        double coverageArea = default(120);       
        double serverRate @unit(s) = default(0.01s);
        double maxFlowRate @unit(s) = default(0.2s);
        string modoP2P @enum("concavo","aditivo","aditivo_ponderado","minhop","random");
        volatile double periodicTimer @unit(s) = default(2s);
        bool setBroadcast = default(true);
        double rateRequest @unit(s) = default(0.2s);
        bool useGlobal = default(false);     
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[dropPk](type=cPacket);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; source=dropPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[queueLength](title="queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);
        
        @display("i=block/app");
        
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);        
}
