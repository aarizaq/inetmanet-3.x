
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class ConvergenceSublayerReceiver : public cSimpleModule
{
  private:
    int commonPartGateIn;
    int higherLayerGateOut;
    cMessage *endTransmissionEvent;

  public:
    ConvergenceSublayerReceiver();
    virtual ~ConvergenceSublayerReceiver();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};
