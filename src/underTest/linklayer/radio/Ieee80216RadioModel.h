
#ifndef IEEE80216RADIOMODEL_H
#define IEEE80216RADIOMODEL_H

#include "IRadioModel.h"

/**
 * Radio model for IEEE 802.11. The implementation is largely based on the
 * Mobility Framework's SnrEval80211 and Decider80211 modules.
 * See the NED file for more info.
 */
class INET_API Ieee80216RadioModel : public IRadioModel
{
  protected:
    double snirThreshold;

  public:
    virtual void initializeFrom(cModule* radioModule);
    virtual double calculateDuration(AirFrame* airframe);
    virtual bool isReceivedCorrectly(AirFrame* airframe, const SnrList& receivedList);
    // used by the Airtime Link Metric computation
    virtual bool haveTestFrame() {return false;}
    virtual double calculateDurationTestFrame(AirFrame *airframe) {return 0;}
    virtual double getTestFrameError(double snirMin, double bitrate) {return 0;}
    virtual int    getTestFrameSize() {return 0;}
  protected:
    // utility
    virtual bool isPacketOK(double snirMin, int lengthMPDU, double bitrate);
    // utility
    virtual double dB2fraction(double dB);
};

#endif
