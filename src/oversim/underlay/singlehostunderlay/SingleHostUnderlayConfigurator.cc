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
 * @file SingleHostUnderlayConfigurator.cc
 * @author Stephan Krause
 * @author Ingmar Baumgart
 * @author Bernhard Heep (migrateRandomNode)
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define WANT_WINSOCK2
#include <platdep/sockets.h>

#ifndef _WIN32
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include <stun/stun_udp.h>
#include <stun/stun.h>

#include <omnetpp.h>

#include "IInterfaceTable.h"
#include "InterfaceEntry.h"
#include "IPv4InterfaceData.h"
#include "IPvXAddressResolver.h"

#include <PeerInfo.h>
#include <IRoutingTable.h>
#include <NodeHandle.h>
#include <GlobalNodeListAccess.h>

#include "SingleHostUnderlayConfigurator.h"

Define_Module(SingleHostUnderlayConfigurator);

void SingleHostUnderlayConfigurator::initializeUnderlay(int stage)
{
    StunAddress4 publicAddr, stunServerAddr;
    IPvXAddress addr;

    if(stage != MAX_STAGE_UNDERLAY)
        return;

    overlayTerminalCount = 1;

    // Set IP, Routes, etc
    cModule* node = getParentModule()->getSubmodule("overlayTerminal", 0);

    std::string nodeInterface = std::string(par("nodeInterface").stringValue());
    std::string stunServer = std::string(par("stunServer").stringValue());
    std::string nodeIP = std::string(par("nodeIP").stringValue());

    if (!nodeInterface.size() && !stunServer.size() && !nodeIP.size()) {
        throw cRuntimeError("SingleHostConfigurator::initializeUnderlay(): "
                            "Please specify at least one of "
                            "**.nodeInterface, **.nodeIP, or **.stunServer");
    }

    if (nodeInterface.size()) {
#ifndef _WIN32
        SOCKET sock_fd;
        struct ifreq req;
        if ((sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP)) !=
            INVALID_SOCKET) {

            snprintf(req.ifr_name, sizeof(req.ifr_name), "%s", nodeInterface.c_str());

            if (!ioctl(sock_fd, SIOCGIFADDR, &req)) {
                addr = IPv4Address(inet_ntoa(((struct sockaddr_in *)
                        (&req.ifr_addr))->sin_addr));
            } else {
                throw cRuntimeError("SingleHostConfigurator::"
                                        "initializeUnderlay(): "
                                        "Invalid interface!");
            }
            close(sock_fd);
        }
#else
        throw cRuntimeError("SingleHostConfigurator::"
                "initializeUnderlay(): "
                "**.nodeInterface parameter not supported on WIN32 yet!");
#endif
    } else if (nodeIP.size()) {
        addr = IPv4Address(nodeIP.c_str());
    }

    if (stunServer.size()) {
        // TODO: use and set overlayPort!
        throw cRuntimeError("SingleHostConfigurator::initializeUnderlay():"
                                " Not implemented yet!");
        int srcPort = 0;
        publicAddr.addr = 0;
        publicAddr.port = srcPort;
        stunServerAddr.addr = 0;
        char tmpAddr[512];
        strncpy (tmpAddr, stunServer.c_str(), 512);

        if (stunParseServerName(tmpAddr, stunServerAddr)) {
            bool presPort = false;
            bool hairpin = false;

            NatType stype = stunNatType(stunServerAddr, false, &presPort,
                                        &hairpin, srcPort, &publicAddr);
            if (stype != StunTypeOpen) {
                EV << "SingleHostConfigurator::initializeUnderlay(): "
                   << "Node is behind NAT or invalid STUN server configuration!"
                   << std::endl;
            }

            publicAddr.addr = htonl(publicAddr.addr);
            addr = IPv4Address(inet_ntoa(*((struct in_addr *)(&(publicAddr.addr)))));
        }
    }

    IPvXAddress gw = addr;
    InterfaceEntry* ifEntry = IPvXAddressResolver().interfaceTableOf(node)->
	    getInterfaceByName("outDev");
    IRoutingTable* rTable = check_and_cast<IRoutingTable*>(node->getSubmodule("routingTable", 0));

    ifEntry->ipv4Data()->setIPAddress(addr.get4());
    ifEntry->ipv4Data()->setNetmask(IPv4Address::ALLONES_ADDRESS);

    IPv4Route* te = new IPv4Route();
    te->setDestination(IPv4Address::UNSPECIFIED_ADDRESS);
    te->setNetmask(IPv4Address::UNSPECIFIED_ADDRESS);
    te->setGateway(gw.get4());
    te->setInterface(ifEntry);
    te->setType(IPv4Route::REMOTE);
    te->setSource(IPv4Route::MANUAL);
    rTable->addRoute(te);

    //add node to bootstrap oracle
    PeerInfo* info = new PeerInfo(0, node->getId(), NULL);
    globalNodeList->addPeer(addr, info);

    if (strlen(par("bootstrapIP")) > 0) {
        PeerInfo* bootstrapInfo = new PeerInfo(0, -1, NULL);
        globalNodeList->addPeer(IPv4Address(par("bootstrapIP").stringValue()),
                                bootstrapInfo);

        globalNodeList->registerPeer(NodeHandle(OverlayKey::ONE,
            IPv4Address(par("bootstrapIP").stringValue()), par("bootstrapPort")));
    }

    // update display
    setDisplayString();

    scheduleAt(simTime(), new cMessage("init phase finished"));
}
void SingleHostUnderlayConfigurator::handleTimerEvent(cMessage* msg)
{
    if (std::string(msg->getName()) != "init phase finished") {
        throw cRuntimeError("unknown self-message received");
    }
    delete msg;

    initFinished();
}

void SingleHostUnderlayConfigurator::setDisplayString()
{
    // updates the statistics display string
    char buf[80];
    sprintf(buf, "%i overlay terminals", overlayTerminalCount);
    getDisplayString().setTagArg("t", 0, buf);
}

void SingleHostUnderlayConfigurator::finishUnderlay()
{
    if (!isInInitPhase()) {
        struct timeval now, diff;
        gettimeofday(&now, NULL);
        timersub(&now, &initFinishedTime, &diff);
        printf("Simulation time: %li.%06li\n", diff.tv_sec, diff.tv_usec);
    }
}
