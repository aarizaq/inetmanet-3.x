#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Module for WiMAX convergency traffic classification 
 */
class ConvergenceSublayerHeaderCompression : public cSimpleModule
{
  private:
    int trafficClassificationGateIn, trafficClassificationGateOut;
    int commonPartGateIn, commonPartGateOut;

  public:
    ConvergenceSublayerHeaderCompression();
    virtual ~ConvergenceSublayerHeaderCompression();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);
    void handleSelfMessage(cMessage *msg);
};
