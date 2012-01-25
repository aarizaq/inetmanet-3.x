//
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
 * @file ZeroconfConnector.h
 * @author Bin Zheng + avahi example code
 */

#ifndef __ZEROCONFCONNECTOR_H__
#define __ZEROCONFCONNECTOR_H__

#undef HAVE_AVAHI

#ifdef HAVE_AVAHI

class BootstrapNodeHandle;
class NodeHandle;

#include <omnetpp.h>
#include <oversim_mapset.h>
#include <semaphore.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#define AVAHI_INIT_FAILED       0
#define AVAHI_INIT_SUCCEEDED    1

struct EqualStr
{
    bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) == 0;
    }
};

typedef std::pair<char *, BootstrapNodeHandle *> LocalNodePair;


typedef UNORDERED_MAP<char *, BootstrapNodeHandle*, HASH_NAMESPACE::hash<char *>,
                            EqualStr>  LocalBNodeSet;
/**
 * Zeroconf module
 *
 * Module for retrieving bootstrap nodes via Zeroconf (AVAHI connector)
 *
 * @author Bin Zheng
 * @see BootstrapList
 */
class ZeroconfConnector : public cSimpleModule
{
    friend void entry_group_callback(AvahiEntryGroup *, AvahiEntryGroupState, AVAHI_GCC_UNUSED void *);
    friend void create_services(AvahiClient *, ZeroconfConnector *);
    friend void resolv_callback(AvahiServiceResolver *,
                                AVAHI_GCC_UNUSED AvahiIfIndex,
                                AVAHI_GCC_UNUSED AvahiProtocol,
                                AvahiResolverEvent,
                                const char *,
                                const char *,
                                const char *,
                                const char *,
                                const AvahiAddress *,
                                uint16_t, AvahiStringList *,
                                AvahiLookupResultFlags,
                                AVAHI_GCC_UNUSED void*);
    friend void browse_callback(AvahiServiceBrowser *,
                                AvahiIfIndex,
                                AvahiProtocol,
                                AvahiBrowserEvent,
                                const char *,
                                const char *,
                                const char *,
                                AVAHI_GCC_UNUSED AvahiLookupResultFlags,
                                void *);
    friend void client_callback(AvahiClient *,
                                AvahiClientState,
                                AVAHI_GCC_UNUSED void *);

public:
    ZeroconfConnector();
    ~ZeroconfConnector();

    /**
     * Get initialization result of ZeroconfConnector
     *
     * @return AVAHI_INIT_FAILED or AVAHI_INIT_SUCCEEDED
     */
    int getInitResult();

    /**
     * Insert a new bootstrap node into the temporary list
     *
     * @param name unique name of the bootstrap node
     * @param node the new bootstrap node
     *
     * @return 0 if inserted with success, otherwise -1
     */
    int insertNode(char *name, BootstrapNodeHandle *node);

    /**
     * Remove a bootstrap node from the temporary list
     *
     * @param name unique name of the node to be removed
     *
     * @return 0 if removed with success, otherwise -1
     */
    int removeNode(char *name);

    /**
     * Announce bootstrap service in the local domain
     *
     * @param node pointer of the local overlay node
     */
    void announceService(const NodeHandle &node);

    /**
     * Withdraw bootstrap service in the local domain
     */
    void revokeService();

    /*
     * Returns true, if Zeroconf is used for bootstrapping
     *
     * @return true, if Zeroconf is enabled
     */
    bool isEnabled() { return enabled; };

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

private:
    int initResult;    //result of initialization

    sem_t nodeSetSem;        //semaphore that protects newSet
    LocalBNodeSet newSet;    //hash map to hold newly found boot nodes

    cMessage *pollingTimer;  //timer that controls periodic polling on newSet

    AvahiClient *client;    //avahi client
    AvahiEntryGroup *group;    //avahi group
    AvahiThreadedPoll *threadedPoll;    //avahi pool

    AvahiServiceBrowser *sbMDNS;    //mDNS service browser
    AvahiServiceBrowser *sbUDNS;    //uDNS service browser

    const char *serviceType;    //e.g. "_p2pbootstrap._udp"
    char *serviceName;    //name of the service
    const char *overlayName;    //e.g. "overlay.net"
    const char *overlayType;    //name of the overlay protocol

    const NodeHandle *thisNode;    //local overlay node
    bool enabled; // true, if ZeroconfConnector is enabled
};

#else

#include <NodeHandle.h>

/**
 * Zeroconf module
 *
 * Module for retrieving bootstrap nodes via Zeroconf (AVAHI connector)
 *
 * @author Bin Zheng
 * @see BootstrapList
 */
class ZeroconfConnector : public cSimpleModule
{
public:
    ZeroconfConnector() {};
    ~ZeroconfConnector() {};

    /**
     * Announce bootstrap service in the local domain
     *
     * @param node pointer of the local overlay node
     */
    void announceService(const NodeHandle &node) {};

    /**
     * Withdraw bootstrap service in the local domain
     */
    void revokeService() {};

    /*
     * Returns true, if Zeroconf is used for bootstrapping
     *
     * @return true, if Zeroconf is enabled
     */
    bool isEnabled() { return false; };
};
#endif

#endif
