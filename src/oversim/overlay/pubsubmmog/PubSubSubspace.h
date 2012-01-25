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
 * @file PubSubSubspace.h
 * @author Stephan Krause
 */


#ifndef __PUBSUBSUBSPACE_H_
#define __PUBSUBSUBSPACE_H_

#include "PubSubSubspaceId.h"
#include "NodeHandle.h"
#include "PubSubMessage_m.h"
#include <deque>

class PubSubSubspace
{
    protected:
        PubSubSubspaceId spaceId;
        NodeHandle responsibleNode;

        simtime_t lastTimestamp;
    public:
        /**
         * Creates a new PubSubSubspace
         *
         * @param id The group ID of the new group
         */
        PubSubSubspace( PubSubSubspaceId id );
        ~PubSubSubspace( );

        const PubSubSubspaceId& getId() { return spaceId; }
        void setResponsibleNode( NodeHandle node ) { responsibleNode = node; }
        NodeHandle getResponsibleNode() { return responsibleNode; }
        
        void setTimestamp() { lastTimestamp = simTime(); }
        void setTimestamp( simtime_t stamp ) { lastTimestamp = stamp; }
        simtime_t getLastTimestamp() { return lastTimestamp; }
        simtime_t getTimeSinceLastTimestamp() { return simTime() - lastTimestamp; }
        friend std::ostream& operator<< (std::ostream& o, const PubSubSubspace& subspace);
};

class PubSubSubspaceLobby : public PubSubSubspace
{
    public:
        std::list<PubSubResponsibleNodeCall*> waitingNodes;
        bool waitingForRespNode;
        PubSubSubspaceLobby( PubSubSubspaceId id );
};

class PubSubSubspaceIntermediate : public PubSubSubspace
{
    public:
        std::set<NodeHandle> children;
        PubSubSubspaceIntermediate( PubSubSubspaceId id ) : PubSubSubspace( id ) {}
        virtual ~PubSubSubspaceIntermediate( ) {}
        virtual bool addChild( NodeHandle node ) { return children.insert( node ).second; }
        virtual bool removeChild( NodeHandle node ) { return children.erase( node ); }
        virtual int getNumChildren() { return children.size(); }

        friend std::ostream& operator<< (std::ostream& o, const PubSubSubspaceIntermediate& subspace);
};

class PubSubSubspaceResponsible : public PubSubSubspaceIntermediate
{
    public:
        class IntermediateNode
        {
            public:
                NodeHandle node;
                std::set<NodeHandle> children;
                unsigned int waitingChildren;
                IntermediateNode() : waitingChildren(0) {}
        };
        std::deque<IntermediateNode> intermediateNodes;
        std::map<NodeHandle,bool> cachedChildren;

        std::deque<PubSubMoveMessage*> waitingMoveMessages;

        static unsigned int maxChildren;

        PubSubSubspaceResponsible( PubSubSubspaceId id );
        void setBackupNode( NodeHandle b ) { backupNode = b; }
        const NodeHandle& getBackupNode() { return backupNode; }

        void setHeartbeatTimer( PubSubTimer* t ) { heartbeatTimer = t; }
        PubSubTimer* getHeartbeatTimer() { return heartbeatTimer; }

        int getHeartbeatFailCount() { return heartbeatFailCount; }
        void incHeartbeatFailCount() { ++heartbeatFailCount; }
        void resetHeartbeatFailCount() { heartbeatFailCount = 0; }

        int getTotalChildrenCount() { return totalChildrenCount; }
        void fixTotalChildrenCount();
        
        int getNumIntermediates() { return intermediateNodes.size(); }
        IntermediateNode* getNextFreeIntermediate();
        
        virtual bool addChild( NodeHandle node );
        virtual IntermediateNode* removeAnyChild( NodeHandle node );
    protected:
        int totalChildrenCount;
        NodeHandle backupNode;

        PubSubTimer* heartbeatTimer;
        int heartbeatFailCount;

        friend std::ostream& operator<< (std::ostream& o, const PubSubSubspaceResponsible& subspace);
};

#endif
