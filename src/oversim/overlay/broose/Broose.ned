//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

package inet.oversim.overlay.broose;

import inet.oversim.common.BaseOverlay;
import inet.oversim.common.IOverlay;


//
// The main module of the Broose implementation
//
// @author Jochen Schenk, Ingmar Baumgart
//
simple Broose extends BaseOverlay
{
    parameters:
        @class(Broose);
        int bucketSize;    // number of nodes a bucket contains
        int rBucketSize;    // number of nodes a right-bucket contains
        double joinDelay @unit(s);    // time to wait to join the overlay after simulation start 
        int brooseShiftingBits;    // number of bits shifted in/out each step
        int userDist;    // number of hops that are added to the estimated hop count
        double refreshTime @unit(s);    // idle time after which a node is pinged
        int numberRetries;    // number of retries in case of timeout
        bool stab1;
        bool stab2;
}

//
// This modul contains the buckets of the Broose implementation.
//
// @author Jochen Schenk
//
simple BrooseBucket
{
    parameters:
        @display("i=block/table");
}

//
// Implementation of the Broose overlay as described in
// "Broose: A Practical Distributed Hashtable based on the
// De-Bruijn Topology" by A. Gai and L. Viennot, published in
// "Technical report, INRIA, june 2004"
//
// @author Jochen Schenk, Ingmar Baumgart
//
module BrooseModules like IOverlay
{
    parameters:
        int brooseShiftingBits; // number of bits shifted in/out each step
    gates:
        input udpIn;    // gate from the UDP layer
        output udpOut;    // gate to the UDP layer
        input tcpIn;    // gate from the TCP layer
        output tcpOut;    // gate to the TCP layer
        input appIn;    // gate from the application
        output appOut;    // gate to the application
    submodules:
        rBucket[2^brooseShiftingBits]: BrooseBucket {
            parameters:
                @display("i=block/table;p=204,60");
        }
        lBucket: BrooseBucket {
            parameters:
                @display("p=360,60;i=block/table");
        }
        bBucket: BrooseBucket {
            parameters:
                @display("p=420,60;i=block/table");
        }
        broose: Broose {
            parameters:
                brooseShiftingBits = brooseShiftingBits;
                @display("p=60,60;i=block/circle");

        }
    connections allowunconnected:
        udpIn --> broose.udpIn;
        udpOut <-- broose.udpOut;
        appIn --> broose.appIn;
        appOut <-- broose.appOut;

}
