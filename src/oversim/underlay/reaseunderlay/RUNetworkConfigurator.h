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
 * @file RUNetworkConfigurator.cc
 * @author Bernhard Müller et.al.
 *    Modification of TGMNetworkConfigurator.h provided by ReaSE.
 */


#ifndef RUNetworkConfigurator_H_
#define RUNetworkConfigurator_H_

#include <omnetpp.h>
#include <cctype>
#include <vector>
#include <map>
#include <ctopology.h>
#include <string>
#include <iostream>
#include "INETDefs.h"
#include "IPv4Address.h"
#include "RoutingTable.h"
#include "InterfaceTable.h"
#include "IPvXAddressResolver.h"
#include "NetworkConfigurator.h"
#include "IPv4InterfaceData.h"
#include "InterfaceEntry.h"

using std::vector;
using std::map;
using std::string;
using std::cerr;
using std::cout;
using std::endl;

const int INIT_STAGES = 3;
// unique hex values for different router- and AS-level node types
const int TRANSIT_AS = 1;
const int STUB_AS = 2;
const int UNSPECIFIED = -1;
const int CORE = 1;
const int GW = 2;
const int EDGE = 3;
const int ENDSYS = 4;

typedef vector<string> StringVector;

/**
 * @brief Structure that contains all information about a router-level node.
 *
 * During initialization all necessary information is extraced of the given node:
 * Does it belong to Stub or Transit AS, ID, router type.
 * In addition, default interfaces are determined for gateway, edge, and host
 * routers. Core routers do not have any default routes.
 */
struct nodeInfoRL
{
    bool isIPNode;
    IInterfaceTable *ift;
    InterfaceEntry *defaultRouteIE;
    int asId, asType, routerType, moduleId;
    IRoutingTable *rt;
    IPv4Address addr;
    bool usesDefaultRoute;
    cModule *module;
    cTopology::Node *node;

    nodeInfoRL() {};
    nodeInfoRL(cTopology::Node *node)
    {
        this->node = node;
        module = node->getModule();
        moduleId = module->getId();
        ift = IPvXAddressResolver().findInterfaceTableOf(module);
        rt = IPvXAddressResolver().findRoutingTableOf(module);
        isIPNode = (rt != NULL);
        int index = 0;
        string fullPath = module->getFullPath();

        // check if stubstring "sas" (StubAS) or "tas" (TransitAS)
        // is contained in fullPath
        if ( (index = fullPath.find("sas")) != -1 )
            asType = STUB_AS;
        else if ( (index = fullPath.find("tas")) != -1 )
            asType = TRANSIT_AS;
        else if ( (index = fullPath.find("ReaSEUnderlayNetwork")) != -1)
            asType = UNSPECIFIED;
        else {
            cerr << "found module that doesn't belong to Transit AS (tas) or Stub AS (sas): "<< fullPath<<endl;
            opp_error("found module that doesn't belong to Transit AS (tas) or Stub AS (sas)");
        }

        // set index to char position after substring "sas/tas"
        if (asType == STUB_AS || asType == TRANSIT_AS) {
            index += 3;
            string currentId;
            while (isdigit(fullPath[index]) && (index < (int) fullPath.length()))
                currentId += fullPath[index++];
            asId = atoi(currentId.data());
        }

        if (module->getProperties()->get("CoreRouter"))
            routerType = CORE;
        else if (module->getProperties()->get("GatewayRouter"))
            routerType = GW;
        else if (module->getProperties()->get("EdgeRouter"))
            routerType = EDGE;
        else if (module->getProperties()->get("Host"))
            routerType = ENDSYS;
        else {
            cerr<<"found module without valid type: "<<fullPath<<endl;
            opp_error("found module without valid type");
        }
        //
        // determine default interface
        //
        if (routerType == CORE) {
            // find last interface that is not loopback
            for (int i=0; i<ift->getNumInterfaces(); i++)
                if (!ift->getInterface(i)->isLoopback())
                    addr = ift->getInterface(i)->ipv4Data()->getIPAddress();
            defaultRouteIE = NULL;
        }
        else {
            for (int i=0; i<ift->getNumInterfaces(); i++) {
                if (!ift->getInterface(i)->isLoopback()) {
                    // find first interface that is not loopback and is connected to
                    // a higher level node. Then, create default route
                    addr = ift->getInterface(i)->ipv4Data()->getIPAddress();
                    if (routerType == GW) {
                        if (module->gate(ift->getInterface(i)->getNodeOutputGateId())\
                            ->getNextGate()->getOwnerModule()->getProperties()->get("CoreRouter")) {
                            defaultRouteIE = ift->getInterface(i);
                            break;
                        }
                    }
                    else if (routerType == EDGE) {
                        if (module->gate(ift->getInterface(i)->getNodeOutputGateId())\
                            ->getNextGate()->getOwnerModule()->getProperties()->get("GatewayRouter")) {
                            defaultRouteIE = ift->getInterface(i);
                            break;
                        }
                    }else if (routerType == ENDSYS) {
                        if (module->gate(ift->getInterface(i)->getNodeOutputGateId())\
                            ->getNextGate()->getOwnerModule()->getProperties()->get("EdgeRouter")) {
                            defaultRouteIE = ift->getInterface(i);
                            break;
                        }
                    }
                }
            }
        }
    };

};

