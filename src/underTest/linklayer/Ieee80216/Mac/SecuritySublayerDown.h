
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class SecuritySublayerDown : public cSimpleModule
{
  private:
    cMessage* endTransmissionEvent;

  public:
    SecuritySublayerDown();
    virtual ~SecuritySublayerDown();

  protected:
    void initialize();
    void handleMessage(cMessage* msg);
};
