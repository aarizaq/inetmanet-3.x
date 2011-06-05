#include <stdio.h>
#include <global_enums.h>

/**
 * Module for WiMAX convergency traffic classification 
 */
class CommonPartSublayerAuthorizationModule : public cSimpleModule
{
  private:
    ServiceFlowMap *map_serviceFlows;

    int availableDatarate[];

    ip_traffic_types getTypeOfServiceFlow(sf_QoSParamSet *req_params);
    void updateDisplay();

  public:
    CommonPartSublayerAuthorizationModule();
    virtual ~CommonPartSublayerAuthorizationModule();

    bool checkQoSParams(sf_QoSParamSet *req_params, link_direction link_type);
    void setAvailableUplinkDatarate(int datarate);
    void setAvailableDownlinkDatarate(int datarate);

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);
    void handleSelfMessage(cMessage *msg);
};