struct edgeRouter
{
    unsigned int edgeIP;
    int usedIPs;
    cModule *module;
};

typedef std::vector<nodeInfoRL> NODE_INFO_RL_VEC;
typedef std::map<int, nodeInfoRL> NODE_MAP;
typedef std::pair<int, nodeInfoRL> NODE_MAP_PAIR;
typedef std::vector<edgeRouter> EDGE_ROUTER_VEC;

/**
 * @brief Structure that contains all information about an AS-level node.
 *
 * During initialization all necessary information is extraced of the given node:
 * Router type and ID.
 */
struct nodeInfoAS
{
    int id;
    int asType;
    cTopology::Node *node;
    cModule *module;
    NODE_MAP nodeMap;
    NODE_INFO_RL_VEC coreNode;
    IPv4Address addr;
    IPv4Address netmask;
    IPv4Address subnetmask;
    EDGE_ROUTER_VEC edgeRouter;

    nodeInfoAS(cTopology::Node *node, IPv4Address a, IPv4Address m) {
        this->node = node;
        this->module = node->getModule();
        addr = a;
        netmask = m;
        int index = 0;
        string fullPath = node->getModule()->getFullPath();

        // check if stubstring "sas" (StubAS) or "tas" (TransitAS)
        // is contained in fullPath
        if ( (index = fullPath.find(".sas")) != -1 )
            asType = STUB_AS;
        else if ( (index = fullPath.find(".tas")) != -1 )
            asType = TRANSIT_AS;
        else if ( (index = fullPath.find("ReaSEUnderlayNetwork")) != -1 )
            asType = UNSPECIFIED;
        else
        {
            cerr << "found module that doesn't belong to TAS or SAS: "<< fullPath<<endl;
            opp_error("found module that doesn't belong to TAS or SAS");
        }

        // set index to char position after substring "sas/tas"
        if (asType == STUB_AS || asType == TRANSIT_AS)
        {
            index += 3;
            string currentId;
            while (isdigit(fullPath[index]) && (index < (int) fullPath.length()))
                currentId += fullPath[index++];
            id = atoi(currentId.data());
        }
    }
};


typedef std::vector<nodeInfoAS> NODE_INFO_AS_VEC;

/**
 * @brief Configures the nodes belonging to the topology before starting
 * the actual simulation.
 *
 * This class is responsible for assignment of IP addresses to all nodes
 * of the topology. Furthermore, routing tables have to be filled and
 * default routing paths must be created.
 * Routing is separated into Intra-AS and Inter-AS routing.
 *
 * @class RUNetworkConfigurator
 */
