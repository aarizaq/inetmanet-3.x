//
// Copyright (C) 2016 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

//#include "inet/linklayer/ieee80211/mac/common/MacUtils.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateControl.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/statistics/BasicStatistics.h"

namespace inet {
namespace ieee80211 {

Define_Module(BasicStatistics);
#ifdef ACTIVE80211STATISTICS

void BasicStatistics::initialize()
{
    resetStatistics();

    WATCH(numRetry);
    WATCH(numSentWithoutRetry);
    WATCH(numGivenUp);
    WATCH(numCollision);
    WATCH(numSent);
    WATCH(numSentBroadcast);

    WATCH(numReceivedUnicast);
    WATCH(numReceivedBroadcast);
    WATCH(numReceivedMulticast);
    WATCH(numReceivedNotForUs);
    WATCH(numReceivedErroneous);
}

void BasicStatistics::resetStatistics()
{
    numRetry = 0;
    numSentWithoutRetry = 0;
    numGivenUp = 0;
    numCollision = 0;
    numSent = 0;
    numSentBroadcast = 0;

    numReceivedUnicast = 0;
    numReceivedMulticast = 0;
    numReceivedBroadcast = 0;
    numReceivedNotForUs = 0;
    numReceivedErroneous = 0;
    numUpperDiscarded = 0;
}

void BasicStatistics::finish()
{

   // receive statistics
    recordScalar("numReceivedUnicast", numReceivedUnicast);
    recordScalar("numReceivedBroadcast", numReceivedBroadcast);
    recordScalar("numReceivedMulticast", numReceivedMulticast);
    recordScalar("numReceivedNotForUs", numReceivedNotForUs);
    recordScalar("numReceivedErroneous", numReceivedErroneous);
}

void BasicStatistics::setMacUtils(MacUtils *utils)
{
    this->utils = utils;
}

void BasicStatistics::setRateControl(IRateControl *rateControl)
{
    this->rateControl = rateControl;
}

void BasicStatistics::frameTransmissionSuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount)
{

}

void BasicStatistics::frameTransmissionUnsuccessful(Ieee80211DataOrMgmtFrame *frame, int retryCount)
{

}

void BasicStatistics::frameTransmissionUnsuccessfulGivingUp(Ieee80211DataOrMgmtFrame *frame, int retryCount)
{

}

void BasicStatistics::frameTransmissionGivenUp(Ieee80211DataOrMgmtFrame *frame)
{
    numGivenUp++;
}

void BasicStatistics::frameReceived(Ieee80211Frame *frame)
{

}

void BasicStatistics::erroneousFrameReceived()
{
    numReceivedErroneous++;
}

void BasicStatistics::upperFrameDiscarded(Ieee80211Frame *frame) {
    numUpperDiscarded++;
}

#endif

}  // namespace ieee80211
}  // namespace inet

