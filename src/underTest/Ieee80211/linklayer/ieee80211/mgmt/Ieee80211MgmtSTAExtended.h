#ifndef IEEE80211_MGMT_STA_EXTENDED_H
#define IEEE80211_MGMT_STA_EXTENDED_H

#include <omnetpp.h>
#include "Ieee80211MgmtBase.h"
#include "NotificationBoard.h"
#include "Ieee80211Primitives_m.h"
#include "Radio80211aControlInfo_m.h"
#include "ChannelAccess.h"
#include "IInterfaceTable.h"

// Notification class of Updated info in the associated AP
#define NF_L2_ASSOCIATED_AP_UPDATE     100        // when the associated AP info is updated (currently Ieee80211)

class INET_API Ieee80211MgmtSTAExtended : public Ieee80211MgmtBase
{
private:
	int  getNumChannels()
	{
	    IChannelControl *cc = dynamic_cast<IChannelControl *>(simulation.getModuleByPath("channelControl"));
	    if (!cc)
	        cc = dynamic_cast<IChannelControl *>(simulation.getModuleByPath("channelcontrol"));
	    if (!cc)
	        throw cRuntimeError("Could not find ChannelControl module with name 'channelControl' in the toplevel network.");
	    return cc->getNumChannels();
	}

  public:
    //
    // Encapsulates information about the ongoing scanning process
    //
    struct ScanningInfo
    {
        MACAddress bssid; // specific BSSID to scan for, or the broadcast address
        std::string ssid; // SSID to scan for (empty=any)
        bool activeScan;  // whether to perform active or passive scanning
        simtime_t probeDelay; // delay (in s) to be used prior to transmitting a Probe frame during active scanning
        std::vector<int> channelList; // list of channels to scan
        int currentChannelIndex; // index into channelList[]
        bool busyChannelDetected; // during minChannelTime, we have to listen for busy channel
        simtime_t minChannelTime; // minimum time to spend on each channel when scanning
        simtime_t maxChannelTime; // maximum time to spend on each channel when scanning
    };

    //
    // Stores AP info received during scanning
    //
    class APInfo : public cPolymorphic
    {
      public:
        int channel;
        MACAddress address; // alias bssid
        std::string ssid;
        Ieee80211SupportedRatesElement supportedRates;
        simtime_t beaconInterval;
        double rxPower;

        bool isAuthenticated;
        int authSeqExpected;  // valid while authenticating; values: 1,3,5...
        cMessage *authTimeoutMsg; // if non-NULL: authentication is in progress

        APInfo()
        {
            channel=-1; beaconInterval=rxPower=0; authSeqExpected=-1;
            isAuthenticated=false; authTimeoutMsg=NULL;
        }

        APInfo(const APInfo& ap_info)
        {
            this->channel = ap_info.channel;
            this->address = ap_info.address;
            this->ssid = ap_info.ssid;
            this->supportedRates = ap_info.supportedRates;
            this->beaconInterval = ap_info.beaconInterval;
            this->rxPower = ap_info.rxPower;
            this->isAuthenticated = ap_info.isAuthenticated;
            this->authSeqExpected = ap_info.authSeqExpected;
            this->authTimeoutMsg = NULL;
        }
    };

    //
    // Associated AP, plus data associated with the association with the associated AP
    //
    class AssociatedAPInfo : public APInfo
    {
      public:
        int receiveSequence;
        bool isAssociated;
        cMessage *beaconTimeoutMsg;
        cMessage *assocTimeoutMsg; // if non-NULL: association is in progress

        AssociatedAPInfo() : APInfo() {receiveSequence=0; isAssociated = false; beaconTimeoutMsg=NULL; assocTimeoutMsg=NULL;}
        AssociatedAPInfo(const AssociatedAPInfo& assoc_ap) : APInfo(assoc_ap) {receiveSequence=0; beaconTimeoutMsg=NULL;}

        friend std::ostream& operator<<(std::ostream& os, const Ieee80211MgmtSTAExtended::AssociatedAPInfo& assocAP)
        {
            os << "AP addr=" << assocAP.address
            << " chan=" << assocAP.channel
            << " ssid=" << assocAP.ssid
            << " beaconIntvl=" << assocAP.beaconInterval
            << " receiveSeq="  << assocAP.receiveSequence
            << " rxPower=" << assocAP.rxPower;
            return os;
        }
    };

    // Connectivity states
    enum MGMT_STATES {NOT_ASSOCIATED,SCANNING,AUTHENTICATING,ASSOCIATING,ASSOCIATED};
    cOutVector connStates;
    MGMT_STATES mgmt_state;

