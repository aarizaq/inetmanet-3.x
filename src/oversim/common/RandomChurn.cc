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
 * @file RandomChurn.cc
 * @author Helge Backhaus
 */

#include <assert.h>

#include <TransportAddress.h>
#include <GlobalStatistics.h>
#include <GlobalStatisticsAccess.h>
#include <UnderlayConfigurator.h>

#include "RandomChurn.h"

Define_Module(RandomChurn);

void RandomChurn::initializeChurn()
{
    Enter_Method_Silent();

    creationProbability = par("creationProbability");
    migrationProbability = par("migrationProbability");
    removalProbability = par("removalProbability");
    initialMean = par("initPhaseCreationInterval");
    initialDeviation = initialMean / 3;
    targetMean = par("targetMobilityDelay");
    targetOverlayTerminalNum = par("targetOverlayTerminalNum");
    WATCH(targetMean);

    churnTimer = NULL;
    churnIntervalChanged = false;
    churnChangeInterval = par("churnChangeInterval");
    initAddMoreTerminals = true;

    globalStatistics = GlobalStatisticsAccess().get();

    // initialize simulation
    mobilityTimer = NULL;
    mobilityTimer = new cMessage("mobilityTimer");
    scheduleAt(simTime(), mobilityTimer);

    if (churnChangeInterval > 0) {
        churnTimer = new cMessage("churnTimer");
        scheduleAt(simTime() + churnChangeInterval, churnTimer);
    }
}

void RandomChurn::handleMessage(cMessage* msg)
{
    if (!msg->isSelfMessage()) {
        delete msg;
        return;
    }

    if (msg == churnTimer) {
        cancelEvent(churnTimer);
        scheduleAt(simTime() + churnChangeInterval, churnTimer);
        if (churnIntervalChanged) {
            targetMean = par("targetMobilityDelay");
            churnIntervalChanged = false;
        }
        else {
            targetMean = par("targetMobilityDelay2");
            churnIntervalChanged = true;
        }
        std::stringstream temp;
        temp << "Churn-rate changed to " << targetMean;
        bubble(temp.str().c_str());
    } else if (msg == mobilityTimer) {
        if (initAddMoreTerminals) {
            // increase the number of nodes steadily during setup
            if (terminalCount < targetOverlayTerminalNum) {
                TransportAddress* ta = underlayConfigurator->createNode(type);
                delete ta; // Address not needed in this churn model
            }

            if (terminalCount >= targetOverlayTerminalNum) {
                initAddMoreTerminals = false;
                underlayConfigurator->initFinished();
            }
            scheduleAt(simTime()
                       + truncnormal(initialMean, initialDeviation), msg);
        }
        else {
            double random = uniform(0, 1);

            // modify the number of nodes according to user parameters
            if (random < creationProbability) {
                TransportAddress* ta = underlayConfigurator->createNode(type);
                delete ta; // Address not needed in this churn model
            } else if (creationProbability <= random &&
                    random < creationProbability + removalProbability &&
                    terminalCount > 1) {
                int oldTerminalCount = terminalCount;
                underlayConfigurator->preKillNode(type);
                assert ((oldTerminalCount - 1) == terminalCount);
            } else if (creationProbability + removalProbability <= random &&
                    random < creationProbability + removalProbability
                    + migrationProbability && terminalCount > 1) {
                underlayConfigurator->migrateNode(type);
            }
            scheduleAt(simTime()
                       + truncnormal(targetMean, targetMean / 3), msg);
        }
    }
}

void RandomChurn::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "random churn");
    getDisplayString().setTagArg("t", 0, buf);
}

RandomChurn::~RandomChurn() {
    // destroy self timer messages
    cancelAndDelete(churnTimer);
    cancelAndDelete(mobilityTimer);
}

