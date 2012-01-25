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
 * @file GiaNeighborCandidateList.h
 * @author Robert Palmer
 */

#ifndef __GIANEIGHBORCANDIDATELIST_H_
#define __GIANEIGHBORCANDIDATELIST_H_

#include <NodeHandle.h>

/**
 *
 * This class is for managing of possible neighbor nodes
 * Used for JOIN-Protocol
 *
 */
class GiaNeighborCandidateList
{
  public:
    /**
     * Get size of candidate list
     * @return Size of candidate list
     */
    uint32_t getSize();

    /**
     * Add an node to candidate list
     * @param node Node to add to candidate list
     */
    void add(const NodeHandle& node);

    /**
     * Removes node from position
     * @param position
     */
    void remove(uint32_t position);

    /**
     * Removes node
     * @param node Node to remove from candidate list
     */
    void remove(const NodeHandle& node);

    /**
     * Check if candidate list contains node
     * @param node
     * @return true if list contains node
     */
    bool contains(const NodeHandle& node);

    const NodeHandle& getRandomCandidate();

    /**
     * Get node from position
     * @param position
     */
    const NodeHandle& get(uint32_t position);

    /**
     * Get position of node
     * @return position of node
     */
    //int getPosition(NodeHandle node);

    /**
     * Clear candidate list
     */
    void clear();

  protected:
    std::set<NodeHandle> candidates; /**< contains all neighbor candidates */
};

#endif
