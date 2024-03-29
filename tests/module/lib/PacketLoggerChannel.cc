//
// Copyright (C) 2013 OpenSim Ltd.
// @author: Zoltan Bojthe
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


#include <fstream>

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {

class INET_API PacketLoggerChannel : public cDatarateChannel
{
  protected:
    long int counter;
    std::ofstream logfile;

  public:
    explicit PacketLoggerChannel(const char *name = NULL) : cDatarateChannel(name) { counter = 0; }
#if OMNETPP_BUILDNUM  < 1503
    virtual void processMessage(cMessage *msg, simtime_t t, result_t& result);
#else
    virtual cChannel::Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;
#endif

  protected:
    virtual void initialize() override;
    virtual void finish() override;
};


Register_Class(PacketLoggerChannel);

void PacketLoggerChannel::initialize()
{
    EV << "PacketLogger initialize()\n";
    cDatarateChannel::initialize();
    const char *logfilename = par("logfile");
    if (*logfilename)
    {
        logfile.open(logfilename, std::ios::out | std::ios::trunc);
        if (!logfile.is_open())
            throw cRuntimeError("logfile '%s' open failed", logfilename);
    }
    counter = 0;
}

#if OMNETPP_BUILDNUM  < 1503
void PacketLoggerChannel::processMessage(cMessage *msg, simtime_t t, result_t& result)
{
    EV << "PacketLogger processMessage()\n";
    cDatarateChannel::processMessage(msg, t, result);

    counter++;
    if (logfile.is_open())
    {
        logfile << '#' << counter << ':' << t.raw() << ": '" << msg->getName() << "' (" << msg->getClassName() << ") sent:" << msg->getSendingTime().raw();
        cPacket* pk = dynamic_cast<cPacket *>(msg);
        if (pk)
            logfile << " (" << pk->getByteLength() << " byte)";
        logfile << " discard:" << result.discard << ", delay:" << result.delay.raw() << ", duration:" << result.duration.raw();
        logfile << endl;
    }
}
#else
cChannel::Result PacketLoggerChannel::processMessage(cMessage *msg, const SendOptions& options, simtime_t t)
{
    EV << "PacketLogger processMessage()\n";
    Result result = cDatarateChannel::processMessage(msg, options, t);

    counter++;
    if (logfile.is_open())
    {
        logfile << '#' << counter << ':' << t.raw() << ": '" << msg->getName() << "' (" << msg->getClassName() << ") sent:" << msg->getSendingTime().raw();
        cPacket* pk = dynamic_cast<cPacket *>(msg);
        if (pk)
            logfile << " (" << pk->getByteLength() << " byte)";
        logfile << " discard:" << result.discard << ", delay:" << result.delay.raw() << ", duration:" << result.duration.raw();
        logfile << endl;
    }
    return result;
}
#endif

void PacketLoggerChannel::finish()
{
    EV << "PacketLogger finish()\n";
    logfile.close();
}

} // namespace inet

