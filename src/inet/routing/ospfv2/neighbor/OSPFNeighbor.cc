//
// Copyright (C) 2006 Andras Babos and Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <memory.h>

#include "inet/routing/ospfv2/neighbor/OSPFNeighbor.h"

#include "inet/networklayer/ipv4/IPv4Datagram_m.h"
#include "inet/routing/ospfv2/messagehandler/MessageHandler.h"
#include "inet/routing/ospfv2/router/OSPFArea.h"
#include "inet/routing/ospfv2/neighbor/OSPFNeighborState.h"
#include "inet/routing/ospfv2/neighbor/OSPFNeighborStateDown.h"
#include "inet/routing/ospfv2/router/OSPFRouter.h"

namespace inet {

namespace ospf {

// FIXME!!! Should come from a global unique number generator module.
unsigned long Neighbor::ddSequenceNumberInitSeed = 0;

Neighbor::Neighbor(RouterID neighbor) :
    neighborID(neighbor),
    neighborIPAddress(NULL_IPV4ADDRESS),
    neighborsDesignatedRouter(NULL_DESIGNATEDROUTERID),
    neighborsBackupDesignatedRouter(NULL_DESIGNATEDROUTERID),
    neighborsRouterDeadInterval(40)
{
    memset(&lastReceivedDDPacket, 0, sizeof(Neighbor::DDPacketID));
    // setting only I and M bits is invalid -> good initializer
    lastReceivedDDPacket.ddOptions.I_Init = true;
    lastReceivedDDPacket.ddOptions.M_More = true;
    inactivityTimer = new cMessage();
    inactivityTimer->setKind(NEIGHBOR_INACTIVITY_TIMER);
    inactivityTimer->setContextPointer(this);
    inactivityTimer->setName("Neighbor::NeighborInactivityTimer");
    pollTimer = new cMessage();
    pollTimer->setKind(NEIGHBOR_POLL_TIMER);
    pollTimer->setContextPointer(this);
    pollTimer->setName("Neighbor::NeighborPollTimer");
    ddRetransmissionTimer = new cMessage();
    ddRetransmissionTimer->setKind(NEIGHBOR_DD_RETRANSMISSION_TIMER);
    ddRetransmissionTimer->setContextPointer(this);
    ddRetransmissionTimer->setName("Neighbor::NeighborDDRetransmissionTimer");
    updateRetransmissionTimer = new cMessage();
    updateRetransmissionTimer->setKind(NEIGHBOR_UPDATE_RETRANSMISSION_TIMER);
    updateRetransmissionTimer->setContextPointer(this);
    updateRetransmissionTimer->setName("Neighbor::Neighbor::NeighborUpdateRetransmissionTimer");
    requestRetransmissionTimer = new cMessage();
    requestRetransmissionTimer->setKind(NEIGHBOR_REQUEST_RETRANSMISSION_TIMER);
    requestRetransmissionTimer->setContextPointer(this);
    requestRetransmissionTimer->setName("Neighbor::NeighborRequestRetransmissionTimer");
    state = new NeighborStateDown;
    previousState = nullptr;
}

Neighbor::~Neighbor()
{
    reset();
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->clearTimer(inactivityTimer);
    messageHandler->clearTimer(pollTimer);
    delete inactivityTimer;
    delete pollTimer;
    delete ddRetransmissionTimer;
    delete updateRetransmissionTimer;
    delete requestRetransmissionTimer;
    if (previousState != nullptr) {
        delete previousState;
    }
    delete state;
}

void Neighbor::changeState(NeighborState *newState, NeighborState *currentState)
{
    if (previousState != nullptr) {
        delete previousState;
    }
    state = newState;
    previousState = currentState;
}

void Neighbor::processEvent(Neighbor::NeighborEventType event)
{
    state->processEvent(this, event);
}

void Neighbor::reset()
{
    for (auto & elem : linkStateRetransmissionList)
    {
        delete (elem);
    }
    linkStateRetransmissionList.clear();

    for (auto & elem : databaseSummaryList) {
        delete (elem);
    }
    databaseSummaryList.clear();
    for (auto & elem : linkStateRequestList) {
        delete (elem);
    }
    linkStateRequestList.clear();

    parentInterface->getArea()->getRouter()->getMessageHandler()->clearTimer(ddRetransmissionTimer);
    clearUpdateRetransmissionTimer();
    clearRequestRetransmissionTimer();

    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}

void Neighbor::initFirstAdjacency()
{
    ddSequenceNumber = getUniqueULong();
    firstAdjacencyInited = true;
}

unsigned long Neighbor::getUniqueULong()
{
    // FIXME!!! Should come from a global unique number generator module.
    return ddSequenceNumberInitSeed++;
}

Neighbor::NeighborStateType Neighbor::getState() const
{
    return state->getState();
}

const char *Neighbor::getStateString(Neighbor::NeighborStateType stateType)
{
    switch (stateType) {
        case DOWN_STATE:
            return "Down";

        case ATTEMPT_STATE:
            return "Attempt";

        case INIT_STATE:
            return "Init";

        case TWOWAY_STATE:
            return "TwoWay";

        case EXCHANGE_START_STATE:
            return "ExchangeStart";

        case EXCHANGE_STATE:
            return "Exchange";

        case LOADING_STATE:
            return "Loading";

        case FULL_STATE:
            return "Full";

        default:
            ASSERT(false);
            break;
    }
    return "";
}

void Neighbor::sendDatabaseDescriptionPacket(bool init)
{
    OSPFDatabaseDescriptionPacket *ddPacket = new OSPFDatabaseDescriptionPacket();

    ddPacket->setType(DATABASE_DESCRIPTION_PACKET);
    ddPacket->setRouterID(IPv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    ddPacket->setAreaID(IPv4Address(parentInterface->getArea()->getAreaID()));
    ddPacket->setAuthenticationType(parentInterface->getAuthenticationType());
    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        ddPacket->setAuthentication(i, authKey.bytes[i]);
    }

    if (parentInterface->getType() != Interface::VIRTUAL) {
        ddPacket->setInterfaceMTU(parentInterface->getMTU());
    }
    else {
        ddPacket->setInterfaceMTU(0);
    }

    OSPFOptions options;
    memset(&options, 0, sizeof(OSPFOptions));
    options.E_ExternalRoutingCapability = parentInterface->getArea()->getExternalRoutingCapability();
    ddPacket->setOptions(options);

    ddPacket->setDdSequenceNumber(ddSequenceNumber);

    long maxPacketSize = (((IP_MAX_HEADER_BYTES + OSPF_HEADER_LENGTH + OSPF_DD_HEADER_LENGTH + OSPF_LSA_HEADER_LENGTH) > parentInterface->getMTU()) ?
                          IPV4_DATAGRAM_LENGTH :
                          parentInterface->getMTU()) - IP_MAX_HEADER_BYTES;
    long packetSize = OSPF_HEADER_LENGTH + OSPF_DD_HEADER_LENGTH;

    if (init || databaseSummaryList.empty()) {
        ddPacket->setLsaHeadersArraySize(0);
    }
    else {
        // delete included LSAs from summary list
        // (they are still in lastTransmittedDDPacket)
        while ((!databaseSummaryList.empty()) && (packetSize <= (maxPacketSize - OSPF_LSA_HEADER_LENGTH))) {
            unsigned long headerCount = ddPacket->getLsaHeadersArraySize();
            OSPFLSAHeader *lsaHeader = *(databaseSummaryList.begin());
            ddPacket->setLsaHeadersArraySize(headerCount + 1);
            ddPacket->setLsaHeaders(headerCount, *lsaHeader);
            delete lsaHeader;
            databaseSummaryList.pop_front();
            packetSize += OSPF_LSA_HEADER_LENGTH;
        }
    }

    OSPFDDOptions ddOptions;
    memset(&ddOptions, 0, sizeof(OSPFDDOptions));
    if (init) {
        ddOptions.I_Init = true;
        ddOptions.M_More = true;
        ddOptions.MS_MasterSlave = true;
    }
    else {
        ddOptions.I_Init = false;
        ddOptions.M_More = (databaseSummaryList.empty()) ? false : true;
        ddOptions.MS_MasterSlave = (databaseExchangeRelationship == Neighbor::MASTER) ? true : false;
    }
    ddPacket->setDdOptions(ddOptions);

    ddPacket->setByteLength(packetSize);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

    if (lastTransmittedDDPacket != nullptr)
        delete lastTransmittedDDPacket;
    lastTransmittedDDPacket = ddPacket->dup();

    if (parentInterface->getType() == Interface::POINTTOPOINT) {
        messageHandler->sendPacket(ddPacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface->getIfIndex(), ttl);
    }
    else {
        messageHandler->sendPacket(ddPacket, neighborIPAddress, parentInterface->getIfIndex(), ttl);
    }
}

bool Neighbor::retransmitDatabaseDescriptionPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        OSPFDatabaseDescriptionPacket *ddPacket = new OSPFDatabaseDescriptionPacket(*lastTransmittedDDPacket);
        MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
        int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;

