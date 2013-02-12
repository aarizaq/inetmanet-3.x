#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

#include "IInterfaceTable.h"
// #include "InterfaceEntry.h"

/**
 * Module for WiMAX convergency traffic classification
 */
class CommonPartSublayerQoSAP : public cSimpleModule
{
  private:
    IInterfaceTable *itable;

  public:
    CommonPartSublayerQoSAP();
    virtual ~CommonPartSublayerQoSAP();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);
    void handleSelfMessage(cMessage *msg);
};
