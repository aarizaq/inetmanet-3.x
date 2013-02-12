
#ifndef _DECIDER_80216_H
#define _DECIDER_80216_H

//#include <BasicDecider.h>


#include "INETDefs.h"
#include "BasicModule.h"
#include "AirFrame_m.h"
#include "SnrControlInfo_m.h"
#include "SnrList.h"

//#include"ControlPlaneBase.h"

//class INET_API Decider80216 : public BasicDecider
class INET_API Decider80216 : public BasicModule
{
  protected:
    /** @brief gate id*/
    /*@{*/
    int upperLayerOut;
    int lowerLayerIn;
    /*@}*/

    /** @brief statistics*/
    /*@{*/
    unsigned long numRcvd;
    unsigned long numSentUp;
    /*@}*/
    /** @brief Level for decision [mW]
     *
     * When a packet contains an snr level higher than snrThresholdLevel it
     * will be considered as lost. This parameter has to be specified at the
     * beginning of a simulation (omnetpp.ini) in dBm.
     */
    double snrThresholdLevel;

    struct SnrStruct
    {
        cMessage *ptr;
        double rcvdPower;
        double simTime;
    };

    SnrStruct snrInfo;
    typedef std::list<SnrStruct> SnrFrameList;
    SnrFrameList SnrMacList;

    cModule *parent;
    cModule *subParent;

  protected:
    virtual void initialize(int stage);
    void getSnrList(AirFrame *af, SnrList &receivedList);
    virtual void handleMessage( cMessage* );
    virtual void handleSelfMsg(cMessage *msg){delete msg;};
    virtual void sendUp(AirFrame*);

  protected:
    bool snrOverThreshold(SnrList &list) const;
    virtual void handleLowerMsg(AirFrame *af, SnrList &list);
};

#endif
