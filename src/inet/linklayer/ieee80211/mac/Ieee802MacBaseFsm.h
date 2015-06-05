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

#ifndef IEEE802MACBASEFSM_H_
#define IEEE802MACBASEFSM_H_

#include <omnetpp.h>
#include <vector>



namespace inet {
namespace ieee80211 {

class Ieee80211Mac;

class Ieee802MacBaseFsm : public cFSM
{
        Ieee80211Mac *mac;
        typedef void (Ieee80211Mac::*StateMethod) (Ieee802MacBaseFsm &, cMessage*);
        std::vector<StateMethod> vectorStates;
        bool initialState = true;
        bool isImmediate = false;
    public:
        Ieee802MacBaseFsm();
        virtual ~Ieee802MacBaseFsm();
        virtual void setNumStates(const unsigned int &states){vectorStates.resize(states);}
        virtual void execute(cMessage *);

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif /* IEEE802MACBASEFSM_H_ */
