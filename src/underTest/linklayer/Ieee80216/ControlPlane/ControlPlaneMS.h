#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"
#include "Ieee80216ManagementMessages_m.h"
#include "Ieee80216Primitives_m.h"
#include "PhyControlInfo_m.h"
#include "WiMAXRadioControl_m.h"

// message kind values for timers
#define MK_Scan_Channel_Timer   1
#define MK_DL_MAP_Timeout       2

class ControlPlaneMS
{
  public:
    /**
    * MobileSubScriberStaion enthält 
    * Parameter, die die Mobilestaton
    * beschreiben
    **********************************/
    struct MobileSubStationInfo
    {
        MACAddress MobileMacAddress; //MacAdrese der Mobilesubscriber Station
        int ScanTimeInterval;
        int DownlinkChannel;
        int UplinkChannel;
    };

  protected:
    virtual void sendLowerMessage(cMessage *msg) = 0;

    void makeRNG_REQ(MobileSubStationInfo MSInfo);
    void makeSBC_REQ(MobileSubStationInfo MSInfo);
    void makeREG_REQ(MobileSubStationInfo MSInfo);
};
