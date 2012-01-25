//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file UnderlayConfigurator.cc
 * @author Stephan Krause, Bernhard Heep, Ingmar Baumgart
 */

#include <omnetpp.h>
#include <GlobalNodeListAccess.h>
#include <ChurnGeneratorAccess.h>
#include <GlobalStatisticsAccess.h>

#include "UnderlayConfigurator.h"

const int32_t UnderlayConfigurator::NUM_COLORS=8;
const char* UnderlayConfigurator::colorNames[] = {
     "red", "green", "yellow", "brown", "grey", "violet", "pink", "orange"};


UnderlayConfigurator::UnderlayConfigurator()
{
    endSimulationTimer = NULL;
    endSimulationNotificationTimer = NULL;
    endTransitionTimer = NULL;
    initFinishedTime.tv_sec = 0;
    initFinishedTime.tv_usec = 0;
}

UnderlayConfigurator::~UnderlayConfigurator()
{
    cancelAndDelete(endSimulationNotificationTimer);
    cancelAndDelete(endSimulationTimer);
    cancelAndDelete(endTransitionTimer);
}

int UnderlayConfigurator::numInitStages() const
{
    return MAX_STAGE_UNDERLAY + 1;
}

void UnderlayConfigurator::initialize(int stage)
{
    if (stage == MIN_STAGE_UNDERLAY) {
        gracefulLeaveDelay = par("gracefulLeaveDelay");
        gracefulLeaveProbability = par("gracefulLeaveProbability");

        transitionTime = par("transitionTime");
        measurementTime = par("measurementTime");

        globalNodeList = GlobalNodeListAccess().get();
        globalStatistics = GlobalStatisticsAccess().get();

        endSimulationNotificationTimer =
            new cMessage("endSimulationNotificationTimer");
        endSimulationTimer = new cMessage("endSimulationTimer");
        endTransitionTimer = new cMessage("endTransitionTimer");

        gettimeofday(&initStartTime, NULL);
        init = true;
        simulationEndingSoon = false;
        initCounter = 0;

        firstNodeId = -1;
        WATCH(firstNodeId);
        WATCH(overlayTerminalCount);
    }

    if (stage >= MIN_STAGE_UNDERLAY && stage <= MAX_STAGE_UNDERLAY) {
        initializeUnderlay(stage);
    }

    if (stage == MAX_STAGE_UNDERLAY) {
        // Create churn generators
        NodeType t;
        t.typeID = 0;

        std::vector<std::string> churnGeneratorTypes =
            cStringTokenizer(par("churnGeneratorTypes"), " ").asVector();
        std::vector<std::string> terminalTypes =
            cStringTokenizer(par("terminalTypes"), " ").asVector();

        if (terminalTypes.size() != 1
                && churnGeneratorTypes.size() != terminalTypes.size())
        {
            opp_error("UnderlayConfigurator.initialize(): "
                      "terminalTypes size does not match churnGenerator size");
        }

        for (std::vector<std::string>::iterator it =
                churnGeneratorTypes.begin(); it != churnGeneratorTypes.end(); ++it) {

            cModuleType* genType = cModuleType::get(it->c_str());

            if (genType == NULL) {
                throw cRuntimeError((std::string("UnderlayConfigurator::"
                   "initialize(): invalid churn generator: ") + *it).c_str());
            }

            ChurnGenerator* gen = check_and_cast<ChurnGenerator*>
                                (genType->create("churnGenerator",
                                                 getParentModule(),
                                                 t.typeID + 1, t.typeID));

            // check threshold for noChurnThreshold hack
            gen->finalizeParameters();

            if ((*it == "oversim.common.LifetimeChurn" ||
                 *it == "oversim.common.ParetoChurn") &&
                ((double)gen->par("noChurnThreshold") > 0) &&
                ((double)gen->par("lifetimeMean") >=
                 (double)gen->par("noChurnThreshold"))) {
                gen->callFinish();
                gen->deleteModule();
                cModuleType* genType =
                    cModuleType::get("oversim.common.NoChurn");
                gen = check_and_cast<ChurnGenerator*>
                        (genType->create("churnGenerator", getParentModule(),
                                         t.typeID + 1, t.typeID));
                gen->finalizeParameters();
                EV << "[UnderlayConfigurator::initialize()]\n"
                   << "    churnGenerator[" << t.typeID
                   << "]: \"oversim.common.NoChurn\" is used instead of \""
                   << *it << "\"!\n    (lifetimeMean exceeds noChurnThreshold)"
                   << endl;
            }

            // Add it to the list of generators and initialize it
            churnGenerator.push_back(gen);
            t.terminalType = (terminalTypes.size() == 1) ?
                terminalTypes[0] :
                terminalTypes[it - churnGeneratorTypes.begin()];

            gen->setNodeType(t);
            gen->buildInside();
            t.typeID++;
        }
    }
}

