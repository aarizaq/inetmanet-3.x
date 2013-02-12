
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class SecuritySublayerTransceiver : public cSimpleModule
{
  private:
    int commonPartGateIn;
    int commonPartGateOut;
    int transceiverRadioGateIn;
    int transceiverRadioGateOut;

  public:
    SecuritySublayerTransceiver();
    virtual ~SecuritySublayerTransceiver();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};