class RUNetworkConfigurator : public cSimpleModule
{
protected:
    std::vector<cTopology*> rlTopology;
    cTopology asTopology;
    int noAS;
    int nextPow; //<! number bits for AS addressing
    NODE_INFO_AS_VEC asNodeVec;
    unsigned int IP_NET_SHIFT; //<! number of bits reserved for AS addressing
    uint32_t NET_MASK; //<! netmask calculated by netshift
public:
    RUNetworkConfigurator();
    virtual ~RUNetworkConfigurator();

protected:
    //
    // stage = 0 --> register interfaces
    //
    virtual int numInitStages() const {return 3;}
    /**
     * Main method of the network configurator.
     *
     * Topology is extracted from NED file, IP addresses are assigned,
     * and routing paths are established.
     */
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg) {opp_error("message received");};
    /** @brief Add Inter-AS routing paths between core nodes
     *
     * Calculate all Inter-AS. This is achieved by calculating all shortest paths between
     * all core routers of the whole topology.
     * Temporarily disable all stub links during calculation of shortest paths to ensure
     * that a stub AS is not crossed but may only be present at start or end of a routing
     * path.
     */
    void createInterASPaths();
    /** @brief Disable all incoming links of Stub AS except to and from dst and src
     *
     * Disable all incoming links to core nodes of each stub AS that do not
     * match on given dst or src router-level node ID.
     */
    void disableStubLinks(nodeInfoRL &dst, nodeInfoRL &src);
    /** @brief Enable all incoming links of Stub AS
     *
     * Enable all incoming links to core nodes of each stub AS
     */
    void enableStubLinks();
    /** @brief Extract topology from NED file
     *
     * Extracts AS-level topology and each router-level topology
     * into asTopology and rlTopology.
     * Additionally, each AS gets assigned a unique calculated prefix
     */
    void extractTopology();
    /** @brief Assign IP address and add default route
     *
     * @brief Assigns an IP address of the calculated prefix to each of the router-level nodes.
     *
     * Additionally, default routes are added for gateway, edge, and host nodes.
     * Core nodes are stored into an additional list for later processing.
     *
     * @param asInfo AS for which IP addresses should be assigned to router-level nodes.
     */
    void assignAddressAndSetDefaultRoutes(nodeInfoAS &asInfo);
    /** @brief Add explicit Intra-AS routing paths (except of default routes)
     *
     * Calculate all Intra-AS routes that are unequal to the default routes.
     * Therefore, all shortest paths between all router-level nodes are calculated.
     * If the first hop is unequal to the default route, a new specific route is added.
     *
     * @param topology
     * @param asInfo AS for which Intra-AS routes should be determined
     */
    void setIntraASRoutes(cTopology &topology, nodeInfoAS &asInfo);

};
namespace RUNetConf {

/**
 * Callback method that is used by extractFromNetwork. This method
 * includes all nodes for which the callback method returns a non-zero
 * value. The second argument is given to the callback method as second
 * argument.
 * Our callback method returns a topology consisting of all core nodes.
 * It does so by searching for the CoreRouter property.
 *
 * @return Returns 1 for nodes that are included into the topology,
 *         0 for nodes that are ignored
 */
static bool getCoreNodes(cModule *curMod, void *nullPointer);
/**
 * Callback method that is used by extractFromNetwork. This method
 * includes all nodes for which the callback method returns a non-zero
 * value. The second argument is given to the callback method as second
 * argument.
 * Our callback method returns a topology consisting of all router-level nodes
 * (core, gateway, edge, host, and servers) that belong to the given AS.
 * It does so by searching for the RL property within the given AS.
 *
 * @return Returns 1 for nodes that are included into the topology,
 *         0 for nodes that are ignored
 */
static bool getRouterLevelNodes(cModule *curMod, void *name);
};
#endif /*RUNetworkConfigurator_H_*/