void UnderlayConfigurator::initFinished()
{
    Enter_Method_Silent();

    if (++initCounter == churnGenerator.size() || !churnGenerator.size()) {
        init = false;
        gettimeofday(&initFinishedTime, NULL);

        scheduleAt(simTime() + transitionTime,
                endTransitionTimer);

        if (measurementTime >= 0) {
            scheduleAt(simTime() + transitionTime + measurementTime,
                       endSimulationTimer);

            if ((transitionTime + measurementTime) < gracefulLeaveDelay) {
                throw cRuntimeError("UnderlayConfigurator::initFinished():"
                                        " gracefulLeaveDelay must be bigger "
                                        "than transitionTime + measurementTime!");
            }

            scheduleAt(simTime() + transitionTime + measurementTime
                    - gracefulLeaveDelay,
                    endSimulationNotificationTimer);
        }
        consoleOut("INIT phase finished");
    }
}

void UnderlayConfigurator::handleMessage(cMessage* msg)
{
    if (msg == endSimulationNotificationTimer) {
        simulationEndingSoon = true;
        // globalNodeList->sendNotificationToAllPeers(NF_OVERLAY_NODE_LEAVE);
    } else if (msg == endSimulationTimer) {
        endSimulation();
    } else if (msg == endTransitionTimer) {
        consoleOut("transition time finished");
        globalStatistics->startMeasuring();
    } else {
        handleTimerEvent(msg);
    }
}

void UnderlayConfigurator::handleTimerEvent(cMessage* msg)
{
    delete msg;
}

void UnderlayConfigurator::finish()
{
    finishUnderlay();
}

void UnderlayConfigurator::finishUnderlay()
{
    //...
}

void UnderlayConfigurator::consoleOut(const std::string& text)
{
    if (!ev.isGUI()) {
        struct timeval now, diff;
        gettimeofday(&now, NULL);
        diff = timeval_substract(now, initStartTime);

        std::stringstream ss;
        std::string line1(71, '*');
        std::string line2(71, '*');

        ss << "   " << text << "   ";
        line1.replace(35 - ss.str().size() / 2,
                      ss.str().size(),
                      ss.str());
        ss.str("");

        ss << "   (sim time: " << simTime()
           << ", real time: " << diff.tv_sec
           << "." << diff.tv_usec << ")   ";
        line2.replace(35 - ss.str().size() / 2,
                      ss.str().size(),
                      ss.str());

        std::cout << "\n" << line1 << "\n"
                  << line2 << "\n" << std::endl;
    } else {
        EV << "[UnderlayConfigurator::consoleOut()] " << text;
    }
}

ChurnGenerator* UnderlayConfigurator::getChurnGenerator(int typeID)
{
    Enter_Method_Silent();

    return churnGenerator[typeID];
}

uint8_t UnderlayConfigurator::getChurnGeneratorNum()
{
    Enter_Method_Silent();

    return churnGenerator.size();
}
