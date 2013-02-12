
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class SecuritySublayerUp : public cSimpleModule
{
  private:
    cMessage* endTransmissionEvent;

  public:
    SecuritySublayerUp();
    virtual ~SecuritySublayerUp();

  protected:
    void initialize();
    void handleMessage(cMessage* msg);
};
