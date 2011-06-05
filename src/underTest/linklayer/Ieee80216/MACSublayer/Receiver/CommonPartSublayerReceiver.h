#ifndef COMMON_PART_SUBLAYER_H
#define COMMON_PART_SUBLAYER_H

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"
#include "Ieee80216MacHeader_m.h"
#include "global_enums.h"

#include <map>
using namespace std;

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class CommonPartSublayerReceiver : public cSimpleModule
{
  protected:
    /** @brief gate id*/

    int convergenceGateOut;
    int controlPlaneGateIn;
    int controlPlaneGateOut;
    int fragmentationGateIn;
    int fragmentationGateOut;

  private:
    ConnectionMap *map_connections;
    MobileSubscriberStationList *msslist;

    int eigene_CID;

    int cur_floor, last_floor;
    int downlink_rate;
    int downlink_daten;

    cOutVector cvec_endToEndDelay[BE + 1];
    map<string, vector<cOutVector *> > map_delay_vectors;
    map<string, double[pBE + 1]> map_sums;
    cOutVector cvec_downlinkUGS;

  public:
    CommonPartSublayerReceiver();
    virtual ~CommonPartSublayerReceiver();

    void setConnectionMap(ConnectionMap *pointer_ConnectionsMap);
    void setSubscriberList(MobileSubscriberStationList *pointer_msslist); // for BS
    void setSubscriberList(structMobilestationInfo *mssinfo); // for MS

  protected:
    void initialize();

    void handleMessage(cMessage *msg);
    /**
    * @brief Wird von handleMessage aufgerufen wenn die Nachricht von dem ControlPlane Module kommt
    */
    void handleControlPlaneMsg(cMessage *msg);
    void handleCommand(cMessage *msg);

    void handleLowerMsg(cMessage *msg);
    void handleMacFrameType(Ieee80216MacHeader *MacFrame);

    bool CIDInConnectionMap(int mac_pdu_CID);
    bool isManagementCID(int mac_pdu_CID);

    void updateDisplay();
};

#endif
