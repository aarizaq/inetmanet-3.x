#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>

#include <Ieee80216ManagementMessages_m.h>

#include <global_enums.h>

#include <map>
using namespace std;

#ifndef Common_Part_Sublayer_ServiceFlows_H
#define Common_Part_Sublayer_ServiceFlows_H

/**
 * Module for WiMAX ServiceFlow Management
 */
class CommonPartSublayerServiceFlows : public cSimpleModule
{
  private:

  public:
    // Holds the mapping of existing CIDs to SFIDs. (unique)
    ConnectionMap map_connections;

    // Holds the actual ServiceFlow objects identified by their unique SFID
    ServiceFlowMap map_serviceFlows;

    // Hold the downlink connections
    ConnectionMap map_dl_connections;
    ServiceFlowMap map_dl_serviceFlows;

    int controlPlaneIn, controlPlaneOut;

  public:
    CommonPartSublayerServiceFlows();
    virtual ~CommonPartSublayerServiceFlows();

    ConnectionMap *getConnectionMap();
    ServiceFlowMap *getServiceFlowMap();

  protected:
    void initialize();

    Ieee80216_DSA_REQ *build_DSA_REQ(int prim_management_cid, ServiceFlow *sf, ip_traffic_types type);
    Ieee80216_DSA_RSP *build_DSA_RSP(int prim_management_cid, ServiceFlow *sf, ip_traffic_types type);
    Ieee80216_DSA_ACK *build_DSA_ACK(int prim_management_cid, ServiceFlow *sf, ip_traffic_types type);
};

#endif