        if (parentInterface->getType() == Interface::POINTTOPOINT) {
            messageHandler->sendPacket(ddPacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface->getIfIndex(), ttl);
        }
        else {
            messageHandler->sendPacket(ddPacket, neighborIPAddress, parentInterface->getIfIndex(), ttl);
        }

        return true;
    }
    else {
        return false;
    }
}

void Neighbor::createDatabaseSummary()
{
    Area *area = parentInterface->getArea();
    unsigned long routerLSACount = area->getRouterLSACount();

    /* Note: OSPF specification says:
     * "LSAs whose age is equal to MaxAge are instead added to the neighbor's
     *  Link state retransmission list."
     * But this task has been already done during the aging of the database. (???)
     * So we'll skip this.
     */
    for (unsigned long i = 0; i < routerLSACount; i++) {
        if (area->getRouterLSA(i)->getHeader().getLsAge() < MAX_AGE) {
            OSPFLSAHeader *routerLSA = new OSPFLSAHeader(area->getRouterLSA(i)->getHeader());
            databaseSummaryList.push_back(routerLSA);
        }
    }

    unsigned long networkLSACount = area->getNetworkLSACount();
    for (unsigned long j = 0; j < networkLSACount; j++) {
        if (area->getNetworkLSA(j)->getHeader().getLsAge() < MAX_AGE) {
            OSPFLSAHeader *networkLSA = new OSPFLSAHeader(area->getNetworkLSA(j)->getHeader());
            databaseSummaryList.push_back(networkLSA);
        }
    }

    unsigned long summaryLSACount = area->getSummaryLSACount();
    for (unsigned long k = 0; k < summaryLSACount; k++) {
        if (area->getSummaryLSA(k)->getHeader().getLsAge() < MAX_AGE) {
            OSPFLSAHeader *summaryLSA = new OSPFLSAHeader(area->getSummaryLSA(k)->getHeader());
            databaseSummaryList.push_back(summaryLSA);
        }
    }

    if ((parentInterface->getType() != Interface::VIRTUAL) &&
        (area->getExternalRoutingCapability()))
    {
        Router *router = area->getRouter();
        unsigned long asExternalLSACount = router->getASExternalLSACount();

        for (unsigned long m = 0; m < asExternalLSACount; m++) {
            if (router->getASExternalLSA(m)->getHeader().getLsAge() < MAX_AGE) {
                OSPFLSAHeader *asExternalLSA = new OSPFLSAHeader(router->getASExternalLSA(m)->getHeader());
                databaseSummaryList.push_back(asExternalLSA);
            }
        }
    }
}

