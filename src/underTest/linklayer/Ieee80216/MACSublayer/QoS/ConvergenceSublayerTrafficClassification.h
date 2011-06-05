#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>

#include "IPv4Datagram.h"
#include "PhyControlInfo_m.h"
#include "IPv4.h"

#include "Ieee80216ManagementMessages_m.h"
#include "Ieee80216Primitives_m.h"
#include "global_enums.h"

#include "ControlPlaneBase.h"
#include "ControlPlaneBasestation.h"
#include "ControlPlaneMobilestation.h"

/*
#include <map>
using namespace std;
*/

/**
 * Module for WiMAX convergency traffic classification
 */
class ConvergenceSublayerTrafficClassification : public cSimpleModule
{
  private:
    //vector<int> existing_cids;
    cMessage *test_timer;
    string *module_name;

    int voip_max_latency, voip_tolerated_jitter;

    // temporary map to control triggering of DSA-REQs
    // (currently, only the key is used as ip_traffic_type indicator)
    map<int, int> current_tt_requests;

    vector<ServiceFlow> provisioned_serviceFlows;

    ControlPlaneBase *controlplane;

    double sf_request_timeout;

    ControlPlaneMobilestation *controlPlaneMobilstation;

    int findMatchingServiceFlow(int traffic_type);
    void startNewRetryTimer(ip_traffic_types traffic_type);
    void startHORetryTimer(ip_traffic_types traffic_type);
  protected:
    int higherLayerGateIn_ftp, higherLayerGateIn_voice_no_supr, higherLayerGateIn_voice_supr,
        higherLayerGateIn_video_stream, higherLayerGateIn_guaranteed_minbw_web_access;

    int higherLayerGateOut;
    int headerCompressionGateIn, headerCompressionGateOut;
    int controlPlaneOut;

  public:
    ConvergenceSublayerTrafficClassification();
    virtual ~ConvergenceSublayerTrafficClassification();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);
    void handleUnclassifiedMessage(cMessage *msg);
    void handleSelfMessage(cMessage *msg);
};
