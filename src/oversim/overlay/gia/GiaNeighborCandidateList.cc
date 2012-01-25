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
 * @file GiaNeighborCandidateList.cc
 * @author Robert Palmer
 */

#include <iterator>
#include <assert.h>

#include <omnetpp.h>

#include "GiaNeighborCandidateList.h"


uint32_t GiaNeighborCandidateList::getSize()
{
    return candidates.size();
}

void GiaNeighborCandidateList::add(const NodeHandle& node)
{
    assert(!(node.isUnspecified()));
    candidates.insert( node );
}

void GiaNeighborCandidateList::remove(uint32_t position)
{
    std::set<NodeHandle>::iterator it = candidates.begin();
    for (uint32_t i=0; i<position; i++) {
        it++;
    }
    candidates.erase( it );
}

void GiaNeighborCandidateList::remove(const NodeHandle& node)
{
    candidates.erase(node);
}

bool GiaNeighborCandidateList::contains(const NodeHandle& node)
{
    if(node.getKey().isUnspecified())
        return false;

    std::set<NodeHandle>::iterator it = candidates.find(node);

    if(it != candidates.end() && it->getKey() == node.getKey())
        return true;
    else
        return false;
}

//bad code
const NodeHandle& GiaNeighborCandidateList::get( uint32_t position )
{
    if ( position >= candidates.size() )
        return NodeHandle::UNSPECIFIED_NODE;
    else {
        std::set<NodeHandle>::iterator it = candidates.begin();
        for (uint32_t i=0; i<position; i++) {
            it++;
        }
        return *it;
    }
}

const NodeHandle& GiaNeighborCandidateList::getRandomCandidate()
{
    return get(intuniform(0, getSize()));
}

// int GiaNeighborCandidateList::getPosition( NodeHandle node )
// {
//     if ( !contains(node) )
// 	return -1;
//     else
//     {
// 	uint i = 0;
// 	std::set<NodeHandle>::iterator theIterator;
// 	for( theIterator = candidates.begin(); theIterator != candidates.end(); theIterator++ )
// 	{
// 	    if ( theIterator->key == node.getKey() )
// 		return i++;
// 	}
//     }
//     return -1;
// }

void GiaNeighborCandidateList::clear()
{
    candidates.clear();
}
