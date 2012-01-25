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
 * @file DHTTestApp.cc
 * @author Ingmar Baumgart
 */

#include <GlobalNodeListAccess.h>
#include <GlobalStatisticsAccess.h>
#include <UnderlayConfiguratorAccess.h>
#include <RpcMacros.h>
#include "CommonMessages_m.h"

#include <GlobalDhtTestMap.h>

#include "DHTTestApp.h"

Define_Module(DHTTestApp);

using namespace std;

DHTTestApp::~DHTTestApp()
{
    cancelAndDelete(dhttestput_timer);
    cancelAndDelete(dhttestget_timer);
    cancelAndDelete(dhttestmod_timer);
}

DHTTestApp::DHTTestApp()
{
    dhttestput_timer = NULL;
    dhttestget_timer = NULL;
    dhttestmod_timer = NULL;
}

void DHTTestApp::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    // fetch parameters
    debugOutput = par("debugOutput");
    activeNetwInitPhase = par("activeNetwInitPhase");

    mean = par("testInterval");
    p2pnsTraffic = par("p2pnsTraffic");
    deviation = mean / 10;

    if (p2pnsTraffic) {
        ttl = 3600*24*365;
    } else {
        ttl = par("testTtl");
    }

    globalNodeList = GlobalNodeListAccess().get();
    underlayConfigurator = UnderlayConfiguratorAccess().get();
    globalStatistics = GlobalStatisticsAccess().get();

    globalDhtTestMap = dynamic_cast<GlobalDhtTestMap*>(simulation.getModuleByPath(
            "globalObserver.globalFunctions[0].function"));

    if (globalDhtTestMap == NULL) {
        throw cRuntimeError("DHTTestApp::initializeApp(): "
                                "GlobalDhtTestMap module not found!");
    }

    // statistics
    numSent = 0;
    numGetSent = 0;
    numGetError = 0;
    numGetSuccess = 0;
    numPutSent = 0;
    numPutError = 0;
    numPutSuccess = 0;

    //initRpcs();
    WATCH(numSent);
    WATCH(numGetSent);
    WATCH(numGetError);
    WATCH(numGetSuccess);
    WATCH(numPutSent);
    WATCH(numPutError);
    WATCH(numPutSuccess);

    nodeIsLeavingSoon = false;

    // initiate test message transmission
    dhttestput_timer = new cMessage("dhttest_put_timer");
    dhttestget_timer = new cMessage("dhttest_get_timer");
    dhttestmod_timer = new cMessage("dhttest_mod_timer");

    if (mean > 0) {
        scheduleAt(simTime() + truncnormal(mean, deviation),
                   dhttestput_timer);
        scheduleAt(simTime() + truncnormal(mean + mean / 3,
                                                      deviation),
                                                      dhttestget_timer);
        scheduleAt(simTime() + truncnormal(mean + 2 * mean / 3,
                                                      deviation),
                                                      dhttestmod_timer);
    }
}

