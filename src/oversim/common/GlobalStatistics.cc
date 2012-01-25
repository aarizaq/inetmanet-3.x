//
// Copyright (C) 2007 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file GlobalStatistics.cc
 * @author IngmarBaumgart
 */

#include <omnetpp.h>

#include "GlobalStatistics.h"

Define_Module(GlobalStatistics);

using namespace std;

const double GlobalStatistics::MIN_MEASURED = 0.1;

void GlobalStatistics::initialize()
{
    sentKBRTestAppMessages = 0;
    deliveredKBRTestAppMessages = 0;

    measuring = par("measureNetwInitPhase");
    measureStartTime = 0;

    currentDeliveryVector.setName("Current Delivery Ratio");

    // start periodic globalStatTimer
    globalStatTimerInterval = par("globalStatTimerInterval");

    if (globalStatTimerInterval > 0) {
        globalStatTimer = new cMessage("globalStatTimer");
        scheduleAt(simTime() + globalStatTimerInterval, globalStatTimer);
    }

    WATCH(measuring);
    WATCH(measureStartTime);
    WATCH(currentDeliveryVector);
}

void GlobalStatistics::startMeasuring()
{
    if (!measuring) {
        measuring = true;
        measureStartTime = simTime();
    }
}


void GlobalStatistics::handleMessage(cMessage* msg)
{
    if (msg == globalStatTimer) {
        // schedule next timer event
        scheduleAt(simTime() + globalStatTimerInterval, msg);

        double ratio;

        // quick hack for live display of the current KBR delivery ratio
        if (sentKBRTestAppMessages == 0) {
            ratio = 0;
        } else {
            ratio = (double)deliveredKBRTestAppMessages /
            (double)sentKBRTestAppMessages;
        }

        if (ratio > 1) ratio = 1;

        currentDeliveryVector.record(ratio);
        sentKBRTestAppMessages = 0;
        deliveredKBRTestAppMessages = 0;

        return;
    }

    error("GlobalStatistics::handleMessage(): Unknown message type!");
}

void GlobalStatistics::finish()
{
    // Here, the FinisherModule is created which will get destroyed at last.
    // This way, all other modules have sent their statistical data to the
    // GobalStatisticModule before GlobalStatistics::finalizeStatistics()
    // is called by FinisherModule::finish()
    cModuleType* moduleType = cModuleType::get("oversim.common.FinisherModule");
    moduleType->create("finisherModule", getParentModule()->getParentModule());
}


void GlobalStatistics::finalizeStatistics()
{
    recordScalar("GlobalStatistics: Simulation Time", simTime());

    bool outputMinMax = par("outputMinMax");
    bool outputStdDev = par("outputStdDev");

    // record stats from other modules
    for (map<std::string, cStdDev*>::iterator iter = stdDevMap.begin();
            iter != stdDevMap.end(); iter++) {

        const std::string& n = iter->first;
        const cStatistic& stat = *(iter->second);

        recordScalar((n + ".mean").c_str(), stat.getMean());

        if (outputStdDev)
            recordScalar((n + ".stddev").c_str(), stat.getStddev());

        if (outputMinMax) {
            recordScalar((n + ".min").c_str(), stat.getMin());
            recordScalar((n + ".max").c_str(), stat.getMax());
        }
    }

    for (map<std::string, cHistogram*>::iterator iter = histogramMap.begin();
            iter != histogramMap.end(); iter++) {
        const std::string& n = iter->first;
        recordStatistic(n.c_str(), iter->second);
    }

    for (map<std::string, OutVector*>::iterator iter = outVectorMap.begin();
    iter != outVectorMap.end(); iter++) {
        const OutVector& ov = *(iter->second);
        double mean = ov.count > 0 ? ov.value / ov.count : 0;
        recordScalar(("Vector: " + iter->first + ".mean").c_str(), mean);
    }
}

void GlobalStatistics::addStdDev(const std::string& name, double value)
{
    if (!measuring) {
        return;
    }

    std::map<std::string, cStdDev*>::iterator sdPos = stdDevMap.find(name);
    cStdDev* sd = NULL;

    if (sdPos == stdDevMap.end()) {
        Enter_Method_Silent();
        sd = new cStdDev(name.c_str());
        stdDevMap.insert(pair<std::string, cStdDev*>(name, sd));
    } else {
        sd = sdPos->second;
    }

    sd->collect(value);
}

void GlobalStatistics::recordHistogram(const std::string& name, double value)
{
    if (!measuring) {
        return;
    }

    std::map<std::string, cHistogram*>::iterator hPos = histogramMap.find(name);
    cHistogram* h = NULL;

    if (hPos == histogramMap.end()) {
        Enter_Method_Silent();
        h = new cHistogram(name.c_str());
        histogramMap.insert(pair<std::string, cHistogram*>(name, h));
    } else {
        h = hPos->second;
    }

    h->collect(value);
}

void GlobalStatistics::recordOutVector(const std::string& name, double value)
{
    if (!measuring) {
        return;
    }

    std::map<std::string, OutVector*>::iterator ovPos =
        outVectorMap.find(name);
    OutVector* ov = NULL;

    if (ovPos == outVectorMap.end()) {
        Enter_Method_Silent();
        ov = new OutVector(name);
        outVectorMap.insert(pair<std::string, OutVector*>(name, ov));
    } else {
        ov = ovPos->second;
    }

    ov->vector.record(value);
    ov->value += value;
    ov->count++;
}

simtime_t GlobalStatistics::calcMeasuredLifetime(simtime_t creationTime)
{
    return simTime() - ((creationTime > measureStartTime)
            ? creationTime : measureStartTime);
}

GlobalStatistics::~GlobalStatistics()
{
    // deallocate vectors
    for (map<std::string, cStdDev*>::iterator it = stdDevMap.begin();
            it != stdDevMap.end(); it++) {
        delete it->second;
    }
    stdDevMap.clear();

    for (map<std::string, OutVector*>::iterator it = outVectorMap.begin();
            it != outVectorMap.end(); it++) {
        delete it->second;
    }
    outVectorMap.clear();

    for (map<std::string, cHistogram*>::iterator iter = histogramMap.begin();
            iter != histogramMap.end(); iter++) {
        delete iter->second;
    }
    histogramMap.clear();
}

