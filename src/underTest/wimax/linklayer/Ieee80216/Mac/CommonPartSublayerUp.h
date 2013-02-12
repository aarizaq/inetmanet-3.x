
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
class CommonPartSublayerUp : public cSimpleModule
{
  private:
    cGate * gateToWatch;

    cQueue queue;
    cMessage *endTransmissionEvent;
    int eigene_CID;

  public:
    CommonPartSublayerUp();
    virtual ~CommonPartSublayerUp();

  protected:
    void initialize();

    void handleMessage(cMessage *msg);
    void handleCommand(int msgkind, cPolymorphic *ctrl);

    void handleLowerMsg(cPacket *msg);
    void handleMacFrameType(Ieee80216MacHeader *MacFrame);
};
