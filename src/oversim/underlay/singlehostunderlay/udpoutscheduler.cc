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
 * @file udpoutscheduler.cc
 * @author Stephan Krause
 */

#include "udpoutscheduler.h"
#include "IPvXAddress.h"
#include <regmacros.h>

Register_Class(UdpOutScheduler);

// Note: this is defined in tunoutscheduler.cc
extern cConfigOption *CFGID_EXTERNALAPP_APP_PORT;

UdpOutScheduler::~UdpOutScheduler()
{
    if (additional_fd >= 0) {
#ifdef _WIN32
        closesocket(additional_fd);
#else
        close(additional_fd);
#endif
    }
}

int UdpOutScheduler::initializeNetwork()
{
    // get app port (0 if external app is not used)
    int appPort = ev.getConfig()->getAsInt(CFGID_EXTERNALAPP_APP_PORT, 0);

    // Initialize TCP socket for App communication if desired
    if (appPort > 0) {

        struct sockaddr_in server;
        SOCKET sock;

        // Waiting for a TCP connection
        sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock == INVALID_SOCKET) {
            opp_error("Error creating socket");
            return -1;
        }

        int on = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

        memset(&server, 0, sizeof (server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        server.sin_port = htons(appPort);

        if (bind( sock, (struct sockaddr*)&server, sizeof( server)) < 0) {
            opp_error("Error binding to app socket");
            return -1;
        }

        if (listen( sock, 5 ) == -1) {
            opp_error("Error listening on app socket");
            return -1;
        }
        // Set additional_fd so we will be called if data
        // (i.e. connection requests) is available at sock
        additional_fd = sock;
        FD_SET(additional_fd, &all_fds);
        if (additional_fd > maxfd) {
            maxfd = additional_fd;
        }
    }

    // Open UDP port
    if (netw_fd != INVALID_SOCKET) {
        // Port is already open, reuse it...
        FD_SET(netw_fd, &all_fds);

        if (netw_fd> maxfd) {
            maxfd = netw_fd;
        }

        return 0;
    }

    sockaddr_in addr;
    netw_fd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    cModule* overlay = simulation.getModuleByPath(
            "SingleHostUnderlayNetwork.overlayTerminal[0].overlay");

    if (overlay == NULL) {
        throw cRuntimeError("UdpOutScheduler::initializeNetwork(): "
                                "Overlay module not found!");
    }

    addr.sin_port = htons(overlay->gate("appIn")->getNextGate()->
                          getOwnerModule()->par("localPort").longValue());

    cModule* underlayConfigurator =
        simulation.getModuleByPath("SingleHostUnderlayNetwork.underlayConfigurator");

    if (underlayConfigurator == NULL) {
        throw cRuntimeError("UdpOutScheduler::initializeNetwork(): "
                                "UnderlayConfigurator module not found!");
    }

    if (strlen(underlayConfigurator->par("nodeIP").stringValue())) {
        addr.sin_addr.s_addr = htonl(IPv4Address(underlayConfigurator->
                                       par("nodeIP").stringValue()).getInt());
    } else {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    if (bind( netw_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        opp_error("Error binding to UDP socket");
        return -1;
    }

    FD_SET(netw_fd, &all_fds);

    if (netw_fd> maxfd) {
        maxfd = netw_fd;
    }

    return 0;
}

void UdpOutScheduler::additionalFD() {
    sockaddr* from = (sockaddr*) new sockaddr_in;
    socklen_t addrlen = sizeof(sockaddr_in);

    SOCKET new_sock = accept( additional_fd, from, &addrlen );

    if (new_sock == INVALID_SOCKET) {
        opp_warning("Error connecting to remote app");
        return;
    }

    if (appConnectionLimit) {
        int count = 0;

        for (SOCKET fd = 0; fd <= maxfd; fd++) {
            if (fd == netw_fd) continue;
            if (fd == additional_fd) continue;
            if (FD_ISSET(fd, &all_fds)) count++;
        }

        if (count >= appConnectionLimit) {
            // We already have too many connections to external applications
            // "reject" connection
#ifdef _WIN32
            closesocket(new_sock);
#else
            close(new_sock);
#endif
            ev << "[UdpOutScheduler::additionalFD()]\n"
               << "    Rejecting new app connection (FD: " << new_sock << ")"
               << endl;

            return;
        }
    }

    FD_SET(new_sock, &all_fds);

    if (new_sock > maxfd) {
        maxfd = new_sock;
    }

    // Inform app about new connection
    appPacketBuffer->push_back(PacketBufferEntry(0, 0, from, addrlen,
                               PacketBufferEntry::PACKET_FD_NEW, new_sock));

    sendNotificationMsg(appNotificationMsg, appModule);

    ev << "[UdpOutScheduler::additionalFD()]\n"
       << "    Accepting new app connection (FD: " << new_sock << ")"
       << endl;
}
