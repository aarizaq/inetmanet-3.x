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

#include <vector>
#include <string>
#include "inet/common/INETDefs.h"



namespace inet {
namespace ieee80211 {

class Ieee80211Mac;
class Ieee802MacBaseFsm;
typedef void (Ieee80211Mac::*Ieee80211MacStateMethod) (Ieee802MacBaseFsm *, cMessage*);


#define FSMIEEE80211_Enter(___fsm)  if (___fsm->isCondition())

#define FSMIEEE80211_No_Event_Transition(__fsm, condition) if ((condition) && ! __fsm->event())

#define FSMIEEE80211_Transition(___fsm, target) \
    ___fsm->setState(target, #target); \
    return

#define FSMIEEE80211_Event_Transition(__fsm, condition) if ((condition) &&  __fsm->isEvent())

class Ieee802MacBaseFsm : public cFSM
{
        Ieee80211Mac *mac = nullptr;
        std::vector<Ieee80211MacStateMethod> vectorStates;
        std::vector<std::string> vectorInfo;
        bool ___is_event = true;
        bool ___exit = false;
        bool ___condition_seen = false;
        bool ___debug = false;
    public:
        virtual bool & debug() {return ___debug;}
        virtual bool & event() {return ___is_event;}
        virtual bool & exit() {return ___exit;}
        virtual bool & condition() {return ___condition_seen;}
        virtual bool isCondition();
        virtual bool isEvent();

        virtual ~Ieee802MacBaseFsm();
        Ieee802MacBaseFsm(int);
        //virtual void setNumStates(int);
        virtual void execute(cMessage *);
        virtual void setStateMethod(int state, Ieee80211MacStateMethod p, const char *msg);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif /* IEEE802MACBASEFSM_H_ */
