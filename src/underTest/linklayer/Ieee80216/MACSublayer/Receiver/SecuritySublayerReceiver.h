
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class SecuritySublayerReceiver : public cSimpleModule
{
  protected:
    /** @brief gate id*/
    //@{
    int receiverRadioGateIn;
    int receiverRadioGateOut;
    int commonPartGateIn;
    int commonPartGateOut;

  private:

  public:
    SecuritySublayerReceiver();
    virtual ~SecuritySublayerReceiver();

  protected:
    void initialize();
    void handleMessage(cMessage *msg);
};
