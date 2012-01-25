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
 * @file PeerInfo.h
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#ifndef __PEERINFO_H__
#define __PEERINFO_H__

#include <sstream>

#include <omnetpp.h>

class PeerInfo;
class BaseOverlay;

/**
 * Base class for providing additional underlay specific
 * information associated with a certain transport address
 */
class PeerInfo
{
    friend class PeerStorage;
public:
    /**
     * constructor
     */
    PeerInfo(uint32_t type, int moduleId, cObject** context);


    virtual ~PeerInfo() {};

    /**
     * has the peer bootstrapped yet?
     *
     * @return true if the peer has bootstrapped, false otherwise
     */
    bool isBootstrapped() { return bootstrapped; };

    /**
     * returns the moduleId of the peer
     *
     * @return the moduleId
     */
    int getModuleID() { return moduleId; };

    /**
     * returns the NPS layer of the peer
     *
     * @return the NPS layer
     */
    int8_t getNpsLayer() { return npsLayer; };

    /**
     * set the NPS layer of the peer
     */
    void setNpsLayer(int8_t layer) { npsLayer = layer; }

   /**
     * returns the type of the node
     *
     * @return the node's typeID
     */
    uint32_t getTypeID() { return type; };

    /**
     * is the peer marked for deletion?
     *
     * @return true if the peer is marked for deletion, false otherwise
     */
    bool isPreKilled() { return preKilled; };

    /**
    * mark that the peer gets deleted soon
    *
    * @param killed true, if the peer gets deleted soon
    */
    void setPreKilled(bool killed = true) { preKilled = killed; };

    /**
     * is the peer malicious?
     *
     * @return true if the peer is malicious, false otherwise
     */
    bool isMalicious() { return malicious; };

    cObject** getContext() { return context; };


    /**
     * standard output stream for PeerInfo,
     * gives moduleID and true if peer has bootstrapped, false otherwise
     *
     * @param os the ostream
     * @param info the PeerInfo
     * @return the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const PeerInfo info);

private:
    virtual void dummy(); /**< dummy-function to make PeerInfo polymorphic */

    /**
     * set the maliciousness of the peer
     *
     * @param malic whether the peer is malicious or not
     */
    void setMalicious(bool malic = true) { malicious = malic; };

    /**
     * sets or deletes the bootstrapped parameter
     *
     * @param bootstrap true or () if peer has bootstrapped, false otherwise
     * */
    void setBootstrapped(bool bootstrap = true) { bootstrapped = bootstrap; };

    bool bootstrapped; /**< true if node has bootstrapped */
    bool malicious; /**< true if the node is malicious */
    bool preKilled; /**< true, if the node is marked for deletion */
    int moduleId;  /**< the moduleId of the peer */
    uint32_t type; /**< ID of the node type */
    int8_t npsLayer; /**< NPS Layer of the node */
    cObject** context; /**< context pointer */
};

#endif
