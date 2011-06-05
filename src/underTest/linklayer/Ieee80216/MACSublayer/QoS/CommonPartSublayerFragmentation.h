#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Module for WiMAX convergency traffic classification 
 */
class CommonPartSublayerFragmentation : public cSimpleModule
{
  private:
    int commonPartGateOut;
    int commonPartGateIn;
    int securityGateIn;
    int securityGateOut;
    int schedulingGateIn;

  public:
    CommonPartSublayerFragmentation();
    virtual ~CommonPartSublayerFragmentation();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);
    void handleSelfMessage(cMessage *msg);
};