void Neighbor::sendLinkStateRequestPacket()
{
    OSPFLinkStateRequestPacket *requestPacket = new OSPFLinkStateRequestPacket();

    requestPacket->setType(LINKSTATE_REQUEST_PACKET);
    requestPacket->setRouterID(IPv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    requestPacket->setAreaID(IPv4Address(parentInterface->getArea()->getAreaID()));
    requestPacket->setAuthenticationType(parentInterface->getAuthenticationType());
    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        requestPacket->setAuthentication(i, authKey.bytes[i]);
    }

    long maxPacketSize = (((IP_MAX_HEADER_BYTES + OSPF_HEADER_LENGTH + OSPF_REQUEST_LENGTH) > parentInterface->getMTU()) ?
                          IPV4_DATAGRAM_LENGTH :
                          parentInterface->getMTU()) - IP_MAX_HEADER_BYTES;
    long packetSize = OSPF_HEADER_LENGTH;

    if (linkStateRequestList.empty()) {
        requestPacket->setRequestsArraySize(0);
    }
    else {
        auto it = linkStateRequestList.begin();

        while ((it != linkStateRequestList.end()) && (packetSize <= (maxPacketSize - OSPF_REQUEST_LENGTH))) {
            unsigned long requestCount = requestPacket->getRequestsArraySize();
            OSPFLSAHeader *requestHeader = (*it);
            LSARequest request;

            request.lsType = requestHeader->getLsType();
            request.linkStateID = requestHeader->getLinkStateID();
            request.advertisingRouter = requestHeader->getAdvertisingRouter();

            requestPacket->setRequestsArraySize(requestCount + 1);
            requestPacket->setRequests(requestCount, request);

            packetSize += OSPF_REQUEST_LENGTH;
            it++;
        }
    }

    requestPacket->setByteLength(packetSize);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    if (parentInterface->getType() == Interface::POINTTOPOINT) {
        messageHandler->sendPacket(requestPacket, IPv4Address::ALL_OSPF_ROUTERS_MCAST, parentInterface->getIfIndex(), ttl);
    }
    else {
        messageHandler->sendPacket(requestPacket, neighborIPAddress, parentInterface->getIfIndex(), ttl);
    }
}

