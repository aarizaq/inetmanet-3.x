
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "MACAddress.h"
#include "Ieee80216MacHeader_m.h"
#include "WiMAXRadioControl_m.h"

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class CommonPartSublayerDown : public cSimpleModule
{
  private:
    cGate *gateToWatch;
    cQueue queue;
    cMessage *endTransmissionEvent;
    int frame_CID;

  public:
    CommonPartSublayerDown();
    virtual ~CommonPartSublayerDown();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
    void handleCommand(int msgkind, cPolymorphic *ctrl);

    void handleControlPlaneMsg(cMessage *msg);
    void handleUpperMsg(cMessage *msg);
};
