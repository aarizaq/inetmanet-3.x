#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

#include "PhyControlInfo_m.h"

#include "IPv4Datagram.h"
#include "InterfaceTableAccess.h"
#include "Ieee80216ManagementMessages_m.h"

/**
 * Control Module for WiMAX MAC convergence sublayer
 */

class ConvergenceSublayerControlModule : public cSimpleModule
{
  private:
    cMessage *timerEvent;

  protected:
    int outerIn, outerOut;
    int tcIn, tcOut;
    int fragIn, fragOut;
    int compIn, compOut;

  public:
    ConvergenceSublayerControlModule();
    virtual ~ConvergenceSublayerControlModule();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleUpperLayerMessage(cPacket *msg);
    void handleSelfMessage(cMessage *msg);
};
