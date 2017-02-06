/*
 * DirectionalDataBase.h
 *
 *  Created on: 6 feb. 2017
 *      Author: Alfonso
 */

#ifndef INET_LINKLAYER_COMMON_DIRECTIONALDATABASE_H_
#define INET_LINKLAYER_COMMON_DIRECTIONALDATABASE_H_


//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <deque>
#include <map>

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/geometry/common/EulerAngles.h"

namespace inet {

class INET_API DirectionalDataBase : public cSimpleModule, protected cListener
{
  protected:

    struct Data {
        simtime_t time;
        EulerAngles angle;
    };

    static simsignal_t angleNotification;

    unsigned int maxDataSize = 20;

    typedef std::deque<Data> DataVector;
    typedef std::map<MACAddress, DataVector> DataMap;
    DataMap dataMap;

  protected:
    // displays summary above the icon
    virtual void updateDisplayString();


  public:
    DirectionalDataBase();
    virtual ~DirectionalDataBase();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    /**
     * Raises an error.
     */
    virtual void handleMessage(cMessage *) override;

  public:
    /**
     * Called by the signal handler whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj,  cObject *details) override;

    /**
     * Returns the host or router this interface table lives in.
     */
    virtual EulerAngles getDirection(const MACAddress &);
    virtual void setDirection(const MACAddress &addr, const EulerAngles &);


};

} // namespace inet


#endif /* INET_LINKLAYER_COMMON_DIRECTIONALDATABASE_H_ */