bool Neighbor::needAdjacency()
{
    Interface::OSPFInterfaceType interfaceType = parentInterface->getType();
    RouterID routerID = parentInterface->getArea()->getRouter()->getRouterID();
    DesignatedRouterID dRouter = parentInterface->getDesignatedRouter();
    DesignatedRouterID backupDRouter = parentInterface->getBackupDesignatedRouter();

    if ((interfaceType == Interface::POINTTOPOINT) ||
        (interfaceType == Interface::POINTTOMULTIPOINT) ||
        (interfaceType == Interface::VIRTUAL) ||
        (dRouter.routerID == routerID) ||
        (backupDRouter.routerID == routerID) ||
        ((neighborsDesignatedRouter.routerID == dRouter.routerID) ||
         (!designatedRoutersSetUp &&
          (neighborsDesignatedRouter.ipInterfaceAddress == dRouter.ipInterfaceAddress))) ||
        ((neighborsBackupDesignatedRouter.routerID == backupDRouter.routerID) ||
         (!designatedRoutersSetUp &&
          (neighborsBackupDesignatedRouter.ipInterfaceAddress == backupDRouter.ipInterfaceAddress))))
    {
        return true;
    }
    else {
        return false;
    }
}

/**
 * If the LSA is already on the retransmission list then it is replaced, else
 * a copy of the LSA is added to the end of the retransmission list.
 * @param lsa [in] The LSA to be added.
 */
