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
import inet.applications.IUDPApp;
//
// TODO auto-generated type
//
simple UDPBasicP2P2Multi like IUDPApp
{
    parameters:
        string fuzzyFisFile = default("P2Pprueba.fis");
        string totalSegmentsStr = default("100 100 100 100 100");
        string segmentSizeStr = default("1000 1000 1000 1000 1000");
        string totalSegmentsIdStr = default("1 2 3 4 5");
        string objectsIdStr = default("");
        string numInitialSegmentsStr = default("");
        bool useZipfLaw = default(true);
        int numObjects = default(1);
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
        bool initNodesRand = default(true);
        int numInitNodesRand = default(1);
        bool appRouting = default(false);
        double coverageArea = default(120);       
        double serverRate @unit(s) = default(0.01s);
        double maxFlowRate @unit(s) = default(0.2s);
        string modoP2P @enum("concavo","aditivo","aditivo_ponderado","minhop","random");
        volatile double periodicTimer @unit(s) = default(2s);
        bool setBroadcast = default(true);
        double rateRequest @unit(s) = default(0.2s);
        bool useGlobal = default(true);
        bool sourceRouting = default(false);
        bool changeDestination = default(true);
        bool SelectServerListInInit = default(false);
        double periodicMeasureTimer @unit(s) = default(10s);
        bool periodicMeasure = default(false);
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[dropPk](type=cPacket);
        @signal[queueLength](type=long);
        @signal[queueLengthInst](type=long);
        
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="packets dropped"; source=dropPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[queueLength](title="queue length"; source= queueLength;record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[queueLengthInst](title="queue length inst"; source = queueLengthInst; record=histogram; interpolationmode=none);
        @display("i=block/app");
        
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);        
}
