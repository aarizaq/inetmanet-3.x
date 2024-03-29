//
// Copyright (C) 2004 Andras Varga
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

#ifndef __INET_TCPSEGMENT_H
#define __INET_TCPSEGMENT_H

#include <list>
#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/ITransportPacket.h"
#include "inet/transportlayer/tcp_common/TCPSegment_m.h"

namespace inet {

namespace tcp {

/** @name Comparing sequence numbers */
//@{
inline bool seqLess(uint32 a, uint32 b) { return a != b && (b - a) < (1UL << 31); }
inline bool seqLE(uint32 a, uint32 b) { return (b - a) < (1UL << 31); }
inline bool seqGreater(uint32 a, uint32 b) { return a != b && (a - b) < (1UL << 31); }
inline bool seqGE(uint32 a, uint32 b) { return (a - b) < (1UL << 31); }
inline uint32 seqMin(uint32 a, uint32 b) { return ((b - a) < (1UL << 31)) ? a : b; }
inline uint32 seqMax(uint32 a, uint32 b) { return ((a - b) < (1UL << 31)) ? a : b; }
//@}

/**
 * Represents a TCP segment. More info in the TCPSegment.msg file
 * (and the documentation generated from it).
 */
class INET_API TCPSegment : public TCPSegment_Base, public ITransportPacket
{
  public:
    TCPSegment(const char *name = nullptr, int kind = 0) : TCPSegment_Base(name, kind) {}
    TCPSegment(const TCPSegment& other) : TCPSegment_Base(other) {}
    ~TCPSegment();
    virtual TCPSegment *dup() const override { return new TCPSegment(*this); }

    /**
     * Adds a message object to the TCP segment. The sequence number + 1 of the
     * last byte of the message should be passed as 2nd argument
     */
    virtual void addPayloadMessage(cPacket *msg, uint32 endSequenceNo);

    /**
     * Removes and returns the first message object in this TCP segment.
     * It also returns the sequence number + 1 of its last octet in outEndSequenceNo.
     */
    virtual cPacket *removeFirstPayloadMessage(uint32& outEndSequenceNo);

    /**
     * Returns RFC 793 specified SEG.LEN:
     *     SEG.LEN = the number of octets occupied by the data in the segment
     *               (counting SYN and FIN)
     *
     */
    virtual uint32_t getSegLen();

    /**
     * Truncate segment.
     * @param firstSeqNo: sequence no of new first byte
     * @param endSeqNo: sequence no of new last byte + 1
     */
    virtual void truncateSegment(uint32 firstSeqNo, uint32 endSeqNo);

    // manage header options:

    /** Calculate Length of TCP Options Array in bytes */
    virtual unsigned short getHeaderOptionArrayLength();

    /** Adds a TCP option to the TCP segment */
    virtual void addHeaderOption(TCPOption *headerOption);

    /** Drops all TCP options of the TCP segment */
    virtual void dropHeaderOptions();

    virtual unsigned int getSourcePort() const override { return TCPSegment_Base::getSrcPort(); }
    virtual void setSourcePort(unsigned int port) override { TCPSegment_Base::setSrcPort(port); }
    virtual unsigned int getDestinationPort() const override { return TCPSegment_Base::getDestPort(); }
    virtual void setDestinationPort(unsigned int port) override { TCPSegment_Base::setDestPort(port); }

  protected:
    /**
     * Truncate segment data. Called from truncateSegment().
     * @param truncleft: number of bytes for truncate from begin of data
     * @param truncright: number of bytes for truncate from end of data
     */
    virtual void truncateData(unsigned int truncleft, unsigned int truncright);
};

} // namespace tcp

} // namespace inet

#endif // ifndef __INET_TCPSEGMENT_H

