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

#include "MyThroughputMeteringChannel.h"
#include <sstream>

Register_Class(MyThroughputMeteringChannel);

MyThroughputMeteringChannel::MyThroughputMeteringChannel(const char *name) : cDatarateChannel(name)
{
    batchSize = 10;    // packets
    maxInterval = 0.1; // seconds

    numPackets = 0;
    numBits = 0;

    intvlStartTime = intvlLastPkTime = 0;
    intvlNumPackets = intvlNumBits = 0;
}

MyThroughputMeteringChannel::~MyThroughputMeteringChannel()
{
}

bool MyThroughputMeteringChannel::initializeChannel(int stage)
{

    cDatarateChannel::initializeChannel(stage);

    const char *fmt = this->par("format");
    char buf[200];
    char *p = buf;
    std::stringstream n;
    for (const char *fp = fmt; *fp && buf+200-p>20; fp++)
    {
        n.str("");
        switch (*fp)
        {
        case 'N': // number of packets
            n << "Number of Packets from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("N",new cOutVector(n.str().c_str())));

            break;
        case 'V': // volume (in bytes)
            n << "Volume from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("V",new cOutVector(n.str().c_str())));
            break;
        case 'p': // current packet/sec
            n << "current packet/sec from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("p",new cOutVector(n.str().c_str())));
            break;
        case 'b': // current bandwidth
            n << "current bandwidth from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("b",new cOutVector(n.str().c_str())));
            break;
        case 'u': // current channel utilization (%)
            n << "current channel utilization (%) from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("u",new cOutVector(n.str().c_str())));
            break;
        case 'P': // average packet/sec on [0,now)
            n << "average packet/sec from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("P",new cOutVector(n.str().c_str())));
            break;
        case 'B': // average bandwidth on [0,now)
            n << "average bandwidth from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("B",new cOutVector(n.str().c_str())));
            break;
        case 'U': // average channel utilization (%) on [0,now)
            n << "average channel utilization (%) from " << this->getSourceGate()->getOwnerModule()->getFullPath();
            this->out_vectors.insert(std::make_pair<const char*,cOutVector*> ("U",new cOutVector(n.str().c_str())));
            break;
        default:
            *p++ = *fp;
        }
    }

    return false;
}

void MyThroughputMeteringChannel::processMessage(cMessage *msg, simtime_t t, result_t& result)
{
    cDatarateChannel::processMessage(msg, t,result);
    if (dynamic_cast<cPacket*>(msg))
    {
        // count packets and bits
        numPackets++;
        numBits += ((cPacket*)msg)->getBitLength();

        // packet should be counted to new interval
        if (intvlNumPackets >= batchSize || t-intvlStartTime >= maxInterval)
            beginNewInterval(t);

        intvlNumPackets++;
        intvlNumBits += ((cPacket*)msg)->getBitLength();
        intvlLastPkTime = t;

        // update display string
        updateDisplay();
    }
}

void MyThroughputMeteringChannel::beginNewInterval(simtime_t now)
{
    simtime_t duration = now - intvlStartTime;

    // record measurements
    currentBitPerSec = intvlNumBits/duration;
    currentPkPerSec = intvlNumPackets/duration;

    // restart counters
    intvlStartTime = now;
    intvlNumPackets = intvlNumBits = 0;
}

void MyThroughputMeteringChannel::updateDisplay()
{
    // retrieve format string
    const char *fmt = this->par("format");

    // produce label, based on format string
    char buf[200];
    char *p = buf;
    simtime_t tt = getTransmissionFinishTime();
    if (tt==0) tt = simTime();
    double bps = (tt==0) ? 0 : numBits/tt;
    double bytes;
    for (const char *fp = fmt; *fp && buf+200-p>20; fp++)
    {
        switch (*fp)
        {
        case 'N': // number of packets
            p += sprintf(p, "%ld", numPackets);
            this->out_vectors["N"]->record(numPackets);
            break;
        case 'V': // volume (in bytes)
            bytes = floor(numBits/8);
            this->out_vectors["V"]->record(bytes);
            if (bytes<1024)
            {
                p += sprintf(p, "%gB", bytes);
            }
            else if (bytes<1024*1024)
            {
                p += sprintf(p, "%.3gKB", bytes/1024);
            }
            else
            {
                p += sprintf(p, "%.3gMB", bytes/1024/1024);
            }
            break;

        case 'p': // current packet/sec
            p += sprintf(p, "%.3gpps", currentPkPerSec);
            this->out_vectors["p"]->record(currentPkPerSec);
            break;
        case 'b': // current bandwidth
            this->out_vectors["b"]->record(currentBitPerSec);
            if (currentBitPerSec<1000000)
                p += sprintf(p, "%.3gk", currentBitPerSec/1000);
            else
                p += sprintf(p, "%.3gM", currentBitPerSec/1000000);
            break;
        case 'u': // current channel utilization (%)
            if (getDatarate()==0)
                p += sprintf(p, "n/a");
            else
            {
                p += sprintf(p, "%.3g%%", currentBitPerSec/getDatarate()*100.0);
                this->out_vectors["u"]->record(currentBitPerSec/getDatarate()*100.0);
            }
            break;

        case 'P': // average packet/sec on [0,now)
            p += sprintf(p, "%.3gpps", tt==0 ? 0 : numPackets/tt);
            this->out_vectors["P"]->record(tt==0 ? 0 : numPackets/tt);
            break;
        case 'B': // average bandwidth on [0,now)
            this->out_vectors["B"]->record(bps);
            if (bps<1000000)
                p += sprintf(p, "%.3gk", bps/1000);
            else
                p += sprintf(p, "%.3gM", bps/1000000);
            break;
        case 'U': // average channel utilization (%) on [0,now)
            if (getDatarate()==0)
                p += sprintf(p, "n/a");
            else
            {
                this->out_vectors["U"]->record(bps/getDatarate()*100.0);
                p += sprintf(p, "%.3g%%", bps/getDatarate()*100.0);
            }
            break;
        default:
            *p++ = *fp;
        }
    }
    *p = '\0';

    // display label
    getSourceGate()->getDisplayString().setTagArg("t", 0, buf);

}

