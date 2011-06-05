#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>

#include <CommonPartSublayerServiceFlows.h>
//#include <CommonPartSublayerAuthorizationModule.h>
#include <Ieee80216ManagementMessages_m.h>

#include <global_enums.h>

#include <map>
using namespace std;

class CommonPartSublayerServiceFlows_BS : public CommonPartSublayerServiceFlows
{
  private:
    // When a ServiceFlow is removed from the system, its ID is saved in this list.
    // The purpose is to re-use this ID later, when a new ServiceFlow is added - so
    // first this list is queried for available IDs before the highest existing ID
    // is incremented by 1.
    list<int> list_removed_SFIDs;

    int allowed_connections;    // the maximum allowed simultaneous connections
    int cur_max_basic_cid, cur_max_primary_cid, //helper: hold the currently highest ID of the
        cur_max_sfid, cur_max_secondary_cid; //corresponding type for CID generation...

    int lower_bound_for_BE_traffic, upper_bound_for_BE_grant;

    // pointer to the connected ControlPlane modules' MSSubscriberList
    MobileSubscriberStationList *mssList;

    // pointer to own BSInfo object
    structBasestationInfo *localBSInfo;

    short pending_requests;

    list<ServiceFlow *> serviceFlowsSortedByPriority[6];

//  CommonPartSublayerAuthorizationModule *cps_auth;

  public:
    CommonPartSublayerServiceFlows_BS();
    ~CommonPartSublayerServiceFlows_BS();

    // Creates a new CID/SFID+ServiceFlow for a given Management Connection type.
    // These are then directly put into the corresponding maps.
    void createManagementConnection(structMobilestationInfo *registered_ss, management_type type);

    void createAndSendNewDSA_REQ(int prim_management_cid, ServiceFlow *requested_sf, ip_traffic_types type);

    void setMSSList(MobileSubscriberStationList *cp_mssList);
    void setBSInfo(structBasestationInfo *bsinfo);

    int availableDatarate[2];
    bool checkQoSParams(sf_QoSParamSet *req_params, link_direction link_type);
    bool kickFlowBelow(ip_traffic_types needy_flow, link_direction link_type, int demanded_traffic_rate);
    void setAvailableUplinkDatarate(int datarate);
    void setAvailableDownlinkDatarate(int datarate);
    ip_traffic_types getTypeOfServiceFlow(sf_QoSParamSet *req_params);

    void sortServiceFlowsByPriority();

    void updateDisplay();

  protected:
    void initialize();

    void handleMessage(cMessage *msg);
    void handle_DSA_REQ(Ieee80216_DSA_REQ *dsa_req);
    void handle_DSA_RSP(Ieee80216_DSA_RSP *dsa_rsp);
    void handle_DSA_ACK(Ieee80216_DSA_ACK *dsa_ack);

//        void createConnection( int ms_cid, ip_traffic_types traffic_type );
    int getFreeSFID();
    int getNewManagementCID(management_type mtype);

    structMobilestationInfo *lookupMS(MACAddress mac_addr);
};
