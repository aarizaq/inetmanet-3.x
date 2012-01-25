//
// Copyright (C) 2010 Institut fuer Telematik, Karlsruher Institut fuer Technologie (KIT)
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
 * @file ReaSEUnderlayConfigurator.h
 * @author Markus Mauch, Stephan Krause, Bernhard Heep, Bernhard Mueller
 */


#ifndef REASEUNDERLAYCONFIGURATOR_H_
#define REASEUNDERLAYCONFIGURATOR_H_


#include <vector>
#include <deque>
#include <set>

#include <omnetpp.h>

#include <UnderlayConfigurator.h>
#include <ConnectReaSE.h>

class AccessInfo;
class IPvXAddress;

/**
 * Configurator module for the ReaSEUnderlay
 *
 * @author Markus Mauch
 * @todo possibility to disable tier1-3 in overlay(access)routers
 */
class ReaSEUnderlayConfigurator : public UnderlayConfigurator
{
public:
    /**
     * Creates an overlay node
     *
     * @param type the NodeType of the node to create
     * @param initialize creation during init phase?
     */
    TransportAddress* createNode(NodeType type, bool initialize=false);

    /**
     * Notifies and schedules overlay nodes for removal.
     *
     * @param type NodeType of the node to remove
     * @param addr NULL for random node
     */
    void preKillNode(NodeType type, TransportAddress* addr=NULL);

    /**
     * Migrates overlay nodes from one access net to another.
     *
     * @param type the NodeType of the node to migrate
     * @param addr NULL for random node
     */
    void migrateNode(NodeType type, TransportAddress* addr=NULL);

private:
    // parameters
    int accessRouterNum; /**< number of access router */
    int overlayAccessRouterNum; /**< number of overlayAccessRouter */
    int overlayTerminalNum; /**< number of terminal in the overlay */
    ConnectReaSE* TerminalConnector;

protected:

    /**
     * Sets up backbone, assigns ip addresses, calculates routing tables,
     * sets some parameters and adds the
     * initial number of nodes to the network
     *
     * @param stage the phase of the initialisation
     */
    void initializeUnderlay(int stage);

    /**
     * process timer messages
     *
     * @param msg the received message
     */
    void handleTimerEvent(cMessage* msg);

    /**
     * Saves statistics, prints simulation time
     */
    void finishUnderlay();

    /**
     * Updates the statistics display string
     */
    void setDisplayString();
    std::vector<cModule*> accessNode; /**< stores accessRouter */
    std::deque<IPvXAddress> killList; //!< stores nodes scheduled to be killed
    std::set<int> scheduledID; //!< stores nodeIds to prevent migration of prekilled nodes

    // statistics
    int numCreated; /**< number of overall created overlay terminals */
    int numKilled; /**< number of overall killed overlay terminals */
};



#endif /* REASEUNDERLAYCONFIGURATOR_H_ */
