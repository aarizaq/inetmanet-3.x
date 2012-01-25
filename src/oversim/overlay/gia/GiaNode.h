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
 * @file GiaNode.h
 * @author Robert Palmer 
 */

#ifndef __GIANODE_H_
#define __GIANODE_H_


#include <string>

#include <NodeHandle.h>


/**
 *
 * This class represents a node in gia overlay network
 * 
 */
class GiaNode : public NodeHandle
{
public:
    GiaNode();

    virtual ~GiaNode() {};

    GiaNode(const NodeHandle& handle);

    GiaNode(const NodeHandle& handle, double cap, int degree);

    static const GiaNode UNSPECIFIED_NODE; /** an unspecified node */

    GiaNode& operator=(const NodeHandle& handle);

    // class methodes
    /**
     * Set capacity (function of bandwidth, cpu power and HDD-fitness
     * @param capacity Capacity to set
     */
    void setCapacity(double capacity);

    /**
     * Get capacity
     * @return GiaNode
     */
    double getCapacity() const;

    friend std::ostream& operator<<(std::ostream& os, const GiaNode& n);

protected:

    double capacity; /** capacity of this node */
};

#endif
