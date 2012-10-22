//
// Copyright (C) 2005 Andras Varga
// Copyright (C) 2009 Juan-Carlos Maureira
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MYTHROUGHPUTMETERINGCHANNEL_H
#define __INET_MYTHROUGHPUTMETERINGCHANNEL_H

#include <omnetpp.h>
#include <map>

/**
 * A cDatarateChannel extended with throughput calculation. Values
 * get displayed on the link, using the connection's "t=" display
 * string tag.
 *
 * The display can be customised with the "format" attribute.
 * In the format string, the following characters will get expanded:
 *   - 'N': number of packets
 *   - 'V': volume (in bytes)
 *   - 'p': current packet/sec
 *   - 'b': current bandwidth
 *   - 'u': current channel utilization (%)
 *   - 'P': average packet/sec on [0,now)
 *   - 'B': average bandwidth on [0,now)
 *   - 'U': average channel utilization (%) on [0,now)
 * Other characters are copied verbatim.
 *
 * "Current" actually means the last measurement interval, which is
 * 10 packets or 0.1s, whichever comes first.
 *
 * PROBLEM: display only gets updated if there's traffic! (For example, a
 * high pk/sec value might stay displayed even when the link becomes idle!)
 */
class SIM_API MyThroughputMeteringChannel : public cDatarateChannel
{
  protected:
    // configuration
    unsigned int batchSize; // number of packets in a batch
    simtime_t maxInterval; // max length of measurement interval (measurement ends
    // if either batchSize or maxInterval is reached, whichever
    // is reached first)

    // global statistics
    long numPackets;
    double numBits; // double to avoid overflow

    int sense;

    // current measurement interval
    simtime_t intvlStartTime;
    simtime_t intvlLastPkTime;
    unsigned long intvlNumPackets;
    unsigned long intvlNumBits;

    // reading from last interval
    double currentBitPerSec;
    double currentPkPerSec;

    std::map<const char*,cOutVector*> out_vectors;

  protected:
    virtual void beginNewInterval(simtime_t now);
    virtual void updateDisplay();

  public:
    /**
     * Constructor.
     */
    explicit MyThroughputMeteringChannel(const char *name=NULL);

    /**
     * Copy constructor.
     */
    MyThroughputMeteringChannel(const MyThroughputMeteringChannel& ch);

    /**
     * Destructor.
     */
    virtual ~MyThroughputMeteringChannel();

    /**
     * Performs bit error rate, delay and transmission time modeling.
     */
    virtual void processMessage(cMessage *msg, simtime_t t, result_t& result);

    /**
     * Initialize Channel
     */
    virtual bool initializeChannel(int stage);

};

#endif
