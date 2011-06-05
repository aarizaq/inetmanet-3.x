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

//
// Spanning Tree Protocol Timers
//
message STPTimer { }

message STPStartProtocol extends STPTimer { }
message STPHelloTimer extends STPTimer { }
message STPForwardTimer extends STPTimer { 
	int port;
}
message STPBPDUTimeoutTimer extends STPTimer {
    int port;
}
message STPHoldTimer extends STPTimer {
    int port;
}

message STPPortEdgeTimer extends STPTimer {
    int port;
}