//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
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
 * @file SimpleNodeEntry.h
 * @author Bernhard Heep
 * @date 2006/11/30
 */

#ifndef __SIMPLENODEENTRY_H
#define __SIMPLENODEENTRY_H


#include <omnetpp.h>
#include <IPvXAddress.h>

#include "UDPPacket_m.h"
#include "TCPSegment.h"


class NodeRecord
{
  public:
    //NodeRecord(uint32_t dim);
    NodeRecord();
    ~NodeRecord();
    NodeRecord(const NodeRecord& nodeRecord);
    NodeRecord& operator=(const NodeRecord& nodeRecord);
    void debugOutput(int dim);

    double* coords;
    //IPvXAddress ip;
    static uint8_t dim;
    static void setDim(uint8_t dimension) { dim = dimension; };
    uint8_t getDim() const { return dim; };
};

/**
 * representation of a single node in the GlobalNodeList
 *
 * @author Bernhard Heep
 */
class SimpleNodeEntry : public cPolymorphic
{
public:

    ~SimpleNodeEntry()
    {
        if (index == -1) delete nodeRecord;
    }

    /**
     * Constructor for node entries with 2D random coordinates
     *
     * @param node pointer to new terminal
     * @param typeRx receive access channel of new terminal
     * @param typeTx transmit access channel of new terminal
     * @param sendQueueLength initial send queue size
     * @param fieldSize length of one side of the coordinate space
     */
    SimpleNodeEntry(cModule* node, cChannelType* typeRx, cChannelType* typeTx,
                    uint32_t sendQueueLength, uint32_t fieldSize);

    /**
     * Constructor for node entries with given n-dim coordinates
     *
     * @param node pointer to new terminal
     * @param typeRx receive access channel of new terminal
     * @param typeTx transmit access channel of new terminal
     * @param sendQueueLength length of the send queue in bytes
     * @param nodeRecord the node's coordinates
     * @param index the position in unusedNodeRecords
     */
    SimpleNodeEntry(cModule* node, cChannelType* typeRx, cChannelType* typeTx,
                    uint32_t sendQueueLength, NodeRecord* nodeRecord,
                    int index);

    /**
     * Getter for SimpleUDP ingate
     *
     * @return the ingate
     */
    inline cGate* getUdpIPv4Gate() const
    {
        return UdpIPv4ingate;
    };

    /**
     * Getter for SimpleUDP IPv6 ingate
     *
     * @return the ingate
     */
    inline cGate* getUdpIPv6Gate() const
    {
        return UdpIPv6ingate;
    };

    /**
     * Getter for SimpleUDP ingate
     *
     * @return the ingate
     */
    inline cGate* getTcpIPv4Gate() const
    {
        return TcpIPv4ingate;
    };

    /**
     * Getter for SimpleUDP IPv6 ingate
     *
     * @return the ingate
     */
    inline cGate* getTcpIPv6Gate() const
    {
        return TcpIPv6ingate;
    };

    typedef std::pair<simtime_t, bool> SimpleDelay; //!< type for return value of calcDelay()

    /**
     * Calculates delay between two nodes
     *
     * @param msg pointer to message to get its length for delay calculation and set bit error flag
     * @param dest destination terminal
     * @param faultyDelay violate triangle inequality?
     * @return delay in s and boolean value that is false if message should be deleted
     */
    SimpleDelay calcDelay(cPacket* msg,
                          const SimpleNodeEntry& dest,
                          bool faultyDelay = false);

    /**
     * OMNeT++ info method
     *
     * @return infostring
     */
    std::string info() const;

    /**
     * Stream output
     *
     * @param out output stream
     * @param entry the terminal
     * @return reference to stream out
     */
    friend std::ostream& operator<<(std::ostream& out, const SimpleNodeEntry& entry);

    simtime_t getAccessDelay() const { return tx.accessDelay; };

    simtime_t getTxAccessDelay() const { return tx.accessDelay; };
    simtime_t getRxAccessDelay() const { return rx.accessDelay; };

    // typo fixed, thanks to huebby
    float getBandwidth() const { return tx.bandwidth; };

    float getTxBandwidth() const { return tx.bandwidth; };
    float getRxBandwidth() const { return rx.bandwidth; };

    float getErrorRate() const { return tx.errorRate; };

    inline float getX() const { return nodeRecord->coords[0]; };
    inline float getY() const { return nodeRecord->coords[1]; };
    inline float getCoords(int dim) const { return nodeRecord->coords[dim]; };
    inline uint8_t getDim() const { return nodeRecord->getDim(); };

    int getRecordIndex() const { return index; };
    NodeRecord* getNodeRecord() const { return nodeRecord; };

    /**
     * Calculates SHA1 hash over errorfree delay (always the same uniform distributed
     * value), uses this to generate a realistic error distribution and
     * returns the real RTT augmented with this error
     */
    static simtime_t getFaultyDelay(simtime_t oldDelay);


protected:

    /**
     * Calculates euclidean distance between two terminals
     *
     * @param entry destination entry
     * @return the euclidean distance
     */
    float operator-(const SimpleNodeEntry& entry) const;

    cGate* UdpIPv4ingate; //!< IPv4 ingate of the SimpleUDP module of this terminal
    cGate* UdpIPv6ingate; //!< IPv6 ingate of the SimpleUDP module of this terminal
    cGate* TcpIPv4ingate; //!< IPv4 ingate of the SimpleTCP module of this terminal
    cGate* TcpIPv6ingate; //!< IPv6 ingate of the SimpleTCP module of this terminal

    struct Channel {
        simtime_t finished; //!< send queue finished
        simtime_t maxQueueTime; //!< maximum time for packets to be queued
        simtime_t accessDelay; //!< first hop delay
        double bandwidth; //!< bandwidth in access net
        double errorRate; //!< packet loss rate
    } rx, tx;

    NodeRecord* nodeRecord;
    int index;
};


#endif // __SIMPLENODEENTRY_H
