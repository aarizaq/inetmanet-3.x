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

#include "DinamicWirelessNodeManager.h"
#include "MobilityBase.h"

DinamicWirelessNodeManager::DinamicWirelessNodeManager()
{
    // TODO Auto-generated constructor stub
    nextNodeVectorIndex =0;

}

DinamicWirelessNodeManager::~DinamicWirelessNodeManager()
{
    // TODO Auto-generated destructor stub
}

void DinamicWirelessNodeManager::newNode(std::string name, std::string type, const Coord& position, simtime_t endLife)
{
    cModule* parentmod = getParentModule();
    int nodeVectorIndex = nextNodeVectorIndex++;
    if (!parentmod) error("Parent Module not found");

    cModuleType* nodeType = cModuleType::get(type.c_str());
    if (!nodeType) error("Module Type \"%s\" not found", type.c_str());

    //TODO: this trashes the vectsize member of the cModule, although nobody seems to use it
    cModule* mod = nodeType->create(name.c_str(), parentmod, nodeVectorIndex, nodeVectorIndex);
    mod->finalizeParameters();
    mod->buildInside();
    mod->scheduleStart(simTime());

    MobilityBase* mm = NULL;
    for (cModule::SubmoduleIterator iter(mod); !iter.end(); iter++) {
        cModule* submod = iter();
        mm = dynamic_cast<MobilityBase*>(submod);
        if (!mm) continue;
        mm->par("initialX").setDoubleValue(position.x);
        mm->par("initialY").setDoubleValue(position.y);
        mm->par("initialZ").setDoubleValue(position.z);
        break;
    }
    if (mm)
        error("Mobility modele not found");
    NodeInf info;
    info.endLife = endLife;
    info.module = mod;
    nodeList[mod->getId()] = info;
    mod->callInitialize();
}


void DinamicWirelessNodeManager::deleteNode(int nodeId)
{
    cModule* mod = simulation.getModule(nodeId);


    if (!mod) error("no node with Id \"%i\" not found", nodeId);

    if (!mod->getSubmodule("notificationBoard")) error("host has no submodule notificationBoard");

    std::map<int,NodeInf>::iterator it = nodeList.find(nodeId);
    if (it == nodeList.end())
        error("no node with Id \"%i\" not found in the list", nodeId);
    nodeList.erase(it);
    mod->callFinish();
    mod->deleteModule();
}