void Neighbor::addToRetransmissionList(const OSPFLSA *lsa)
{
    auto it = linkStateRetransmissionList.begin();
    for ( ; it != linkStateRetransmissionList.end(); it++) {
        if (((*it)->getHeader().getLinkStateID() == lsa->getHeader().getLinkStateID()) &&
            ((*it)->getHeader().getAdvertisingRouter().getInt() == lsa->getHeader().getAdvertisingRouter().getInt()))
        {
            break;
        }
    }

    OSPFLSA *lsaCopy = nullptr;
    switch (lsa->getHeader().getLsType()) {
        case ROUTERLSA_TYPE:
            lsaCopy = new OSPFRouterLSA(*(check_and_cast<const OSPFRouterLSA *>(lsa)));
            break;

        case NETWORKLSA_TYPE:
            lsaCopy = new OSPFNetworkLSA(*(check_and_cast<const OSPFNetworkLSA *>(lsa)));
            break;

        case SUMMARYLSA_NETWORKS_TYPE:
        case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
            lsaCopy = new OSPFSummaryLSA(*(check_and_cast<const OSPFSummaryLSA *>(lsa)));
            break;

        case AS_EXTERNAL_LSA_TYPE:
            lsaCopy = new OSPFASExternalLSA(*(check_and_cast<const OSPFASExternalLSA *>(lsa)));
            break;

        default:
            ASSERT(false);    // error
            break;
    }

    if (it != linkStateRetransmissionList.end()) {
        delete (*it);
        *it = static_cast<OSPFLSA *>(lsaCopy);
    }
    else {
        linkStateRetransmissionList.push_back(static_cast<OSPFLSA *>(lsaCopy));
    }
}

