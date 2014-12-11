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

/*void Ieee802MacBaseFsm::setNumStates(int numStates)
{
    // TODO Auto-generated constructor stub
    mac = dynamic_cast<Ieee80211Mac*>(this->getOwner());
    vectorStates.resize(numStates);
    vectorInfo.resize(numStates);
    for (unsigned int i = 0; i < vectorStates.size(); i++)
        vectorStates[i] = nullptr;
}
*/

Ieee802MacBaseFsm::Ieee802MacBaseFsm(int numStates)
{
    // TODO Auto-generated constructor stub
    mac = dynamic_cast<Ieee80211Mac*>(this->getOwner());
    vectorStates.resize(numStates);
    vectorInfo.resize(numStates);
    for (unsigned int i = 0; i < vectorStates.size(); i++)
        vectorStates[i] = nullptr;
}

Ieee802MacBaseFsm::~Ieee802MacBaseFsm()
{
    // TODO Auto-generated destructor stub
}

bool Ieee802MacBaseFsm::isCondition()
{
    if (!___is_event)
    {
        if (!___condition_seen)
        {
            ___condition_seen = true;
            exit() = false;
            return false;
        }
        else
            throw cRuntimeError("FSMIEEE80211_Enter() must precede all FSMA_*_Transition()'s in the code"); \
    }
    return true;
}

bool Ieee802MacBaseFsm::isEvent()
{
    if (event())
    {
        event() = false;
        return true;
    }
    return false;
}

void Ieee802MacBaseFsm::execute(cMessage *msg)
{

    condition() = false;
    event()  = true;
    do
    {
        exit() = true;
        EV_DEBUG << "Processing Ieee80211 FSM, state" <<  vectorInfo[getState()] << endl;
        int oldState = getState();
        Ieee80211MacStateMethod method = vectorStates[getState()];
        (mac->*method)(this, msg);
        if (oldState != getState())
        {
            // continue
            exit() = false;
        }
    } while(!exit());
    EV_DEBUG << "End Ieee80211 FSM, state" <<  vectorInfo[getState()] << endl;
}

void Ieee802MacBaseFsm::Ieee802MacBaseFsm::setStateMethod(int state, Ieee80211MacStateMethod p, const char *msg)
{
    if (state < 0 || vectorStates.size())
        throw cRuntimeError("Invalid stae: '%d'", state);
    vectorStates[state] = p;
    vectorInfo[state] = msg;
}

} /* namespace ieee80211 */
} /* namespace inet */
