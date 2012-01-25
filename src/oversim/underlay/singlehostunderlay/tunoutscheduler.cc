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
 * @file tunoutscheduler.cc
 * @author Stephan Krause
 */

#include "tunoutscheduler.h"

Register_Class(TunOutScheduler);

// Note: this is defined in tunoutscheduler.cc
extern cConfigOption *CFGID_EXTERNALAPP_APP_PORT;

TunOutScheduler::~TunOutScheduler()
{
    if (additional_fd >= 0) {
#ifdef _WIN32
        closesocket(additional_fd);
#else
        close(additional_fd);
#endif
    }

    delete dev;
}


int TunOutScheduler::initializeNetwork()
{
#if defined _WIN32 || defined __APPLE__
    throw cRuntimeError("TunOutSchedulter::initializeNetwork():"
                        "TUN interface not supported on Windows/Max OS!");
    return -1;
#else
    // Initialize TUN device for network communication
    // see /usr/src/linux/Documentation/network/tuntap.txt
    struct ifreq ifr;
    int err;
    dev = new char[IFNAMSIZ];

    // get app port (0 if external app is not used)
    int appPort = ev.getConfig()->getAsInt(CFGID_EXTERNALAPP_APP_PORT, 0);

    // Initialize TCP socket for App communication if desired
    if (appPort > 0) {
        struct sockaddr_in server;
        SOCKET sock;

        // Waiting for a TCP connection
        // WARNING: Will only accept exactly ONE app connecting to the socket
        sock = socket( AF_INET, SOCK_STREAM, 0 );
        if (sock == INVALID_SOCKET) {
            opp_error("Error creating socket");
            return -1;
        }

        int on = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        memset( &server, 0, sizeof (server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl( INADDR_ANY );
        server.sin_port = htons( appPort );

        if (bind( sock, (struct sockaddr*)&server, sizeof( server)) < 0) {
            opp_error("Error binding to app socket");
            return -1;
        }
        if (listen( sock, 5 ) == -1 ) {
            opp_error("Error listening on app socket");
            return -1;
        }
        // Set additional_fd so we will be called if data
        // (i.e. connection requests) is available at sock
        additional_fd = sock;
        FD_SET(additional_fd, &all_fds);
        if (additional_fd > maxfd) maxfd = additional_fd;
    }

    if (netw_fd != INVALID_SOCKET) {
        opp_error("Already bound to TUN device!");
        return -1;
    }

    if ((netw_fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
        opp_warning("Error opening tun device");
        return 0;
    } else {
        ev << "[TunOutScheduler::initializeNetwork()]\n"
        << "\t Successfully opened TUN device"
        << endl;
    }

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, "tun%d", IFNAMSIZ);

    if((err = ioctl(netw_fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
        close(netw_fd);
        opp_error("Error ioctl tun device");
        return -1;
    }

    strncpy(dev, ifr.ifr_name, IFNAMSIZ);
    ev << "[TunOutScheduler::initializeNetwork()]\n"
       << "    Bound to device " << dev << "\n"
       << "    Remember to bring up TUN device with ifconfig before proceeding"
       << endl;

    FD_SET(netw_fd, &all_fds);
    if( netw_fd> maxfd ) maxfd = netw_fd;
    return 0;
#endif
}

void TunOutScheduler::additionalFD() {
    sockaddr* from = (sockaddr*) new sockaddr_in;
    socklen_t addrlen = sizeof(sockaddr_in);

    SOCKET new_sock = accept( additional_fd, 0, 0 );
    if (new_sock == INVALID_SOCKET) {
        opp_warning("Error connecting to remote app");
        return;
    }
    if (appConnectionLimit) {
        int count = 0;
        for (SOCKET fd = 0; fd < maxfd; fd++) {
            if( fd == netw_fd ) continue;
            if( fd == additional_fd ) continue;
            if( FD_ISSET(fd, &all_fds)) count++;
        }
        if( count > appConnectionLimit ) {
            // We already have too many connections to external applications
            // "reject" connection
            close(new_sock);
            ev << "[UdpOutScheduler::additionalFD()]\n"
                << "    Rejecting new app connection (FD: " << new_sock << ")"
                << endl;
            return;
        }
    }

    FD_SET(new_sock, &all_fds);
    if( new_sock > maxfd ) maxfd = new_sock;

    // Inform app about new connection
    appPacketBuffer->push_back(PacketBufferEntry(0, 0, from, addrlen,
                                         PacketBufferEntry::PACKET_FD_NEW, new_sock));
    sendNotificationMsg(appNotificationMsg, appModule);

    ev << "[UdpOutScheduler::additionalFD()]\n"
        << "    Accepting new app connection (FD: " << new_sock << ")"
        << endl;

}

