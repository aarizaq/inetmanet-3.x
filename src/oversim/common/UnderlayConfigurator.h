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
 * @file UnderlayConfigurator.h
 * @author Stephan Krause, Bernhard Heep, Ingmar Baumgart
 */

#ifndef _UNDERLAYCONFIGURATOR_H_
#define _UNDERLAYCONFIGURATOR_H_

#include <omnetpp.h>

class PeerInfo;
class ChurnGenerator;
class NodeType;
class TransportAddress;
class GlobalNodeList;
class GlobalStatistics;

#ifndef timersub
# define timersub(a, b, result)                                               \
do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
        --(result)->tv_sec;                                                     \
       (result)->tv_usec += 1000000;                                           \
    }                                                                         \
} while (0)
#endif

/**
 * Base class for configurators of different underlay models
 *
 * @author Stephan Krause, Bernhard Heep
 */
class UnderlayConfigurator : public cSimpleModule
{
public:

    UnderlayConfigurator();
    virtual ~UnderlayConfigurator();

    /**
     * still in initialization phase?
     */
    bool isInInitPhase() { return init; };

    /**
     * Is the simulation ending soon?
     */
    bool isSimulationEndingSoon() { return simulationEndingSoon; };

    /**
     * Return the gracefulLeaveDelay
     */
    simtime_t getGracefulLeaveDelay() { return gracefulLeaveDelay; };


    bool isTransitionTimeFinished() { return transitionTimeFinished; };

    /**
     * Creates an overlay node
     *
     * @param type NodeType of the node to create
     * @param initialize are we in init phase?
     */
    virtual TransportAddress* createNode(NodeType type, bool initialize=false) = 0;

    /**
     * Notifies and schedules overlay nodes for removal.
     *
     * @param type NodeType of the node to remove
     * @param addr NULL for random node
     */
    virtual void preKillNode(NodeType type, TransportAddress* addr=NULL) = 0;

    /**
     * Migrates overlay nodes from one access net to another.
     *
     * @param type NodeType of the node to migrate
     * @param addr NULL for random node
     */
    virtual void migrateNode(NodeType type, TransportAddress* addr=NULL) = 0;

    void initFinished();

    ChurnGenerator* getChurnGenerator(int typeID);

    uint8_t getChurnGeneratorNum();

protected:

    /**
     * OMNeT number of init stages
     */
    int numInitStages() const;

    /**
     * OMNeT init methods
     */
    virtual void initialize(int stage);

    /**
     * Init method for derived underlay configurators
     */
    virtual void initializeUnderlay(int stage) = 0;

    virtual void handleTimerEvent(cMessage* msg);

    /**
     * Cleans up configurator
     */
    void finish();

    /**
     * Cleans up concrete underlay configurator
     */
    virtual void finishUnderlay();

    /**
     * Sets display string
     */
    virtual void setDisplayString() = 0;

    /**
     * Node mobility simulation
     *
     * @param msg timer-message
     */
    void handleMessage(cMessage* msg);

    int overlayTerminalCount; //!< current number of overlay terminals
    int firstNodeId; //!< the Id of the overlayTerminal created first in the overlay
    simtime_t gracefulLeaveDelay; //!< delay until scheduled node is removed from overlay
    double gracefulLeaveProbability; //!< probability that node is notified befor removal

    GlobalNodeList* globalNodeList; //!< pointer to GlobalNodeList
    GlobalStatistics* globalStatistics; //!< pointer to GlobalStatistics
    std::vector<ChurnGenerator*> churnGenerator; //!< pointer to the ChurnGenerators

    cMessage* endSimulationTimer; //!< timer to signal end of simulation
    cMessage* endSimulationNotificationTimer; //!< timer to notify nodes that simulation ends soon
    cMessage* endTransitionTimer; //!< timer to signal end of transition time

    struct timeval initFinishedTime; //!< timestamp at end of init phase
    struct timeval initStartTime; //!< timestamp at begin of init phase

    simtime_t transitionTime; //!< time to wait before measuring after init phase is finished
    simtime_t measurementTime; //!< duration of the simulation after init and transition phase

    static const int NUM_COLORS;
    static const char* colorNames[];

private:
    bool init;
    bool simulationEndingSoon;
    bool transitionTimeFinished;
    unsigned int initCounter;

    void consoleOut(const std::string& text);
};

#endif
