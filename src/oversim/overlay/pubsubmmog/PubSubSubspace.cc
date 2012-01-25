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
 * @file PubSubSubspace.cc
 * @author Stephan Krause
 */


#include "PubSubSubspace.h"

using namespace std;

PubSubSubspace::PubSubSubspace( PubSubSubspaceId id ) : spaceId(id)
{
    responsibleNode = NodeHandle::UNSPECIFIED_NODE;
    lastTimestamp = 0;
}

PubSubSubspace::~PubSubSubspace( )
{
}

std::ostream& operator<< (std::ostream& o, const PubSubSubspace& subspace)
{
    o << "Id: " << subspace.spaceId << " responsible: " << subspace.responsibleNode;
    return o;
}

PubSubSubspaceLobby::PubSubSubspaceLobby( PubSubSubspaceId id ) : PubSubSubspace( id )
{
    waitingForRespNode = false;
}

std::ostream& operator<< (std::ostream& o, const PubSubSubspaceIntermediate& subspace)
{
    o << dynamic_cast<const PubSubSubspace&>(subspace) << "\n";
    o << "  Children:\n";
    set<NodeHandle>::iterator it;
    for( it = subspace.children.begin(); it != subspace.children.end(); ++it ){
        o << "    " << *it << "\n";
    }
    return o;
}

unsigned int PubSubSubspaceResponsible::maxChildren;

PubSubSubspaceResponsible::PubSubSubspaceResponsible( PubSubSubspaceId id )
    : PubSubSubspaceIntermediate( id )
{
    backupNode = NodeHandle::UNSPECIFIED_NODE;
    heartbeatTimer = NULL;
    heartbeatFailCount = 0;
    totalChildrenCount = 0;
}

bool PubSubSubspaceResponsible::addChild( NodeHandle child )
{
    if( getNumChildren() + getNumIntermediates() < (int) maxChildren ) {
        // we still have room in our children list, add to our own
        if( PubSubSubspaceIntermediate::addChild( child ) ){
            ++totalChildrenCount;
        }
        return true;
    } else {
        // Child has to go to an intermediate
        if( cachedChildren.insert( make_pair(child, false) ).second ){
            ++totalChildrenCount;
        }
        return false;
    }
}

PubSubSubspaceResponsible::IntermediateNode* PubSubSubspaceResponsible::removeAnyChild( NodeHandle child )
{
    if( removeChild( child ) || cachedChildren.erase( child )){
        --totalChildrenCount;
        return NULL;
    } else {
        std::deque<IntermediateNode>::iterator it;
        for( it = intermediateNodes.begin(); it != intermediateNodes.end(); ++it ){
            if( it->children.erase( child ) ) {
                --totalChildrenCount;
                return &*it;
            }
        }
        return NULL;
    }
}

PubSubSubspaceResponsible::IntermediateNode* PubSubSubspaceResponsible::getNextFreeIntermediate()
{
    std::deque<IntermediateNode>::iterator it;
    for( it = intermediateNodes.begin(); it != intermediateNodes.end(); ++it ){
        if( it->node.isUnspecified() ) continue;
        int childIntermediates = intermediateNodes.size() - (it - intermediateNodes.begin() +1 )* maxChildren;
        if( childIntermediates < 0 ) childIntermediates = 0;
        if( it->children.size() + it->waitingChildren + childIntermediates < maxChildren ) return &*it;
    }
    return NULL;
}

void PubSubSubspaceResponsible::fixTotalChildrenCount()
{
    totalChildrenCount = children.size() + cachedChildren.size();
    std::deque<IntermediateNode>::iterator it;
    for( it = intermediateNodes.begin(); it != intermediateNodes.end(); ++it ){
        totalChildrenCount += it->children.size();
    }
}

std::ostream& operator<< (std::ostream& o, const PubSubSubspaceResponsible& subspace)
{
    o << dynamic_cast<const PubSubSubspaceIntermediate&>(subspace) << "  BackupNode: " << subspace.backupNode;
    o << "\n  cachedChildren:\n";
    map<NodeHandle, bool>::const_iterator iit;
    for( iit = subspace.cachedChildren.begin(); iit != subspace.cachedChildren.end(); ++iit ){
        o << "    " << iit->first << " waiting: " << iit->second << "\n";
    }
    o << "  totalChildrenCount: " << subspace.totalChildrenCount;
    o << "\n  IntermediateNodes:\n";
    std::deque<PubSubSubspaceResponsible::IntermediateNode>::const_iterator it;
    for( it = subspace.intermediateNodes.begin(); it != subspace.intermediateNodes.end(); ++it ){
        o << "    " << it->node;
            o << "\n    Children:\n";
        for( set<NodeHandle>::iterator iit = it->children.begin(); iit != it->children.end(); ++iit ){
            o << "      " << *iit << "\n";
        }
    }
    return o;
}

