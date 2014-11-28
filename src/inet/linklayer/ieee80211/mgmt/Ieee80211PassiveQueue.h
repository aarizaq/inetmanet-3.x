//
// Copyright (C) 2005 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IEEE80211PASSIVEQUEUE_H
#define __INET_IEEE80211PASSIVEQUEUE_H

#include <map>
#include "inet/common/INETDefs.h"
#include "inet/common/queue/PassiveQueueBase.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"


namespace inet {

namespace ieee80211 {

class INET_API Ieee80211PassiveQueue : public PassiveQueueBase
{
  public:
        // Multi queue implementation
        virtual int getNumQueues() = 0;
        virtual void requestPacket(const int&) = 0;
        virtual int getNumPendingRequests(const int&) = 0;
        virtual bool isEmpty(const int&) = 0;
        virtual void clear(const int&) = 0;
        virtual cMessage *pop(const int&) = 0;
        virtual void requestPacket() {PassiveQueueBase::requestPacket();}


        virtual Ieee80211DataOrMgmtFrame *getQueueElement(const int &, const int &) const = 0;
        virtual unsigned int getDataSize(const int &cat) const = 0;
        virtual unsigned int getManagementSize() const = 0;
};

}

} // namespace inet

#endif // ifndef __INET_PASSIVEQUEUEBASE_H

