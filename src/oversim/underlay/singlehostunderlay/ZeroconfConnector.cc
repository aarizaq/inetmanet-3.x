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
 * @file ZeroconfConnector.cc
 * @author Bin Zheng + avahi example code
 */

#include "ZeroconfConnector.h"

Define_Module(ZeroconfConnector);

#ifdef HAVE_AVAHI

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <BaseOverlay.h>
#include <GlobalNodeListAccess.h>
#include <BootstrapList.h>

using namespace std;

// TODO cleanup code
void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {
    AvahiClient *client = avahi_entry_group_get_client(g);
    assert(client);
    ZeroconfConnector *zConfigurator = (ZeroconfConnector *)userdata;
    assert(zConfigurator);

    /* Called whenever the entry group state changes */
    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            cerr << "Service " << zConfigurator->serviceName << " successfully established." << endl;
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : {
            char *n;

            /* A service name collision happened. Pick a new name */
            n = avahi_alternative_service_name(zConfigurator->serviceName);
            avahi_free(zConfigurator->serviceName);
            zConfigurator->serviceName = n;

            cerr << "Service name collision, renaming service to " << n  << "." << endl;

            /* Recreate the services with new name*/
            create_services(client, zConfigurator);
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            cerr << "Entry group failure: " << avahi_strerror(avahi_client_errno(client)) << endl;

            avahi_threaded_poll_quit(zConfigurator->threadedPoll);
            break;

        default:
            break;
    }
}


void create_services(AvahiClient *c, ZeroconfConnector *zConfigurator) {
    char *n, r[3][128];
    int ret;

    assert(c);
    assert(zConfigurator);

    if (!zConfigurator) {
        cerr << "ZeroconfConnector not available, can not create service" << endl;
        return;
    }

    /*
     * If this is the first time we're called, let's create a new
     * entry group if necessary
     */
    if (!zConfigurator->group)
        if (!(zConfigurator->group = avahi_entry_group_new(c, entry_group_callback, zConfigurator))) {
            cerr << "avahi_entry_group_new() failed: " << avahi_strerror(avahi_client_errno(c)) << endl;
            goto fail;
        }

    /*
     * If the group is empty (either because it was just created, or
     * because it was reset previously, add our entries
     */
    if (avahi_entry_group_is_empty(zConfigurator->group)) {
        cerr << "Adding service " << zConfigurator->serviceName << endl;

        snprintf(r[0], sizeof(r[0]), "peerid=%s", (zConfigurator->thisNode->key.toString(16)).c_str());
        snprintf(r[1], sizeof(r[1]), "overlayid=%s", zConfigurator->overlayName);
        snprintf(r[2], sizeof(r[2]), "overlaytype=%s", zConfigurator->overlayType);


        if ((ret = avahi_entry_group_add_service(zConfigurator->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,(AvahiPublishFlags)0,
            zConfigurator->serviceName, zConfigurator->serviceType, NULL, NULL, zConfigurator->thisNode->getPort(), r[0], r[1], r[2], NULL)) < 0) {

            if (ret == AVAHI_ERR_COLLISION)
                goto collision;

            cerr << "Failed to add _p2pbootstrap._udp service: " << avahi_strerror(ret) << endl;
            goto fail;
        }

        /* Tell the server to register the service */
        if ((ret = avahi_entry_group_commit(zConfigurator->group)) < 0) {
            cerr << "Failed to commit entry group: " << avahi_strerror(ret) << endl;
            goto fail;
        }
    }

    return;

collision:


    /* A service name collision happened. Pick a new name */
    n = avahi_alternative_service_name(zConfigurator->serviceName);
    avahi_free(zConfigurator->serviceName);
    zConfigurator->serviceName = n;

    cerr << "Service name collision, renaming service to " << n << endl;

    avahi_entry_group_reset(zConfigurator->group);

    create_services(c, zConfigurator);
    return;

fail:
    avahi_threaded_poll_quit(zConfigurator->threadedPoll);
}

