
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class ConvergenceSublayerTransceiver : public cSimpleModule
{
  private:
    cMessage *endTransmissionEvent;

    int higherLayerGateIn;
    int commonPartGateOut;

  public:
    ConvergenceSublayerTransceiver();
    virtual ~ConvergenceSublayerTransceiver();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};
