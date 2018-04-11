//
// Copyright (C) 2013, NESG (Network Engineering and Security Group), http://nesg.ugr.es,
// - Gabriel Maciá Fernández (gmacia@ugr.es)
// - Leovigildo Sánchez Casado (sancale@ugr.es)
// - Rafael A. Rodríguez Gómez (rodgom@ugr.es)
// - Roberto Magán Carrión (rmagan@ugr.es)
// - Pedro García Teodoro (pgteodor@ugr.es)
// - José Camacho Páez (josecamacho@ugr.es)
// - Jesús E. Díaz Verdejo (jedv@ugr.es)
//
// This file is part of NETA.
//
//    NETA is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NETA is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NETA.  If not, see <http://www.gnu.org/licenses/>.
//

#include "inet/neta/attacks/controllers/NA_Attack.h"
#include "inet/neta/common/log/NA_NesgLog.h"
#include "inet/neta/hackedmodules/NA_HackedModule.h"
#include "inet/neta/hackedmodules/networklayer/ipv4/NA_IPv4.h"

namespace inet {

namespace neta {


Define_Module (NA_Attack);

void NA_Attack::initialize() {

    // Get the type of attack to be launched
    attackType = (char*) par(NA_ATTACK_TYPE).stringValue();

    // Activate the attack only if defined in the active parameter in module (.ned)
    if (par(NA_ATTACK_ACTIVE).boolValue() == true) {

        getAttackModules();
        scheduleAttack();
    }
}

void NA_Attack::getAttackModules() {
    cTopology topo; //Used to discover the topology of the node and find modules for activating the attack
    cTopology::Node *node;
    string nodeName;

    // extract all modules with the @attackType property set in the simulation
    topo.extractByProperty(par(NA_ATTACK_TYPE).stringValue());

    LOG << "------------------------------------\n";
    LOG << "Found " << topo.getNumNodes() << " possible modules for attack\n";
    LOG << "------------------------------------\n";

    // Now, only the modules contained in the parent module of this NA_Attack object are activated.
    string prefix = this->getParentModule()->getFullPath(); // First we get the full path of the parent node
    int numModules = 0;
    for (int i = 0; i < topo.getNumNodes(); i++) {
        node = topo.getNode(i);
        nodeName = node->getModule()->getFullPath();
        if (not nodeName.compare(0, prefix.size(), prefix)) {

            LOG << "--->Inserting module in list: " << nodeName << "\n";
            modList.push_back(node->getModule());
            numModules++;
        }
    }
    LOG << "-----------------------------------\n";
    LOG << "Inserted " << numModules << " modules in list\n";
    LOG << "-----------------------------------\n";
}

void NA_Attack::scheduleAttack() {
    cMessage *msg = new cMessage(NA_ATTACK_START_MESSAGE);
    LOG << "Scheduling the attack \n";
    scheduleAt(par(NA_ATTACK_START_TIME).doubleValue(), msg);
    if (par("endTime").doubleValue()) //When the value differs from 0
    {
        cMessage *msgEnd = new cMessage(NA_ATTACK_END_MESSAGE);
        scheduleAt(par(NA_ATTACK_END_TIME).doubleValue(), msgEnd);
    }
}

void NA_Attack::handleMessage(cMessage *msg) {
    LOG << "Message received: " << msg->getFullName() << "\n";
    if (not strcmp(msg->getFullName(), NA_ATTACK_START_MESSAGE)) {
        activateModules();
    } else {
        deactivateModules();
    }
    delete (msg);
}

void NA_Attack::activateModules() {
    char msgCaption[30];

    // Concatenate the <attackType> + Activate
    opp_strcpy(msgCaption, attackType);
    strcat(msgCaption, NA_ATTACK_ACTIVATE_TAG);

    // Generate the specific attack controller message.
    // This method belongs to the specific attack controller.
    cMessage *msg = check_and_cast<cMessage *>(
            generateAttackMessage(msgCaption));
    EV << "\n\n";
    LOG << "-----------------------------------\n";
    LOG << "ACTIVATING HACKED MODULES\n";
    LOG << "-----------------------------------\n";

    sendMessageToHackedModules(msg);
}

void NA_Attack::deactivateModules() {

    char msgCaption[30];

    // Concatenate the <attackType> + Activate
    opp_strcpy(msgCaption, attackType);
    strcat(msgCaption, NA_ATTACK_DEACTIVATE_TAG);

    // Generate the specific attack controller message.
    // This method belongs to the specific attack controller.
    cMessage *msg = check_and_cast<cMessage *>(
            generateAttackMessage(msgCaption));

    EV << "\n\n";
    LOG << "-----------------------------------\n";
    LOG << "DEACTIVATING HACKED MODULES\n";
    LOG << "-----------------------------------\n";

    sendMessageToHackedModules(msg);
}

void NA_Attack::sendMessageToHackedModules(cMessage *msg) {

    unsigned int i;
    NA_HackedModule *modHacked;

    for (i = 0; i < modList.size(); i++) {
        LOG << "Module: " << modList[i]->getFullPath() << "\n";
        if (modList[i]->isSimple()) { // Activation is only done in simple modules (implemented in C++ classes).

            modHacked = dynamic_cast<NA_HackedModule *>(modList[i]);

            LOG << "--> Sending message: " << msg->getFullName() << "\n";
            // Send the message to the specific hacked module
            modHacked->handleMessageFromAttackController(msg);
        } else {
            LOG << "--> Message not sent. Not a simple module.\n";
        }
    }
    LOG << "-----------------------------------\n";
}

cMessage *NA_Attack::generateAttackMessage(const char *name) {

    LOG << "ERROR: EN NA_ATTACK GENERATE ATTACK MESSAGE\n";
    cMessage *msg = new cMessage(name);
    return msg;
}


}
}
