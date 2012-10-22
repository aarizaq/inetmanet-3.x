//
// Copyright (C) 2012 Calin Cerchez
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef LTERADIOMODEL_H_
#define LTERADIOMODEL_H_

#include "IRadioModel.h"

#include "IRadioModel.h"
#include "IModulation.h"

class INET_API LTERadioModel : public IRadioModel
{
  protected:
    double snirThreshold;
    long headerLengthBits;
    double bandwidth;
    IModulation *modulation;

  public:
    LTERadioModel();
    virtual ~LTERadioModel();

    virtual void initializeFrom(cModule *radioModule);

    virtual double calculateDuration(AirFrame *airframe);

    virtual bool isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList);
    // used by the Airtime Link Metric computation
    virtual bool haveTestFrame() {return false;}
    virtual double calculateDurationTestFrame(AirFrame *airframe) {return 0;}
    virtual double getTestFrameError(double snirMin, double bitrate) {return 0;}
    virtual int getTestFrameSize() {return 0;}
  protected:
    // utility
    virtual bool isPacketOK(double snirMin, int length, double bitrate);
    // utility
    virtual double dB2fraction(double dB);
};

#endif /* LTERADIOMODEL_H_ */
