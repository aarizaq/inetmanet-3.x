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

/**
 * @file PubSubLobby.h
 * @author Stephan Krause
 */

#ifndef __PUBSUBLOBBY_H_
#define __PUBSUBLOBBY_H_

#include <deque>
#include <algorithm>
#include <omnetpp.h>
#include <NodeHandle.h>
#include <BaseOverlay.h>

#include "PubSubMessage_m.h"
#include "PubSubSubspace.h"

class PubSubLobby : public BaseOverlay
{
    public:
        // OMNeT++
        virtual ~PubSubLobby();
        virtual void initializeOverlay(int stage);
        virtual void finishOverlay();
        virtual void handleUDPMessage(BaseOverlayMessage* msg);
        virtual void handleTimerEvent(cMessage* msg);

        virtual bool handleRpcCall(BaseCallMessage* msg);
        virtual void handleRpcResponse(BaseResponseMessage *msg,
                                       cPolymorphic* context,
                                       int rpcId, simtime_t rtt);
        virtual void handleRpcTimeout(BaseCallMessage *msg,
                                      const TransportAddress &dest,
                                      cPolymorphic* context, int rpcId,
                                      const OverlayKey &destKey);

    protected:
        class ChildEntry {
            public:
                NodeHandle handle;
                int ressources;
                std::set<int> dutySet;
                bool operator< (const ChildEntry c) const { return ressources < c.ressources; }
                bool operator== (const ChildEntry c) const { return handle ==  c.handle; }
                bool operator== (const NodeHandle n) const { return handle ==  n; }
                bool operator== (const TransportAddress n) const { return (TransportAddress) handle == n; }
        };

        int subspaceSize;
        int numSubspaces;
        std::vector<std::vector<PubSubSubspaceLobby> > subspaces;
        std::list<PubSubHelpCall*> waitingForHelp;

//        typedef std::map<NodeHandle, ChildEntry> PlayerMap;
        typedef std::map<TransportAddress, ChildEntry> PlayerMap;
        PlayerMap playerMap;
        typedef std::multimap<int, ChildEntry*, std::greater<int> > PlayerRessourceMap;
        PlayerRessourceMap playerRessourceMap;

        virtual void handleJoin( PubSubJoinCall* joinMsg );
        virtual void handleHelpCall( PubSubHelpCall* helpMsg );
        virtual void handleRespCall( PubSubResponsibleNodeCall* respCall );
        virtual void handleTakeOverResponse( PubSubTakeOverSubspaceResponse* takeOverResp );
        virtual void handleTakeOverTimeout( PubSubTakeOverSubspaceCall* takeOverCall, const TransportAddress& oldNode );
        void handleHelpReleaseMessage( PubSubHelpReleaseMessage* helpRMsg );
        void replaceResponsibleNode( int subspaceId, NodeHandle respNode );
        void replaceResponsibleNode( PubSubSubspaceId subspaceId, NodeHandle respNode );
//        void failedNode(const NodeHandle& node);
        void failedNode(const TransportAddress& node);

        // statistics
        int numPubSubSignalingMessages;
        int pubSubSignalingMessagesSize;

    public:
        friend std::ostream& operator<< (std::ostream& o, const ChildEntry& entry);

};

#endif
