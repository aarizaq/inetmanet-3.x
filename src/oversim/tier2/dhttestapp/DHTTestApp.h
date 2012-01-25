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
 * @file DHTTestApp.h
 * @author Ingmar Baumgart
 */

#ifndef __DHTTESTAPP_H_
#define __DHTTESTAPP_H_

#include <omnetpp.h>

#include <GlobalNodeList.h>
#include <GlobalStatistics.h>
#include <UnderlayConfigurator.h>
#include <TransportAddress.h>
#include <OverlayKey.h>
#include <InitStages.h>
#include <BinaryValue.h>
#include <BaseApp.h>
#include <set>
#include <sstream>

class GlobalDhtTestMap;

/**
 * A simple test application for the DHT layer
 *
 * A simple test application that does random put and get calls
 * on the DHT layer
 *
 * @author Ingmar Baumgart
 */
class DHTTestApp : public BaseApp
{
private:
    /**
     * A container used by the DHTTestApp to
     * store context information for statistics
     *
     * @author Ingmar Baumgart
     */
    class DHTStatsContext : public cPolymorphic
    {
    public:
        bool measurementPhase;
        simtime_t requestTime;
        OverlayKey key;
        BinaryValue value;

        DHTStatsContext(bool measurementPhase,
                        simtime_t requestTime,
                        const OverlayKey& key,
                        const BinaryValue& value = BinaryValue::UNSPECIFIED_VALUE) :
            measurementPhase(measurementPhase), requestTime(requestTime),
            key(key), value(value) {};
    };

    void initializeApp(int stage);

    /**
     * Get a random key of the hashmap
     */
    OverlayKey getRandomKey();

    /**
     * generate a random human readable binary value
     */
    BinaryValue generateRandomValue();

    void finishApp();

    /**
     * processes get responses
     *
     * method to handle get responses
     * should be overwritten in derived application if needed
     * @param msg get response message
     * @param context context object used for collecting statistics
     */
    virtual void handleGetResponse(DHTgetCAPIResponse* msg,
                                   DHTStatsContext* context);

    /**
     * processes put responses
     *
     * method to handle put responses
     * should be overwritten in derived application if needed
     * @param msg put response message
     * @param context context object used for collecting statistics
     */
    virtual void handlePutResponse(DHTputCAPIResponse* msg,
                                   DHTStatsContext* context);

    /**
     * processes self-messages
     *
     * method to handle self-messages
     * should be overwritten in derived application if needed
     * @param msg self-message
     */
    virtual void handleTimerEvent(cMessage* msg);

    /**
     * handleTraceMessage gets called of handleMessage(cMessage* msg)
     * if a message arrives at trace_in. The command included in this
     * message should be parsed and handled.
     *
     * @param msg the command message to handle
     */
    virtual void handleTraceMessage(cMessage* msg);

    virtual void handleNodeLeaveNotification();

    // see RpcListener.h
    void handleRpcResponse(BaseResponseMessage* msg, const RpcState& state,
                           simtime_t rtt);

    UnderlayConfigurator* underlayConfigurator; /**< pointer to UnderlayConfigurator in this node */

    GlobalNodeList* globalNodeList; /**< pointer to GlobalNodeList in this node*/

    GlobalStatistics* globalStatistics; /**< pointer to GlobalStatistics module in this node*/
    GlobalDhtTestMap* globalDhtTestMap; /**< pointer to the GlobalDhtTestMap module */

    // parameters
    bool debugOutput; /**< debug output yes/no?*/
    double mean; //!< mean time interval between sending test messages
    double deviation; //!< deviation of time interval
    int ttl; /**< ttl for stored DHT records */
    bool p2pnsTraffic; //!< model p2pns application traffic */
    bool activeNetwInitPhase; //!< is app active in network init phase?

    // statistics
    int numSent; /**< number of sent packets*/
    int numGetSent; /**< number of get sent*/
    int numGetError; /**< number of false get responses*/
    int numGetSuccess; /**< number of false get responses*/
    int numPutSent; /**< number of put sent*/
    int numPutError; /**< number of error in put responses*/
    int numPutSuccess; /**< number of success in put responses*/

    cMessage *dhttestput_timer, *dhttestget_timer, *dhttestmod_timer;
    bool nodeIsLeavingSoon; //!< true if the node is going to be killed shortly

    static const int DHTTESTAPP_VALUE_LEN = 20;

public:
    DHTTestApp();

    /**
     * virtual destructor
     */
    virtual ~DHTTestApp();

};

#endif
