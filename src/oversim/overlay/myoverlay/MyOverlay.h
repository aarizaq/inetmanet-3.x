//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

/**
 * @author Antonio Zea
 */

#ifndef _MYOVERLAY_
#define _MYOVERLAY_

#include "BaseOverlay.h"

class MyOverlay : public BaseOverlay
{
private:
    // RPC timer
    cMessage *rpcTimer;

    // Routing parameters
    int myKey;               // our overlay key
    NodeHandle prevNode;     // next node in chain
    NodeHandle nextNode;     // previous node in chain

    //module parameters
    double dropChance;       // we'll store the "dropChance" parameter here

    // statistics
    int numDropped;          // how many packets have we dropped?

    // routine for RPC timer
    void handleTimerEvent(cMessage *msg);

    // overlay routines
    void initializeOverlay(int stage);                      // called when the overlay is being initialized
    void setOwnNodeID();                                    // (optional) called to set the key of this node (random otherwise)
    void joinOverlay();                                     // called when the node is ready to join the overlay
    void finishOverlay();                                   // called when the module is about to be destroyed

    // obligatory: called when we need the next hop to route a packet to the given key
    NodeVector* findNode(const OverlayKey& key,             // key to route to
                                 int numRedundantNodes,     // how many candidates for next hop we want (see getMaxNumSiblings)
                                 int numSiblings,           // how many siblings we'll return (?) (see getMaxRedundantNodes)
                                 BaseOverlayMessage* msg);  // message being routed

    // obligatory: In general, called when we need to know whether node is amongst numSiblings closest nodes to key.
    // But normally it is called with node set to thisNode, and asking whether this node is responsible for key.
    bool isSiblingFor(const NodeHandle& node,               // which node (usually thisNode) we're referring to
                                 const OverlayKey& key,     // key in question
                                 int numSiblings,           // how many siblings we're querying about
                                 bool* err);                // set to false when we couldn't determine the range

    // obligatory: Set the maximum number of siblings that can be queried about (usually 1)
    int getMaxNumSiblings();

    // obligatory: Set the maximum number of redundant nodes that can be queried about (usually 1)
    int getMaxNumRedundantNodes();

    // Our RPC interface
    // asynchronously request the neighbors of neighborKey
    void getNeighbors(const OverlayKey& neighborKey);
    // function to call to respond about the queried neighbors
    virtual void callbackNeighbors(const NodeHandle& neighborKey,
                                   const NodeHandle& prevNeighbor,
                                   const NodeHandle& nextNeighbor);
    // function to call if the query times out
    virtual void callbackTimeout(const OverlayKey &neighborKey);

    // internal handling of RPCs
    bool handleRpcCall(BaseCallMessage *msg);                    // called when we receive an RPC from another node
    void handleRpcResponse(BaseResponseMessage* msg,         // called when we receive an RPC response from another node
                              cPolymorphic* context,
                              int rpcId,
                              simtime_t rtt);
    void handleRpcTimeout(BaseCallMessage* msg,                 // called when an RPC times out
                             const TransportAddress& dest,
                             cPolymorphic* context, int rpcId,
                             const OverlayKey&);

public:
    MyOverlay() { rpcTimer = NULL; };
    ~MyOverlay() { cancelAndDelete(rpcTimer); };
};

#endif