void resolv_callback(
    AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void* userdata) {

    char *peerID = NULL;
    char *overlayType = NULL;
    AvahiStringList *record;
    AvahiClient *client = NULL;
    ZeroconfConnector *zConfigurator = NULL;

    assert(r);

    client = avahi_service_resolver_get_client(r);
    assert(client);

    zConfigurator = (ZeroconfConnector *)userdata;
    assert(zConfigurator);


    /* Called whenever a service has been resolved successfully or timed out */
    switch (event) {
        case AVAHI_RESOLVER_FAILURE:
            cerr << "(Resolver) Failed to resolve service " << name << " of type " << type << " in domain " <<
                domain << ": " << avahi_strerror(avahi_client_errno(client)) << endl;
            break;

        case AVAHI_RESOLVER_FOUND: {
            char a[AVAHI_ADDRESS_STR_MAX], *t;

            cout << "Service " << name << " of type " << type << " in domain " << domain << endl;

            avahi_address_snprint(a, sizeof(a), address);
            t = avahi_string_list_to_string(txt);
            fprintf(stderr,
                    "\t%s:%u (%s)\n"
                    "\tTXT=%s\n"
                    "\tcookie is %u\n"
                    "\tis_local: %i\n"
                    "\tour_own: %i\n"
                    "\twide_area: %i\n"
                    "\tmulticast: %i\n"
                    "\tcached: %i\n",
                    host_name, port, a,
                    t,
                    avahi_string_list_get_service_cookie(txt),
                    !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
                    !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
                    !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
                    !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
                    !!(flags & AVAHI_LOOKUP_RESULT_CACHED));

            avahi_free(t);

            /* parse the TXT record to search for the overlay algorithm and nodeID of the node */
            for (record = txt; record; record = record->next) {
                if (!strncmp((char *)record->text, "overlaytype", 11)) {
                    if ((char)record->text[11] == '=') {
                        overlayType = avahi_strdup((const char *)(record->text + 12));
                    }
                } else {
                    if (!strncmp((char *)record->text, "peerid", 6)) {
                        if (record->text[6] == '=') {
                            peerID = avahi_strdup((const char *)(record->text + 7));
                        }
                    }
                }
            }

            if (!overlayType || !peerID || strncmp(overlayType, zConfigurator->overlayType,
                strlen(overlayType))) {
                cerr << "TXT record of the node is defect, or node does not use the desired p2p algorithm." << endl;

                if (overlayType)
                    avahi_free(overlayType);

                if (peerID)
                    avahi_free(peerID);

                break;
            }

            if (zConfigurator) {
                BootstrapNodeHandle *node = new BootstrapNodeHandle(OverlayKey(peerID, 16),
                    IPvXAddress(a), (int)port,
                    !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA) ? DNSSD : MDNS);

                if (node) {
                    zConfigurator->insertNode(avahi_strdup(name), node);
                } else {
                    cerr << "Failed to allocate memory for NodeHandle" << endl;
                }
            }

            avahi_free(overlayType);
            avahi_free(peerID);

            break;
        }

        default:
            break;
    }

    avahi_service_resolver_free(r);

    return;
}

void browse_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiClient *c = NULL;
    ZeroconfConnector *zConfigurator = NULL;
    assert(b);

    c = avahi_service_browser_get_client(b);
    assert(c);

    zConfigurator = (ZeroconfConnector *)userdata;
    assert(zConfigurator);

    /* Called whenever a new services becomes available on the LAN or is removed from the LAN */
    switch (event) {
        case AVAHI_BROWSER_FAILURE:
            cerr << "(Browser) " << avahi_strerror(avahi_client_errno(c)) << endl;
            avahi_threaded_poll_quit(zConfigurator->threadedPoll);
            return;

        case AVAHI_BROWSER_NEW:
            cerr << "(Browser) NEW: service " << name << " of type " << type << " in domain " << domain << endl;

            if (strcmp(name, zConfigurator->serviceName)) {
            	if (!(avahi_service_resolver_new(c, interface, protocol, name, type, domain, AVAHI_PROTO_INET, (AvahiLookupFlags)0, resolv_callback, zConfigurator)))
                    cerr << "Failed to resolve service " << name << ":" << avahi_strerror(avahi_client_errno(c)) << endl;
            }
            break;

        case AVAHI_BROWSER_REMOVE:
            cerr << "(Browser) REMOVE: service " << name << " of type " << type << " in domain " << domain << endl;
            zConfigurator->removeNode((char *)name);
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            cerr << "(Browser) " << (event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW") << endl;
            break;

        default:
            break;
    }
}


void client_callback(
    AvahiClient *c,
    AvahiClientState state,
    AVAHI_GCC_UNUSED void * userdata) {
    ZeroconfConnector *zConfigurator = NULL;

    assert(c);
    zConfigurator = (ZeroconfConnector *)userdata;
    assert(zConfigurator);

    /* Called whenever the client or server state changes */
    switch (state) {
        case AVAHI_CLIENT_FAILURE:
            cerr <<  "Client failure: " << avahi_strerror(avahi_client_errno(c)) << endl;
            avahi_threaded_poll_quit(zConfigurator->threadedPoll);

            break;

        case AVAHI_CLIENT_S_REGISTERING:
            if (zConfigurator->group)
                avahi_entry_group_reset(zConfigurator->group);

            break;

        default:
           break;
    }
}


ZeroconfConnector::ZeroconfConnector()
{
    initResult = AVAHI_INIT_FAILED;

    client = NULL;
    group = NULL;
    threadedPoll = NULL;

    sbMDNS = sbUDNS = NULL;

    serviceType = overlayName = overlayType = NULL;
    serviceName = NULL;
    thisNode = NULL;

    if (sem_init(&nodeSetSem, 0, 1) == -1) {
        cerr << "nodeSetSem can't be initialized." << endl;
    }

    pollingTimer = NULL;

    return;
}