    // Paula Uribe: new vector for record the beacons arrival and mgmt queue length
    cOutVector mgmtBeaconsArrival;
    cOutVector mgmtQueueLenVec;
    cOutVector rcvdPowerVectormW;
    cOutVector rcvdPowerVectordB;

    InterfaceEntry * myEntry;

  protected:
    // Paula Uribe: add radio reference
    cModule* hostName;

    NotificationBoard *nb;

    // number of channels in ChannelControl -- used if we're told to scan "all" channels
    int numChannels;

    // scanning status
    bool isScanning;
    ScanningInfo scanning;

    // APInfo list: we collect scanning results and keep track of ongoing authentications here
    // Note: there can be several ongoing authentications simultaneously
    typedef std::list<APInfo> AccessPointList;
    AccessPointList apList;

    // associated Access Point
    bool isAssociated;         // is the mgmtModule is associated
    AssociatedAPInfo assocAP;

    double max_beacons_missed; // number of max beacon missed to notify the agent about beacon_lost event

  protected:

    ~Ieee80211MgmtSTAExtended()
    {

    }

    virtual int numInitStages() const {return 2;}
    virtual void initialize(int);

    virtual void finish();

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleUpperMessage(cPacket *msg);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleCommand(int msgkind, cPolymorphic *ctrl);

    /** Utility function for handleUpperMessage() */
    virtual Ieee80211DataFrame *encapsulate(cPacket *msg);

    /** Utility function: sends authentication request */
    virtual void startAuthentication(APInfo *ap, simtime_t timeout);

    /** Utility function: sends association request */
    virtual void startAssociation(APInfo *ap, simtime_t timeout);

    /** Utility function: looks up AP in our AP list. Returns NULL if not found. */
    virtual APInfo *lookupAP(const MACAddress& address);

    /** Utility function: clear the AP list, and cancel any pending authentications. */
    virtual void clearAPList();

    /** Utility function: switches to the given radio channel. */
    virtual void changeChannel(int channelNum);

    /** Stores AP info received in a beacon or probe response */
    virtual void storeAPInfo(const MACAddress& address, const Ieee80211BeaconFrameBody& body);

    /** Switches to the next channel to scan; returns true if done (there wasn't any more channel to scan). */
    virtual bool scanNextChannel();

    /** Broadcasts a Probe Request */
    virtual void sendProbeRequest();

    /** Missed a few consecutive beacons */
    virtual void beaconLost();

    /** Sends back result of scanning to the agent */
    virtual void sendScanConfirm();

    /** Sends back result of authentication to the agent */
    virtual void sendAuthenticationConfirm(APInfo *ap, int resultCode);

    /** Sends back result of association to the agent */
    virtual void sendAssociationConfirm(APInfo *ap, int resultCode);

    /** Utility function: Cancel the existing association */
    virtual void disassociate();

    /** Utility function: sends a confirmation to the agent */
    virtual void sendConfirm(Ieee80211PrimConfirm *confirm, int resultCode);

    /** Utility function: sends a management frame */
    virtual void sendManagementFrame(Ieee80211ManagementFrame *frame, const MACAddress& address);

    /** Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

    /** Utility function: converts Ieee80211StatusCode (->frame) to Ieee80211PrimResultCode (->primitive) */
    virtual int statusCodeToPrimResultCode(int statusCode);

    /** Redefined from PassiveQueueBase. Overwritten from Ieee80211MgmtBase */
    virtual cMessage *enqueue(cMessage *msg);
    virtual cMessage* dequeue();

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame);
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame);
    virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame);
    virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame);
    virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame);
    virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame);
    virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame);
    virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame);
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame);
    virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame);
    virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame);
    //@}

    /** @name Processing of different agent commands */
    //@{
    virtual void processScanCommand(Ieee80211Prim_ScanRequest *ctrl);
    virtual void processAuthenticateCommand(Ieee80211Prim_AuthenticateRequest *ctrl);
    virtual void processDeauthenticateCommand(Ieee80211Prim_DeauthenticateRequest *ctrl);
    virtual void processAssociateCommand(Ieee80211Prim_AssociateRequest *ctrl);
    virtual void processReassociateCommand(Ieee80211Prim_ReassociateRequest *ctrl);
    virtual void processDisassociateCommand(Ieee80211Prim_DisassociateRequest *ctrl);
    //@}
};

#endif


