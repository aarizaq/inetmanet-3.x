// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file TopologyVis.h
 * @author Bernhard Heep
 */

#ifndef TOPOLOGYVIS_H_
#define TOPOLOGYVIS_H_

class NodeHandle;
class GlobalNodeList;
class cModule;


enum VisDrawDirection
{
    VIS_IN,
    VIS_OUT
};

class TopologyVis
{
public:
    TopologyVis();

protected:
    // overlay identity
    cModule* thisTerminal; /** pointer to corresponding node */
    GlobalNodeList* globalNodeList;

    void initVis(cModule* terminal);

    /**
     * Draws an arrow from this node to neighbor
     *
     * @param neighbor neighbor to point to
     * @param flush delete all previous drawn arrows starting at this node?
     * @param displayString display string to define the arrow drawing style
     * @todo add neighbor's kind to distinguish arrows pointing to the
     * same neighbor
     */
    void showOverlayNeighborArrow(const NodeHandle& neighbor,
                                  bool flush = true,
                                  const char* displayString = NULL);

    /**
     * Removes an arrow from this node to neighbor
     *
     * @param neighbor neighbor to remove arrow to
     * @todo add neighbor's kind to distinguish arrows pointing to the
     * same neighbor
     */
    void deleteOverlayNeighborArrow(const NodeHandle& neighbor);

private:
    /**
     * compacts arrow gate-array
     *
     * @param terminal node
     * @param dir in- or out-array?
     */
    void compactGateArray(cModule* terminal, enum VisDrawDirection dir);
};

#endif /*TOPOLOGYVIS_H_*/
