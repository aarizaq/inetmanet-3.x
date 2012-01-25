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
 * @file SimpleUnderlayConfigurator.h
 * @author Stephan Krause
 */

#ifndef __SIMPLEUNDERLAYCONFIGURATOR_H__
#define __SIMPLEUNDERLAYCONFIGURATOR_H__


#include <omnetpp.h>
#include <BasicModule.h>
#include <deque>
#include <set>

#include <UnderlayConfigurator.h>
#include <InitStages.h>
#include <SimpleInfo.h>


/**
 * Sets up a SimpleNetwork.
 * Adds overlay nodes to the network in init phase
 * and adds/removes/migrates overlay nodes after init phase.
 */
class SimpleUnderlayConfigurator : public UnderlayConfigurator
{
public:
    ~SimpleUnderlayConfigurator();

    /**
     * Creates an overlay node
     *
     * @param type NodeType of the node to create
     * @param initialize are we in init phase?
     */
    virtual TransportAddress* createNode(NodeType type, bool initialize=false);

    /**
     * Notifies and schedules overlay nodes for removal.
     *
     * @param type NodeType of the node to remove
     * @param addr NULL for random node
     */
    virtual void preKillNode(NodeType type, TransportAddress* addr=NULL);

    /**
     * Migrates overlay nodes from one access net to another.
     *
     * @param type NodeType of the node to migrate
     * @param addr NULL for random node
     */
    virtual void migrateNode(NodeType type, TransportAddress* addr=NULL);

    uint32_t getFieldSize() { return fieldSize; };
    uint32_t getFieldDimension() { return dimensions; };
    uint32_t getSendQueueLenghth() { return sendQueueLength; };

protected:

    /**
     * Enables access to the globalHashMap, sets some parameters and adds the
     * initial number of nodes to the network
     *
     * @param stage the phase of the initialisation
     */
    void initializeUnderlay(int stage);

    void handleTimerEvent(cMessage* msg);

    /**
     * Saves statistics, prints simulation time
     */
    void finishUnderlay();

    /**
     * Prints statistics
     */
    void setDisplayString();
    uint32_t parseCoordFile(const char * nodeCoordinateSource);

    uint32 nextFreeAddress; /**< adress of the node that will be created next */
    std::deque<IPvXAddress> killList; //!< stores nodes scheduled to be killed
    std::set<int> scheduledID; //!< stores nodeIds to prevent migration of prekilled nodes

    uint32_t sendQueueLength; /**< send queue length of overlay terminals */
    uint32_t fieldSize;
    int dimensions;
    bool fixedNodePositions;
    bool useIPv6;

    bool useXmlCoords;
    const char* nodeCoordinateSource;
    uint32_t maxCoordinate;

    std::vector<std::pair<NodeRecord*, bool> > nodeRecordPool;

    // statistics
    int numCreated; /**< number of overall created overlay terminals */
    int numKilled; /**< number of overall killed overlay terminals */
};

#endif
