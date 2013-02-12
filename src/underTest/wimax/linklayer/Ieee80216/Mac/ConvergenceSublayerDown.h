
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class ConvergenceSublayerDown : public cSimpleModule
{
  private:
    cMessage *endTransmissionEvent;
  public:
    ConvergenceSublayerDown();
    virtual ~ConvergenceSublayerDown();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};
