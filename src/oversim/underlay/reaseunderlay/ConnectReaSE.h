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
 * @file ConnectReaSE.h
 * @author Markus Mauch, Bernhard Mueller
 */

#ifndef CONNECTREASE_H_
#define CONNECTREASE_H_

#include <omnetpp.h>

#include <InitStages.h>


class IInterfaceTable;
class InterfaceEntry;
class IRoutingTable;
class IPv4Route;



struct edgeRoutes
{
public:
        int countPPPInterfaces;
        IInterfaceTable* interfaceTable; //!< pointer to interface table of this node
        IRoutingTable* routingTable; //!< pointer to routing table of this node
        uint32 IPAddress; //!< the IP Address
        uint32 lastIP;  //!< last assigned IP address FIXME: check overlays for side effects of reused IP addresses
        cModule* Router;
        std::vector<uint32> IPAddresses; //!< the IP Addresses in use of edge router
        std::string channelTypeRxStr; //!< the current active channel type (rx)
        std::string channelTypeTxStr; //!< the current active channel type (tx)
};

struct autoSystem
{
public:
    //cModule* AS;
    std::vector<edgeRoutes> edgeRouter;
    uint32 edgeShift;
};

struct topologyProperty
{
public:
    cModule* pModule;
    const char* property;
};

struct edgePool
{
public:
    edgeRoutes* edge;
    uint32 indexAS;
};


class terminalInfo
{
public:
    uint32 IPAddress; //!< the IP Address
    cModule* module;
    IInterfaceTable* interfaceTable; //!< pointer to interface table of this node
    IRoutingTable* routingTable; //!< pointer to routing table of this node
    cModule* PPPInterface; //!< pointer to PPP module
    cModule* remotePPPInterface; //!< pointer to remote PPP module
    InterfaceEntry* interfaceEntry; //!< pointer to interface entry
    InterfaceEntry* remoteInterfaceEntry; //!< pointer to remote interface entry
    IInterfaceTable* remoteInterfaceTable; //!< pointer to remote interface table
    IPv4Route* routingEntry; //!< pointer to routing entry
    IPv4Route* remoteRoutingEntry;
    edgeRoutes* edgeRouter; //!< pointer to connected edge router
    int ASindex; // deleteable?
    simtime_t createdAt; //!< creation timestamp

    friend std::ostream& operator<<(std::ostream& os, terminalInfo& n);
};

class AccessInfo
{
public:
    edgeRoutes* edge;
    cModule* terminal;
    uint32 IPAddress;
    int ASindex;
};

class ConnectReaSE : public cSimpleModule
{

public:
    /**
     * Gathers some information about the terminal and appends it to
     * the overlay terminal vector.
     *
     */
    virtual int addOverlayNode(AccessInfo* overlayNode, bool migrate = false);
    /**
     * Getter for router module
     *
     * @return pointer to router module
     */
    virtual AccessInfo getAccessNode();
    /**
     * Removes a node from the network
     */
    virtual cModule* removeOverlayNode(int ID);
    /**
     * searches overlayTerminal[] for a given node
     *
     * @param ID position of the node in overlayTerminal
     * @return the nodeId if found, -1 else
     */
    virtual cModule* getOverlayNode(int ID);
    /*
     * migrates given node to a different edge router
     */
    virtual AccessInfo migrateNode(int ID);

protected:
    /**
         * OMNeT number of init stages
         *
         * @return needed number of init stages
         */
        virtual int numInitStages() const
        {
            return MAX_STAGE_UNDERLAY + 1;
        }

        /**
         * Gather some information about the router node.
         */
        virtual void initialize(int stage);

        /**
         * OMNeT handleMessage method
         *
         * @param msg the message to handle
         */
        virtual void handleMessage(cMessage* msg);

        /**
         * Displays the current number of terminals connected to the network
         */
        virtual void updateDisplayString();

        std::vector<std::string> channelTypesRx; //!< vector of possible access channels (rx)

        std::vector<std::string> channelTypesTx; //!< vector of possible access channels (tx)


        std::vector<terminalInfo> overlayTerminal; //!< the terminals at this access router

        // statistics
        cOutVector lifetimeVector; //!< vector of node lifetimes

        std::vector<autoSystem> AS_Pool; //<! list of autonomous systems of the topology
        std::vector<edgePool> globalEdgePool; //<!  dedicated list of all connectable routers
        uint32 totalCountOfAS, nextPow, ASShift;
        double channelDiversity; //<! percentage that a channel delay can differ

private:
    /**
     * Returns a module's fist unconnected gate
     *
     * @param owner gate owner module
     * @param name name of the gate vector
     * @param type gate type (input or output)
     */
    cGate* firstUnusedGate(cModule* owner, const char* name, cGate::Type type = cGate::NONE);
    /**
     * @brief Gathers all needed edge router information of a specified autonomous system.
     *
     * This method gathers all edge routers, their IPs and the IPs of all connected hosts.
     *
     * @param currAS AS module
     */
    void setUpAS(cModule* currAS);
    /**
     * @brief Finds submodules with special properties.
     *
     * @param currModule currently tested module
     * @param properties pointer to topologyProperty (parent module and special property)
     */
    static bool extractFromParentModule(cModule* currModule, void * properties);

};


#endif /* CONNECTREASE_H_ */