void DHTTestApp::handleRpcResponse(BaseResponseMessage* msg,
                                   const RpcState& state, simtime_t rtt)
{
    RPC_SWITCH_START(msg)
    RPC_ON_RESPONSE( DHTputCAPI ) {
        handlePutResponse(_DHTputCAPIResponse,
                          check_and_cast<DHTStatsContext*>(state.getContext()));
        EV << "[DHTTestApp::handleRpcResponse()]\n"
           << "    DHT Put RPC Response received: id=" << state.getId()
           << " msg=" << *_DHTputCAPIResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_ON_RESPONSE(DHTgetCAPI)
    {
        handleGetResponse(_DHTgetCAPIResponse,
                          check_and_cast<DHTStatsContext*>(state.getContext()));
        EV << "[DHTTestApp::handleRpcResponse()]\n"
           << "    DHT Get RPC Response received: id=" << state.getId()
           << " msg=" << *_DHTgetCAPIResponse << " rtt=" << rtt
           << endl;
        break;
    }
    RPC_SWITCH_END()
}

void DHTTestApp::handlePutResponse(DHTputCAPIResponse* msg,
                                   DHTStatsContext* context)
{
    DHTEntry entry = {context->value, simTime() + ttl};

    globalDhtTestMap->insertEntry(context->key, entry);

    if (context->measurementPhase == false) {
        // don't count response, if the request was not sent
        // in the measurement phase
        delete context;
        return;
    }

    if (msg->getIsSuccess()) {
        RECORD_STATS(numPutSuccess++);
        RECORD_STATS(globalStatistics->addStdDev("DHTTestApp: PUT Latency (s)",
                               SIMTIME_DBL(simTime() - context->requestTime)));
    } else {
        RECORD_STATS(numPutError++);
    }

    delete context;
}

void DHTTestApp::handleGetResponse(DHTgetCAPIResponse* msg,
                                   DHTStatsContext* context)
{
    if (context->measurementPhase == false) {
        // don't count response, if the request was not sent
        // in the measurement phase
        delete context;
        return;
    }

    RECORD_STATS(globalStatistics->addStdDev("DHTTestApp: GET Latency (s)",
                               SIMTIME_DBL(simTime() - context->requestTime)));

    if (!(msg->getIsSuccess())) {
        //cout << "DHTTestApp: success == false" << endl;
        RECORD_STATS(numGetError++);
        delete context;
        return;
    }

    const DHTEntry* entry = globalDhtTestMap->findEntry(context->key);

    if (entry == NULL) {
        //unexpected key
        RECORD_STATS(numGetError++);
        //cout << "DHTTestApp: unexpected key" << endl;
        delete context;
        return;
    }

    if (simTime() > entry->endtime) {
        //this key doesn't exist anymore in the DHT, delete it in our hashtable

        globalDhtTestMap->eraseEntry(context->key);
        delete context;

        if (msg->getResultArraySize() > 0) {
            RECORD_STATS(numGetError++);
            //cout << "DHTTestApp: deleted key still available" << endl;
            return;
        } else {
            RECORD_STATS(numGetSuccess++);
            //cout << "DHTTestApp: success (1)" << endl;
            return;
        }
    } else {
        delete context;
        if ((msg->getResultArraySize() > 0) && (msg->getResult(0).getValue() != entry->value)) {
            RECORD_STATS(numGetError++);
            //cout << "DHTTestApp: wrong value" << endl;
            //cout << "value: " << msg->getResult(0).getValue() << endl;
            return;
        } else {
            RECORD_STATS(numGetSuccess++);
            //cout << "DHTTestApp: success (2)" << endl;
            return;
        }
    }

}

void DHTTestApp::handleTraceMessage(cMessage* msg)
{
    char* cmd = new char[strlen(msg->getName()) + 1];
    strcpy(cmd, msg->getName());

    if (strlen(msg->getName()) < 5) {
        delete[] cmd;
        delete msg;
        return;
    }

    if (strncmp(cmd, "PUT ", 4) == 0) {
        // Generate key
        char* buf = cmd + 4;

        while (!isspace(buf[0])) {
            if (buf[0] == '\0')
                throw cRuntimeError("Error parsing PUT command");
            buf++;
        }

        buf[0] = '\0';
        BinaryValue b(cmd + 4);
        OverlayKey destKey(OverlayKey::sha1(b));

        // get value
        buf++;

        // build putMsg
        DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
        dhtPutMsg->setKey(destKey);
        dhtPutMsg->setValue(buf);
        dhtPutMsg->setTtl(ttl);
        dhtPutMsg->setIsModifiable(true);
        RECORD_STATS(numSent++; numPutSent++);
        sendInternalRpcCall(TIER1_COMP, dhtPutMsg,
                new DHTStatsContext(globalStatistics->isMeasuring(),
                                    simTime(), destKey));
    } else if (strncmp(cmd, "GET ", 4) == 0) {
        // Get key
        BinaryValue b(cmd + 4);
        OverlayKey key(OverlayKey::sha1(b));

        DHTgetCAPICall* dhtGetMsg = new DHTgetCAPICall();
        dhtGetMsg->setKey(key);
        RECORD_STATS(numSent++; numGetSent++);
        sendInternalRpcCall(TIER1_COMP, dhtGetMsg,
                new DHTStatsContext(globalStatistics->isMeasuring(),
                                    simTime(), key));
    } else {
        throw cRuntimeError("Unknown trace command; "
                                "only GET and PUT are allowed");
    }

    delete[] cmd;
    delete msg;
}

void DHTTestApp::handleTimerEvent(cMessage* msg)
{
    if (msg->isName("dhttest_put_timer")) {
        // schedule next timer event
        scheduleAt(simTime() + truncnormal(mean, deviation), msg);

        // do nothing if the network is still in the initialization phase
        if (((!activeNetwInitPhase) && (underlayConfigurator->isInInitPhase()))
                || underlayConfigurator->isSimulationEndingSoon()
                || nodeIsLeavingSoon)
            return;

        if (p2pnsTraffic) {
            if (globalDhtTestMap->p2pnsNameCount < 4*globalNodeList->getNumNodes()) {
                for (int i = 0; i < 4; i++) {
                    // create a put test message with random destination key
                    OverlayKey destKey = OverlayKey::random();
                    DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
                    dhtPutMsg->setKey(destKey);
                    dhtPutMsg->setValue(generateRandomValue());
                    dhtPutMsg->setTtl(ttl);
                    dhtPutMsg->setIsModifiable(true);

                    RECORD_STATS(numSent++; numPutSent++);
                    sendInternalRpcCall(TIER1_COMP, dhtPutMsg,
                            new DHTStatsContext(globalStatistics->isMeasuring(),
                                                simTime(), destKey, dhtPutMsg->getValue()));
                    globalDhtTestMap->p2pnsNameCount++;
                }
            }
            cancelEvent(msg);
            return;
        }

        // create a put test message with random destination key
        OverlayKey destKey = OverlayKey::random();
        DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
        dhtPutMsg->setKey(destKey);
        dhtPutMsg->setValue(generateRandomValue());
        dhtPutMsg->setTtl(ttl);
        dhtPutMsg->setIsModifiable(true);

        RECORD_STATS(numSent++; numPutSent++);
        sendInternalRpcCall(TIER1_COMP, dhtPutMsg,
                new DHTStatsContext(globalStatistics->isMeasuring(),
                                    simTime(), destKey, dhtPutMsg->getValue()));
    } else if (msg->isName("dhttest_get_timer")) {
        scheduleAt(simTime() + truncnormal(mean, deviation), msg);

        // do nothing if the network is still in the initialization phase
        if (((!activeNetwInitPhase) && (underlayConfigurator->isInInitPhase()))
                || underlayConfigurator->isSimulationEndingSoon()
                || nodeIsLeavingSoon) {
            return;
        }

        if (p2pnsTraffic && (uniform(0, 1) > ((double)mean/1800.0))) {
            return;
        }

        const OverlayKey& key = globalDhtTestMap->getRandomKey();

        if (key.isUnspecified()) {
            EV << "[DHTTestApp::handleTimerEvent() @ " << thisNode.getIp()
               << " (" << thisNode.getKey().toString(16) << ")]\n"
               << "    Error: No key available in global DHT test map!"
               << endl;
            return;
        }

        DHTgetCAPICall* dhtGetMsg = new DHTgetCAPICall();
        dhtGetMsg->setKey(key);
        RECORD_STATS(numSent++; numGetSent++);

        sendInternalRpcCall(TIER1_COMP, dhtGetMsg,
                new DHTStatsContext(globalStatistics->isMeasuring(),
                                    simTime(), key));
    } else if (msg->isName("dhttest_mod_timer")) {
        scheduleAt(simTime() + truncnormal(mean, deviation), msg);

        // do nothing if the network is still in the initialization phase
        if (((!activeNetwInitPhase) && (underlayConfigurator->isInInitPhase()))
                || underlayConfigurator->isSimulationEndingSoon()
                || nodeIsLeavingSoon) {
            return;
        }

        if (p2pnsTraffic) {
            if (globalDhtTestMap->p2pnsNameCount >= 4*globalNodeList->getNumNodes()) {
                const OverlayKey& key = globalDhtTestMap->getRandomKey();

                if (key.isUnspecified())
                    return;

                DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
                dhtPutMsg->setKey(key);
                dhtPutMsg->setValue(generateRandomValue());
                dhtPutMsg->setTtl(ttl);
                dhtPutMsg->setIsModifiable(true);

                RECORD_STATS(numSent++; numPutSent++);
                sendInternalRpcCall(TIER1_COMP, dhtPutMsg,
                        new DHTStatsContext(globalStatistics->isMeasuring(),
                                            simTime(), key, dhtPutMsg->getValue()));
            }
            cancelEvent(msg);
            return;
        }

        const OverlayKey& key = globalDhtTestMap->getRandomKey();

        if (key.isUnspecified())
            return;

        DHTputCAPICall* dhtPutMsg = new DHTputCAPICall();
        dhtPutMsg->setKey(key);
        dhtPutMsg->setValue(generateRandomValue());
        dhtPutMsg->setTtl(ttl);
        dhtPutMsg->setIsModifiable(true);

        RECORD_STATS(numSent++; numPutSent++);
        sendInternalRpcCall(TIER1_COMP, dhtPutMsg,
                new DHTStatsContext(globalStatistics->isMeasuring(),
                                    simTime(), key, dhtPutMsg->getValue()));
    }
}


BinaryValue DHTTestApp::generateRandomValue()
{
    char value[DHTTESTAPP_VALUE_LEN + 1];

    for (int i = 0; i < DHTTESTAPP_VALUE_LEN; i++) {
        value[i] = intuniform(0, 25) + 'a';
    }

    value[DHTTESTAPP_VALUE_LEN] = '\0';
    return BinaryValue(value);
}

void DHTTestApp::handleNodeLeaveNotification()
{
    nodeIsLeavingSoon = true;
}

void DHTTestApp::finishApp()
{
    simtime_t time = globalStatistics->calcMeasuredLifetime(creationTime);

    if (time >= GlobalStatistics::MIN_MEASURED) {
        // record scalar data
        globalStatistics->addStdDev("DHTTestApp: Sent Total Messages/s",
                                    numSent / time);
        globalStatistics->addStdDev("DHTTestApp: Sent GET Messages/s",
                                    numGetSent / time);
        globalStatistics->addStdDev("DHTTestApp: Failed GET Requests/s",
                                    numGetError / time);
        globalStatistics->addStdDev("DHTTestApp: Successful GET Requests/s",
                                    numGetSuccess / time);

        globalStatistics->addStdDev("DHTTestApp: Sent PUT Messages/s",
                                    numPutSent / time);
        globalStatistics->addStdDev("DHTTestApp: Failed PUT Requests/s",
                                    numPutError / time);
        globalStatistics->addStdDev("DHTTestApp: Successful PUT Requests/s",
                                    numPutSuccess / time);

        if ((numGetSuccess + numGetError) > 0) {
            globalStatistics->addStdDev("DHTTestApp: GET Success Ratio",
                                        (double) numGetSuccess
                                        / (double) (numGetSuccess + numGetError));
        }
    }
}

