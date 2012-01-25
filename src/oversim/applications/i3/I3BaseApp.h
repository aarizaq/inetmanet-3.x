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
 * @file I3BaseApp.h
 * @author Antonio Zea
 */


#ifndef __I3BASEAPP_H__
#define __I3BASEAPP_H__


#include <omnetpp.h>
#include "INETDefs.h"
#include "INotifiable.h"
#include "UDPSocket.h"


#include "I3Trigger.h"
#include "I3IdentifierStack.h"
#include "I3Message.h"
#include "I3IPAddress.h"
#include "I3Identifier.h"

/** Basic template class for I3 applications. */
class I3BaseApp : public cSimpleModule, public INotifiable
{
public:
    struct I3CachedServer {
        I3IPAddress address;

        simtime_t lastReply;
        simtime_t roundTripTime;

        I3CachedServer();
        friend std::ostream& operator<<(std::ostream& os, const I3CachedServer& ip);
    };

    /** Constructor */
    I3BaseApp();

    /** Destructor */
    ~I3BaseApp();

protected:
    UDPSocket socket;

    enum I3MobilityStage {
        I3_MOBILITY_BEFORE_UPDATE,
        I3_MOBILITY_UPDATED
    };

    /** Number of sent messages */
    int numSent;

    int sentBytes;

    /** Number of received messages */
    int numReceived;

    int receivedBytes;

    /** Number of times this node has been isolated - i.e. without any I3 servers known */
    int numIsolations;

    /** Cached IP address of this node */
    IPvXAddress nodeIPAddress;

    /** Stored I3 triggers sent from this node, to be refreshed automatically */
    std::set<I3Trigger> insertedTriggers;

    std::map<I3Identifier, I3CachedServer> samplingCache;

    std::map<I3Identifier, I3CachedServer> identifierCache;

    I3CachedServer gateway;

    cMessage *refreshTriggersTimer;

    int refreshTriggersTime;

    cMessage *refreshSamplesTimer;

    int refreshSamplesTime;

    cMessage *initializeTimer;

    cMessage *bootstrapTimer;

    /** Returns number of init stages required */
    int numInitStages() const;

    /** Basic initialization */
    void initialize(int stage);

    /** App initialization - should be overwritten by application. I3 related commands should go in initializeI3.
         * @param stage Initialization stage passed from initialize()
        */
    virtual void initializeApp(int stage);

    /** Internal I3 bootstrap - connects to I3, inserts sampling triggers and initializes timers. */
    void bootstrapI3();

    /** Application I3 initialize - should be overwritten by application.*/
    virtual void initializeI3();


    /** Handles timers - should be overwritten by application
      * @param msg Timer to be handled
      */
    virtual void handleTimerEvent(cMessage* msg);

    /** Handles messages incoming from UDP gate
      * @param msg Message sent
      */
    virtual void handleUDPMessage(cMessage* msg);

    /** Handles incoming messages
      * @param msg Incoming message
      */
    void handleMessage(cMessage *msg);

    /** Delivers packets coming from I3 - should be overwritten by application
      * @param trigger Application trigger to which the packet was sent
      * @param stack Identifier stack passed from I3
      * @param msg  Arriving message
      */
    virtual void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);

    /** Sends a message to I3
      * @param msg Message to be sent
      */
    void sendToI3(I3Message *msg);

    /** Sends a message through UDP
      * @param msg Message to be sent
      * @param ip IP of destination
      */
    void sendThroughUDP(cMessage *msg, const I3IPAddress &ip);

    /** Refreshes (reinserts) stored triggers */
    void refreshTriggers();

    /** Refreshes sampling triggers and selects fastest server as gateway */
    void refreshSamples();

    I3Identifier retrieveClosestIdentifier();

    /** Routes a packet through I3, passing an identifier stack composed of a single identifier
      * @param id Destination identifier
      * @param msg Message to be sent
      * @param useHint Use address in server cache if existant
      */
    void sendPacket(const I3Identifier &id, cPacket *msg, bool useHint = false);

    /**  Routes a packet through I3
      * @param stack Destination identifier stack
      * @param msg Message to be sent
      * @param useHint Use address in server cache if existant
      */
    void sendPacket(const I3IdentifierStack &stack, cPacket *msg, bool useHint = false);

    /** Inserts a trigger into I3, composed by the given identifier and an identifier stack
      * containing only this node's IP address
      * @param identifier Trigger's identifier
      * @param store Sets whether to store the trigger for auto-refresh
      */
    void insertTrigger(const I3Identifier &identifier, bool store = true);

    /** Inserts a trigger into I3 with the given identifier and identifier stack
      * @param identifier Trigger's identifier
      * @param stack Trigger's identifier stack
      * @param store Sets whether to store the trigger for auto-refresh
      */
    void insertTrigger(const I3Identifier &identifier, const I3IdentifierStack &stack, bool store = true);

    /** Inserts the given trigger into I3
      * @param t Trigger to be inserted
      * @param store Sets whether to store the trigger for auto-refresh
      */
    void insertTrigger(const I3Trigger &t, bool store = true);

    /** Removes all triggers from the list of inserted triggers whose identifiers equal the one given
      * @param identifier Identifier to be compared against
      */
    void removeTrigger(const I3Identifier &identifier);

    /** Removes a trigger from I3
     * @param trigger Trigger to be removed
     */
    void removeTrigger(const I3Trigger &trigger);

    /** Returns the list of inserted triggers */
    std::set<I3Trigger> &getInsertedTriggers();


    virtual void receiveChangeNotification (int category, const cObject *details);

    virtual void doMobilityEvent(I3MobilityStage stage);

private:
    bool mobilityInStages;


};

#endif
