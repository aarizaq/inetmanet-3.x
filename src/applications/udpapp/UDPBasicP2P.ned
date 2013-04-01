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
simple UDPBasicP2P like IUDPApp
{
    parameters:
        bool fuzzy = default(false);
        string fuzzyFisFile = default("P2Pprueba.fis");
        int totalSegments;
        int segmentSize @unit(B);
        int numInitialSegments;
        int requestPerPacket = default(10); // max sub segment request
        int maxPacketSize  @unit(B) = default(1000B);
        double packetRate @unit(s) = default(0.1s);
        int localPort = default(-1);  // local port (-1: use ephemeral port)
        int destPort = default(-1);  // local port (-1: use ephemeral port)
        double startTime @unit(s) = default(uniform(30s,50s));  // start timer
        volatile double waitTime  @unit(s) = default(uniform(0.2s,0.5s)); //timer that the node wait after received succesfully a pacet before request other 
        volatile double requestTimer  @unit(s) = default(uniform(1s,2s)); //
        bool changeSugSegment = default(true);
        bool forceMinHop = default(true);
    gates:
        input udpIn @labels(UDPControlInfo/up);
        output udpOut @labels(UDPControlInfo/down);        
}