void Neighbor::removeFromRetransmissionList(LSAKeyType lsaKey)
{
    auto it = linkStateRetransmissionList.begin();
    while (it != linkStateRetransmissionList.end()) {
        if (((*it)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRetransmissionList.erase(it);
        }
        else {
            it++;
        }
    }
}

bool Neighbor::isLinkStateRequestListEmpty(LSAKeyType lsaKey) const
{
    for (auto lsa : linkStateRetransmissionList) {
        if ((lsa->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            (lsa->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

OSPFLSA *Neighbor::findOnRetransmissionList(LSAKeyType lsaKey)
{
    for (auto & elem : linkStateRetransmissionList) {
        if (((elem)->getHeader().getLinkStateID() == lsaKey.linkStateID) &&
            ((elem)->getHeader().getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return elem;
        }
    }
    return nullptr;
}

void Neighbor::startUpdateRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->startTimer(updateRetransmissionTimer, parentInterface->getRetransmissionInterval());
    updateRetransmissionTimerActive = true;
}

void Neighbor::clearUpdateRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->clearTimer(updateRetransmissionTimer);
    updateRetransmissionTimerActive = false;
}

void Neighbor::addToRequestList(const OSPFLSAHeader *lsaHeader)
{
    linkStateRequestList.push_back(new OSPFLSAHeader(*lsaHeader));
}

void Neighbor::removeFromRequestList(LSAKeyType lsaKey)
{
    auto it = linkStateRequestList.begin();
    while (it != linkStateRequestList.end()) {
        if (((*it)->getLinkStateID() == lsaKey.linkStateID) &&
            ((*it)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            delete (*it);
            it = linkStateRequestList.erase(it);
        }
        else {
            it++;
        }
    }

    if ((getState() == Neighbor::LOADING_STATE) && (linkStateRequestList.empty())) {
        clearRequestRetransmissionTimer();
        processEvent(Neighbor::LOADING_DONE);
    }
}

bool Neighbor::isLSAOnRequestList(LSAKeyType lsaKey) const
{
    for (auto lsaHeader : linkStateRequestList) {
        if ((lsaHeader->getLinkStateID() == lsaKey.linkStateID) &&
            (lsaHeader->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

OSPFLSAHeader *Neighbor::findOnRequestList(LSAKeyType lsaKey)
{
    for (auto & elem : linkStateRequestList) {
        if (((elem)->getLinkStateID() == lsaKey.linkStateID) &&
            ((elem)->getAdvertisingRouter() == lsaKey.advertisingRouter))
        {
            return elem;
        }
    }
    return nullptr;
}

void Neighbor::startRequestRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->startTimer(requestRetransmissionTimer, parentInterface->getRetransmissionInterval());
    requestRetransmissionTimerActive = true;
}

void Neighbor::clearRequestRetransmissionTimer()
{
    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    messageHandler->clearTimer(requestRetransmissionTimer);
    requestRetransmissionTimerActive = false;
}

void Neighbor::addToTransmittedLSAList(LSAKeyType lsaKey)
{
    TransmittedLSA transmit;

    transmit.lsaKey = lsaKey;
    transmit.age = 0;

    transmittedLSAs.push_back(transmit);
}

bool Neighbor::isOnTransmittedLSAList(LSAKeyType lsaKey) const
{
    for (const auto & elem : transmittedLSAs) {
        if ((elem.lsaKey.linkStateID == lsaKey.linkStateID) &&
            (elem.lsaKey.advertisingRouter == lsaKey.advertisingRouter))
        {
            return true;
        }
    }
    return false;
}

void Neighbor::ageTransmittedLSAList()
{
    auto it = transmittedLSAs.begin();
    while ((it != transmittedLSAs.end()) && (it->age == MIN_LS_ARRIVAL)) {
        transmittedLSAs.pop_front();
        it = transmittedLSAs.begin();
    }
    for (it = transmittedLSAs.begin(); it != transmittedLSAs.end(); it++) {
        it->age++;
    }
}

void Neighbor::retransmitUpdatePacket()
{
    OSPFLinkStateUpdatePacket *updatePacket = new OSPFLinkStateUpdatePacket();

    updatePacket->setType(LINKSTATE_UPDATE_PACKET);
    updatePacket->setRouterID(IPv4Address(parentInterface->getArea()->getRouter()->getRouterID()));
    updatePacket->setAreaID(IPv4Address(parentInterface->getArea()->getAreaID()));
    updatePacket->setAuthenticationType(parentInterface->getAuthenticationType());
    AuthenticationKeyType authKey = parentInterface->getAuthenticationKey();
    for (int i = 0; i < 8; i++) {
        updatePacket->setAuthentication(i, authKey.bytes[i]);
    }

    bool packetFull = false;
    unsigned short lsaCount = 0;
    unsigned long packetLength = IP_MAX_HEADER_BYTES + OSPF_LSA_HEADER_LENGTH;
    auto it = linkStateRetransmissionList.begin();

    while (!packetFull && (it != linkStateRetransmissionList.end())) {
        LSAType lsaType = static_cast<LSAType>((*it)->getHeader().getLsType());
        OSPFRouterLSA *routerLSA = (lsaType == ROUTERLSA_TYPE) ? dynamic_cast<OSPFRouterLSA *>(*it) : nullptr;
        OSPFNetworkLSA *networkLSA = (lsaType == NETWORKLSA_TYPE) ? dynamic_cast<OSPFNetworkLSA *>(*it) : nullptr;
        OSPFSummaryLSA *summaryLSA = ((lsaType == SUMMARYLSA_NETWORKS_TYPE) ||
                                      (lsaType == SUMMARYLSA_ASBOUNDARYROUTERS_TYPE)) ? dynamic_cast<OSPFSummaryLSA *>(*it) : nullptr;
        OSPFASExternalLSA *asExternalLSA = (lsaType == AS_EXTERNAL_LSA_TYPE) ? dynamic_cast<OSPFASExternalLSA *>(*it) : nullptr;
        long lsaSize = 0;
        bool includeLSA = false;

        switch (lsaType) {
            case ROUTERLSA_TYPE:
                if (routerLSA != nullptr) {
                    lsaSize = calculateLSASize(routerLSA);
                }
                break;

            case NETWORKLSA_TYPE:
                if (networkLSA != nullptr) {
                    lsaSize = calculateLSASize(networkLSA);
                }
                break;

            case SUMMARYLSA_NETWORKS_TYPE:
            case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
                if (summaryLSA != nullptr) {
                    lsaSize = calculateLSASize(summaryLSA);
                }
                break;

            case AS_EXTERNAL_LSA_TYPE:
                if (asExternalLSA != nullptr) {
                    lsaSize = calculateLSASize(asExternalLSA);
                }
                break;

            default:
                break;
        }

        if (packetLength + lsaSize < parentInterface->getMTU()) {
            includeLSA = true;
            lsaCount++;
        }
        else {
            if ((lsaCount == 0) && (packetLength + lsaSize < IPV4_DATAGRAM_LENGTH)) {
                includeLSA = true;
                lsaCount++;
                packetFull = true;
            }
        }

        if (includeLSA) {
            packetLength += lsaSize;
            switch (lsaType) {
                case ROUTERLSA_TYPE:
                    if (routerLSA != nullptr) {
                        unsigned int routerLSACount = updatePacket->getRouterLSAsArraySize();

                        updatePacket->setRouterLSAsArraySize(routerLSACount + 1);
                        updatePacket->setRouterLSAs(routerLSACount, *routerLSA);

                        unsigned short lsAge = updatePacket->getRouterLSAs(routerLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getRouterLSAsForUpdate(routerLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getRouterLSAsForUpdate(routerLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                case NETWORKLSA_TYPE:
                    if (networkLSA != nullptr) {
                        unsigned int networkLSACount = updatePacket->getNetworkLSAsArraySize();

                        updatePacket->setNetworkLSAsArraySize(networkLSACount + 1);
                        updatePacket->setNetworkLSAs(networkLSACount, *networkLSA);

                        unsigned short lsAge = updatePacket->getNetworkLSAs(networkLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getNetworkLSAsForUpdate(networkLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getNetworkLSAsForUpdate(networkLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                case SUMMARYLSA_NETWORKS_TYPE:
                case SUMMARYLSA_ASBOUNDARYROUTERS_TYPE:
                    if (summaryLSA != nullptr) {
                        unsigned int summaryLSACount = updatePacket->getSummaryLSAsArraySize();

                        updatePacket->setSummaryLSAsArraySize(summaryLSACount + 1);
                        updatePacket->setSummaryLSAs(summaryLSACount, *summaryLSA);

                        unsigned short lsAge = updatePacket->getSummaryLSAs(summaryLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getSummaryLSAsForUpdate(summaryLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getSummaryLSAsForUpdate(summaryLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                case AS_EXTERNAL_LSA_TYPE:
                    if (asExternalLSA != nullptr) {
                        unsigned int asExternalLSACount = updatePacket->getAsExternalLSAsArraySize();

                        updatePacket->setAsExternalLSAsArraySize(asExternalLSACount + 1);
                        updatePacket->setAsExternalLSAs(asExternalLSACount, *asExternalLSA);

                        unsigned short lsAge = updatePacket->getAsExternalLSAs(asExternalLSACount).getHeader().getLsAge();
                        if (lsAge < MAX_AGE - parentInterface->getTransmissionDelay()) {
                            updatePacket->getAsExternalLSAsForUpdate(asExternalLSACount).getHeaderForUpdate().setLsAge(lsAge + parentInterface->getTransmissionDelay());
                        }
                        else {
                            updatePacket->getAsExternalLSAsForUpdate(asExternalLSACount).getHeaderForUpdate().setLsAge(MAX_AGE);
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        it++;
    }

    updatePacket->setByteLength(packetLength - IP_MAX_HEADER_BYTES);

    MessageHandler *messageHandler = parentInterface->getArea()->getRouter()->getMessageHandler();
    int ttl = (parentInterface->getType() == Interface::VIRTUAL) ? VIRTUAL_LINK_TTL : 1;
    messageHandler->sendPacket(updatePacket, neighborIPAddress, parentInterface->getIfIndex(), ttl);
}

void Neighbor::deleteLastSentDDPacket()
{
    if (lastTransmittedDDPacket != nullptr) {
        delete lastTransmittedDDPacket;
        lastTransmittedDDPacket = nullptr;
    }
}

} // namespace ospf

} // namespace inet

