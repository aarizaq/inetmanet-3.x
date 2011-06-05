#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>

#include <CommonPartSublayerServiceFlows.h>
#include <Ieee80216ManagementMessages_m.h>

#include <global_enums.h>

#include <map>
using namespace std;

class CommonPartSublayerServiceFlows_MS : public CommonPartSublayerServiceFlows
{
  public:
    CommonPartSublayerServiceFlows_MS();
    ~CommonPartSublayerServiceFlows_MS();

    void createAndSendNewDSA_REQ(int prim_management_cid, ServiceFlow *requested_sf,
                                 ip_traffic_types type);

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handle_DSA_REQ(Ieee80216_DSA_REQ *dsa_req);
    void handle_DSA_RSP(Ieee80216_DSA_RSP *dsa_rsp);
    void handle_DSA_ACK(Ieee80216_DSA_ACK *dsa_ack);
};
