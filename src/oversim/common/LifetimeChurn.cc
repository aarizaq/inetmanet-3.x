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
 * @file LifetimeChurn.cc
 * @author Helge Backhaus, Ingmar Baumgart
 */

#include <algorithm>

#include "GlobalStatisticsAccess.h"
#include "UnderlayConfigurator.h"
#include "Churn_m.h"

#include "LifetimeChurn.h"

Define_Module(LifetimeChurn);

void LifetimeChurn::initializeChurn()
{
    Enter_Method_Silent();

    initialMean = par("initPhaseCreationInterval");
    initialDeviation = initialMean / 3;
    lifetimeMean = par("lifetimeMean");
    lifetimeDistName = par("lifetimeDistName").stdstringValue();
    lifetimeDistPar1 = par("lifetimeDistPar1");

    WATCH(lifetimeMean);

    globalStatistics = GlobalStatisticsAccess().get();

    lastCreate = lastDelete = simTime();

    simtime_t initFinishedTime = initialMean * targetOverlayTerminalNum;

    // create the remaining nodes in bootstrap phase
    int targetOverlayTerminalNum = par("targetOverlayTerminalNum");
    contextVector.assign(2*targetOverlayTerminalNum, (cObject*)NULL);

    for (int i = 0; i < targetOverlayTerminalNum; i++) {

        scheduleCreateNodeAt(truncnormal(initialMean * i, initialDeviation),
                             initFinishedTime + distributionFunction()
                                     - truncnormal(initialMean * i,
                                                   initialDeviation), i);

        // create same number of currently dead nodes
        scheduleCreateNodeAt(initFinishedTime + distributionFunction(),
                             distributionFunction(), targetOverlayTerminalNum + i);
    }

    initFinishedTimer = new cMessage("initFinishedTimer");

    scheduleAt(initFinishedTime, initFinishedTimer);
}

void LifetimeChurn::handleMessage(cMessage* msg)
{
    if (!msg->isSelfMessage()) {
        delete msg;
        return;
    }

    // init phase finished
    if (msg == initFinishedTimer) {
        underlayConfigurator->initFinished();
        cancelEvent(initFinishedTimer);
        delete initFinishedTimer;
        initFinishedTimer = NULL;
        return;
    }

    ChurnMessage* churnMsg = check_and_cast<ChurnMessage*> (msg);

    if (churnMsg->getCreateNode() == true) {
        createNode(churnMsg->getLifetime(), false, churnMsg->getContextPos());
    } else {
        deleteNode(churnMsg->getAddr(), churnMsg->getContextPos());
    }

    delete msg;
}

void LifetimeChurn::createNode(simtime_t lifetime, bool initialize,
                               int contextPos)
{

    ChurnMessage* churnMsg = new ChurnMessage("DeleteNode");
    NodeType createType = type;
    createType.context = &(contextVector[contextPos]);
    TransportAddress* ta = underlayConfigurator->createNode(createType, initialize);
    churnMsg->setAddr(*ta);
    churnMsg->setContextPos(contextPos);
    delete ta;
    churnMsg->setCreateNode(false);
    scheduleAt(std::max(simTime(), simTime() + lifetime
            - underlayConfigurator->getGracefulLeaveDelay()), churnMsg);

    RECORD_STATS(globalStatistics->recordOutVector(
                    "LifetimeChurn: Session Time", SIMTIME_DBL(lifetime)));
    RECORD_STATS(globalStatistics->recordOutVector(
                    "LifetimeChurn: Time between creates",
                    SIMTIME_DBL(simTime() - lastCreate)));

    lastCreate = simTime();
}

void LifetimeChurn::deleteNode(TransportAddress& addr, int contextPos)
{
    underlayConfigurator->preKillNode(NodeType(), &addr);

    scheduleCreateNodeAt(simTime() + distributionFunction(),
                         distributionFunction(), contextPos);

    RECORD_STATS(globalStatistics->recordOutVector(
                 "LifetimeChurn: Time between deletes",
                 SIMTIME_DBL(simTime() - lastDelete)));

    lastDelete = simTime();
}

void LifetimeChurn::scheduleCreateNodeAt(simtime_t creationTime,
                                         simtime_t lifetime, int contextPos)
{
    ChurnMessage* churnMsg = new ChurnMessage("CreateNode");
    churnMsg->setCreateNode(true);
    churnMsg->setLifetime(SIMTIME_DBL(lifetime));
    churnMsg->setContextPos(contextPos);
    scheduleAt(creationTime, churnMsg);
}

double LifetimeChurn::distributionFunction()
{
    double par;

    if (lifetimeDistName == "weibull") {
        par = lifetimeMean / tgamma(1 + (1 / lifetimeDistPar1));
        return weibull(par, lifetimeDistPar1);
    } else if (lifetimeDistName == "pareto_shifted") {
        par = lifetimeMean * (lifetimeDistPar1 - 1) / lifetimeDistPar1;
        return pareto_shifted(lifetimeDistPar1, par, 0);
    } else if (lifetimeDistName == "truncnormal") {
        par = lifetimeMean;
        return truncnormal(par, par/3.0);
    } else {
        opp_error("LifetimeChurn::distribution function: Invalid value "
            "for parameter lifetimeDistName!");
    }

    return 0;
}

void LifetimeChurn::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "lifetime churn");
    getDisplayString().setTagArg("t", 0, buf);
}

LifetimeChurn::~LifetimeChurn()
{
    cancelAndDelete(initFinishedTimer);
    for (std::vector<cObject*>::iterator it = contextVector.begin();
         it != contextVector.end(); it++) {
        if (*it) {
            delete *it;
        }
    }

}
