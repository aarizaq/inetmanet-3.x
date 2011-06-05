#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"
#include "Ieee80216ManagementMessages_m.h"
#include "Ieee80216Primitives_m.h"
#include "PhyControlInfo_m.h"
#include "RadioState.h"
#include "NotificationBoard.h"
#include "ControlPlaneMS.h"

class ControlPlaneBaseMS : public cSimpleModule, public ControlPlaneMS, public INotifiable
{
  public:
/**
* @name Struct ScanningInfo
* Informationen abspeicher die für den Scan benötigt werden
*/
    struct ScanningInfo
    {
        MACAddress BSID;        // specific BSID to scan for provider
        bool activeScan;        // whether to perform active or passive scanning
        std::vector<int> channelList; // list of channels to scan
        int currentChannelIndex; // index into channelList[]
        int scanChannel;
        bool busyChannelDetected; // during minChannelTime, we have to listen for busy channel
        double minChannelTime;  // minimum time to spend on each channel when scanning
        double maxChannelTime;  // maximum time to spend on each channel when scanning
        bool recieve_DL_MAP;
        bool recieve_DCD;
        bool recieve_UCD;
    };

/**
* @name Structur Basestation
* Abspeichern von Parametern jeder empfangenen Basisstation
*/
    struct BSInfo
    {
        MACAddress BasestationID;
        int DownlinkChannel;
        int UplinkChannel;
        cMessage *authTimeoutMsg; // if non-NULL: authentication is in progress
        //
        //BSInfo() {
        //    channel=-1;
        // authTimeoutMsg=NULL;
        //}
    };

    /**
     * @name BasestationList
     * Die Liste enthälte Parameter von allen gescänten Basisstationen
     */
    typedef std::list<BSInfo> BasestationList;
    BasestationList bsList;

    cMessage *ScanChannelTimer;
    cMessage *DLMAPTimer;

  public:
    MobileSubStationInfo MSInfo;
    NotificationBoard *nb;

  protected:
    // number of channels in ChannelControl -- used if we're told to scan "all" channels
    int numChannels;

    // scanning status
    bool isScanning;
    ScanningInfo scanning;

    // associated Access Point
    bool isAssociated;

    cMessage *assocTimeoutMsg;  // if non-NULL: association is in progress

    /** Physical radio (medium) state copied from physical layer */
    RadioState::State radioState;

  private:
    cGate * gateToWatch;
    cQueue queue;
    cMessage *endTransmissionEvent;

  public:
    ControlPlaneBaseMS();
    ~ControlPlaneBaseMS();

  protected:
    void initialize();

    void startTransmitting(cMessage *msg);

    void displayStatus(bool isBusy);
    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);

    /** Prozeduren für Events*/
    void handleManagmentFrame(Ieee80216GenericMacHeader *GenericFrame);
    void handleTimer(cMessage *msg);
    void handleCommand(int msgkind, cPolymorphic *ctrl);

    void sendLowerMessage(cMessage *msg);

    /** Prozeduren für abspeichern von BS */
    void storeBSInfo(const MACAddress & BasestationID);
    void UplinkChannelBS(const MACAddress & BasestationID, const int UpChannel);
    BSInfo *lookupBS(const MACAddress & BasestationID);
    void clearBSList();

    /** Prozeduren für scännen von Kanälen */
    void processScanCommand(Ieee80216Prim_ScanRequest *ctrl);
    void changeDownlinkChannel(int channelNum);
    void changeUplinkChannel(int channelNum);
    bool scanNextChannel();

    /** Prozeduren für MAC Managementnachrichten */
    void handleDL_MAPFrame(Ieee80216DL_MAP *frame);
    void handleDCDFrame(Ieee80216_DCD *frame);
    void handleUCDFrame(Ieee80216_UCD *frame);
    void handle_RNG_RSP_Frame(Ieee80216_RNG_RSP *frame);
    void handle_SBC_RSP_Frame(Ieee80216_SBC_RSP *frame);
    void handle_REG_RSP_Frame(Ieee80216_REG_RSP *frame);

    /** @brief Called by the NotificationBoard whenever a change occurs we're interested in */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
};