ZeroconfConnector::~ZeroconfConnector()
{
    if (!enabled) {
        return;
    }

    LocalBNodeSet::iterator iter;
    cancelAndDelete(pollingTimer);

    avahi_threaded_poll_stop(threadedPoll);
    avahi_service_browser_free(sbMDNS);
    avahi_service_browser_free(sbUDNS);

    if (group)
        avahi_entry_group_free(group);

    avahi_client_free(client);
    avahi_threaded_poll_free(threadedPoll);

    sem_destroy(&nodeSetSem);

    for (iter = newSet.begin(); iter != newSet.end(); iter++) {
        avahi_free(iter->first);
        delete iter->second;
    }
    newSet.clear();

    if (serviceName)
        avahi_free(serviceName);

    if (thisNode)
        delete thisNode;

    return;
}


void ZeroconfConnector::announceService(const NodeHandle &node)
{
    if (node.isUnspecified()) {
        cerr << "unspecified node for service announcement" << endl;
        return;
    }

    if (!(thisNode = new NodeHandle(node))) {
        cerr << "no resource to save local node" << endl;
        return;
    }

    create_services(client, this);

    return;
}

void ZeroconfConnector::revokeService()
{
    /*
     * Reset the avahi entry group to revoke
     * bootstrap service.
     */
    if (group)
        avahi_entry_group_reset(group);

    return;
}


int ZeroconfConnector::insertNode(char *name, BootstrapNodeHandle *node)
{
    if ((!node)) {
        cerr << "Trying to insert invalid node" << endl;
        return -1;
    }

    cerr << "insertNode called" << endl;

    sem_wait(&nodeSetSem);
    newSet.insert(LocalNodePair(name, node));

    sem_post(&nodeSetSem);

    return 0;
}


int ZeroconfConnector::removeNode(char *name)
{
    LocalBNodeSet::iterator iter;
    if (!name) {
        cerr << "node name invalid" << endl;
        return -1;
    }

    sem_wait(&nodeSetSem);

    iter = newSet.find((char *)name);
    if (iter != newSet.end()) {
        delete iter->second;
        avahi_free(iter->first);
        newSet.erase(iter);
    }

    sem_post(&nodeSetSem);
    return 0;
}


inline int ZeroconfConnector::getInitResult()
{
    return initResult;
}


void ZeroconfConnector::initialize()
{
    int error;

    enabled = par("enableZeroconf");
    serviceType = par("serviceType");
    serviceName = avahi_strdup(par("serviceName"));
    overlayType = par("overlayType");
    overlayName = par("overlayName");

    if (!enabled) {
        return;
    }

    if (!(threadedPoll = avahi_threaded_poll_new())) {
        cerr << "Failed to create a threaded poll." << endl;
        goto fail;
    }


    if (!(client = avahi_client_new(avahi_threaded_poll_get(threadedPoll), (AvahiClientFlags)0, client_callback,
        this, &error))) {
        cerr << "Failed to create a client." << endl;
        goto fail;
    }

    /* using AVAHI_PROTO_INET means we only need IPv4 informaion */
    if (!(sbMDNS = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_INET, serviceType, NULL,
        (AvahiLookupFlags)0, browse_callback, this))) {
        cerr << "Failed to create a service browser." << endl;
        goto fail;
    }

    if (!(sbUDNS = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_INET, serviceType, overlayName,
        (AvahiLookupFlags)0, browse_callback, this))) {
        cerr << "Failed to create a service browser." << endl;
        goto fail;
    }

    if (avahi_threaded_poll_start(threadedPoll) < 0) {
        cerr << "Failed to start the threaded poll." << endl;
        goto fail;
    }

    pollingTimer = new cMessage("Zeroconf timer");

    scheduleAt(simTime() + 1, pollingTimer);
    initResult = AVAHI_INIT_SUCCEEDED;

    return;

fail:
    if (sbMDNS)
        avahi_service_browser_free(sbMDNS);

    if (sbUDNS)
        avahi_service_browser_free(sbUDNS);

    if (client)
        avahi_client_free(client);

    if (threadedPoll)
        avahi_threaded_poll_free(threadedPoll);

    if (pollingTimer)
        delete pollingTimer;

    return;
}

void ZeroconfConnector::handleMessage(cMessage *msg)
{
    char *name;
    BootstrapNodeHandle *node;
    LocalBNodeSet::iterator tempIter;

    BootstrapList *bootstrapList =
        check_and_cast<BootstrapList *>(getParentModule()->getSubmodule("singleHost", 0)->getSubmodule("bootstrapList", 0));

    if (msg->isSelfMessage()) {
        if (!sem_trywait(&nodeSetSem)) {
            if (!newSet.empty()) {
                for (LocalBNodeSet::iterator iter = newSet.begin(); iter != newSet.end();) {
                    name = iter->first;
                    node = iter->second;
                    tempIter = iter;
                    iter++;

                    /*
                     * Insert bootstrap node into bootstrapList. After
                     * insertion, BootstrapList is responsible for deleting
                     * these nodes
                     */
                    bootstrapList->insertBootstrapCandidate(*node);
                    newSet.erase(tempIter);
                    avahi_free(name);
                }

                newSet.clear();
            }

            sem_post(&nodeSetSem);
        }
    }

    scheduleAt(simTime() + 5, msg);

    return;
}

#endif
