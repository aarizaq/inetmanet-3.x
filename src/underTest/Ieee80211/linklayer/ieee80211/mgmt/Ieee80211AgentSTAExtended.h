//

#ifndef IEEE80211_AGENT_STA_EXTENDED_H
#define IEEE80211_AGENT_STA_EXTENDED_H

#include <vector>
#include <omnetpp.h>
#include "Ieee80211Primitives_m.h"
#include "NotificationBoard.h"
#include "IInterfaceTable.h"


/**
 * Used in 802.11 infrastructure mode: in a station (STA), this module
 * controls channel scanning, association and handovers, by sending commands
 * (e.g. Ieee80211Prim_ScanRequest) to the management getModule(Ieee80211MgmtSTA).
 *
 * See corresponding NED file for a detailed description.
 *
 * @author Andras Varga
 *
 * Modified by Juan-Carlos Maureira. INRIA 2009
 * - correct the dissasociation and back to scanning behavior
 * - add a fixed ssid to connect with.
 * - Agent status logging vector.
 * - Agent starting time random. (avoid all the STAs starts to scan at the same time)
 * - Add Agent Interface to allow polymorphism and dynamic agent's module loading
 */
class INET_API Ieee80211AgentSTAExtended : public cSimpleModule, public INotifiable
{
  protected:
    bool activeScan;
    std::vector<int> channelsToScan;
    simtime_t probeDelay;
    simtime_t minChannelTime;
    simtime_t maxChannelTime;
    simtime_t authenticationTimeout;
    simtime_t associationTimeout;
    // JcM add: agent starting time
    simtime_t startingTime;

    // JcM add: default ssid to connect with.
    std::string default_ssid;
    InterfaceEntry * myEntry;

  protected:
    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    /** Overridden cSimpleModule method */
    virtual void handleMessage(cMessage *msg);

    /** Handle timers */
    virtual void handleTimer(cMessage *msg);

    /** Handle responses from mgmgt */
    virtual void handleResponse(cMessage *msg);

    /** Redefined from INotifiable; called by NotificationBoard */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    // utility method: attaches object to a message as controlInfo, and sends it to mgmt
    virtual void sendRequest(Ieee80211PrimRequest *req);

    /** Sending of Request primitives */
    //@{
    virtual void sendScanRequest();
    virtual void sendAuthenticateRequest(const MACAddress& address);
    virtual void sendDeauthenticateRequest(const MACAddress& address, int reasonCode);
    virtual void sendAssociateRequest(const MACAddress& address);
    virtual void sendReassociateRequest(const MACAddress& address);
    virtual void sendDisassociateRequest(const MACAddress& address, int reasonCode);
    //@}

    /** Processing Confirm primitives */
    //@{
    virtual void processScanConfirm(Ieee80211Prim_ScanConfirm *resp);
    virtual void processAuthenticateConfirm(Ieee80211Prim_AuthenticateConfirm *resp);
    virtual void processAssociateConfirm(Ieee80211Prim_AssociateConfirm *resp);
    virtual void processReassociateConfirm(Ieee80211Prim_ReassociateConfirm *resp);
    //@}

    /** Choose one AP from the list to associate with */
    virtual int chooseBSS(Ieee80211Prim_ScanConfirm *resp);

    // utility method, for debugging
    virtual void dumpAPList(Ieee80211Prim_ScanConfirm *resp);
};

#endif


