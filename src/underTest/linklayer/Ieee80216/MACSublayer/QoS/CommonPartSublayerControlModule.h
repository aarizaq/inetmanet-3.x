#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Module for WiMAX convergency traffic classification 
 */
class CommonPartSublayerControlModule : public cSimpleModule
{
  private:
    int authIn, authOut, controlIn, controlOut, serviceflowIn, serviceflowOut;

  public:
    CommonPartSublayerControlModule();
    virtual ~CommonPartSublayerControlModule();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);
    void handleSelfMessage(cMessage *msg);
};
