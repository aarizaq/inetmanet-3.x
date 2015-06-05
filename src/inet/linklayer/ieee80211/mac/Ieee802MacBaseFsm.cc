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

#include "inet/linklayer/ieee80211/mac/Ieee802MacBaseFsm.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"

namespace inet {
namespace ieee80211 {

Ieee802MacBaseFsm::Ieee802MacBaseFsm()
{
    // TODO Auto-generated constructor stub
    mac = dynamic_cast<Ieee80211Mac*>(this->getOwner());
}

Ieee802MacBaseFsm::~Ieee802MacBaseFsm()
{
    // TODO Auto-generated destructor stub
}


void Ieee802MacBaseFsm::execute(cMessage *msg)
{
    isImmediate = false;
    do
    {
        StateMethod method = vectorStates[getState()];
        (mac->*method)(*this, msg);
    } while(isImmediate);
}


} /* namespace ieee80211 */
} /* namespace inet */
