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

#ifndef DINAMICWIRELESSNODEMANAGER_H_
#define DINAMICWIRELESSNODEMANAGER_H_
#include <omnetpp.h>
#include "INETDefs.h"
#include "Coord.h"

class DinamicWirelessNodeManager : public cSimpleModule
{
    private:
        class NodeInf
        {
            public:
            cModule *module;
            simtime_t endLife;
        };
        std::map<int,NodeInf> nodeList;
        int nextNodeVectorIndex;
    public:
        DinamicWirelessNodeManager();
        virtual ~DinamicWirelessNodeManager();
        virtual void initialize();
        virtual void finish();
        virtual void handleMessage(cMessage *msg);
        virtual void handleSelfMsg(cMessage *msg);
        void newNode(std::string name, std::string nodeId, const Coord& position, simtime_t);
        void deleteNode(int nodeId);
};

#endif /* DINAMICWIRELESSNODEMANAGER_H_ */
