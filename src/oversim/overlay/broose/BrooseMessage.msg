//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
// @author Jochen Schenk, Ingmar Baumgart
//

cplusplus {{
#include <IPvXAddress.h>
#include <NodeHandle.h>
#include <CommonMessages_m.h>
#include <OverlayKey.h>

// constants for message length in bit
static const int INDEX_L = 8;
static const int BUCKETTYPE_L = 2;
static const int PROTOSTATE_L = 2;
static const int STEP_L = 8;
static const int RIGHTSHIFTING_L = 1;
static const int NODENUM_L = 8;

#define BUCKETCALL_L(msg) (BASECALL_L(msg) + INDEX_L + BUCKETTYPE_L + PROTOSTATE_L)
#define BUCKETRESPONSE_L(msg) (BASERESPONSE_L(msg) + NODENUM_L \
	                       + msg->getNodesArraySize() * NODEHANDLE_L)
#define BROOSEFINDNODEEXTMESSAGE_L (KEY_L + STEP_L + RIGHTSHIFTING_L + NODEHANDLE_L)

}}

class noncobject IPvXAddress;
class noncobject NodeHandle;
class noncobject OverlayKey;
class BaseCallMessage;
class BaseResponseMessage;	

enum BucketType
{

    RIGHT = 0;
    LEFT = 1;
    BROTHER = 2;
}

enum ProState
{

    PINIT = 0;
    PRSET = 1;
    PBSET = 2;
    PREADY = 3;
}

//
// requests a bucket from the destination node
//
packet BucketCall extends BaseCallMessage
{
        int bucketType enum(BucketType);  // identifies the type of the bucket
        int bucketIndex;                  // the index of the bucket
        int proState enum(ProState);      // the protocol overlay state
}

//
// response message to a BucketCall 
//
packet BucketResponse extends BaseResponseMessage
{
       NodeHandle nodes[];    // array of nodes in the bucket
}

//
// message to lookup up an overlay node
//
packet BrooseFindNodeExtMessage
{
        OverlayKey routeKey;   // overlay node to route the message to 
        int step;              // the remaining de bruijn routing steps
        int maxDistance;       // the estimated maximum distance to any key
        bool rightShifting;    // true if message belongs to a rightshift lookup
        NodeHandle lastNode;   // last node that handled this message
}
