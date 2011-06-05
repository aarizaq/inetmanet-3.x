#ifndef WIMAXPATHLOSSRECEPTIONMODEL_H
#define WIMAXPATHLOSSRECEPTIONMODEL_H

#include "IReceptionModel.h"

/**
 * Path loss model which calculates the received power using a path loss exponent
 * and the distance.
 */
class INET_API WiMAXPathLossReceptionModel : public IReceptionModel
{
  private:
    double standardabweichung;

  public:
    cOutVector normalVector;
    cOutVector logNormalVector;

    /**
     * Parameters read from the radio module: pathLossAlpha.
     */
    virtual void initializeFrom(cModule* radioModule);

    /**
     * Perform the calculation.
     */
    virtual double calculateReceivedPower(double pSend, double carrierFrequency, double distance);
};

#endif
