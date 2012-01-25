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
 * @file PastryTypes.h
 * @author Felix Palmen
 *
 * This file contains some structs and typedefs used internally by Pastry
 */

#ifndef __PASTRY_TYPES_H_
#define __PASTRY_TYPES_H_

#include <map>
#include <OverlayKey.h>
#include <NodeHandle.h>

#include "PastryMessage_m.h"

/**
 * value for infinite proximity (ping timeout):
 */
#define PASTRY_PROX_INFINITE -1

/**
 * value for undefined proximity:
 */
#define PASTRY_PROX_UNDEF -2

/**
 * value for not yet determined proximity value:
 */
#define PASTRY_PROX_PENDING -3

/**
 * struct-type for temporary proximity metrics to a STATE message
 */
struct PastryStateMsgProximity
{
    std::vector<simtime_t> pr_rt;
    std::vector<simtime_t> pr_ls;
    std::vector<simtime_t> pr_ns;
};

/**
 * struct-type containing local info while processing a STATE message
 */
struct PastryStateMsgHandle
{
    PastryStateMessage* msg;
    PastryStateMsgProximity* prox;
    bool outdatedUpdate;
    uint32_t nonce;

    PastryStateMsgHandle() : msg(NULL), prox(NULL), outdatedUpdate(false) {};
    PastryStateMsgHandle(PastryStateMessage* msg)
    : msg(msg), prox(NULL), outdatedUpdate(false)
    {
        nonce = intuniform(0, 0x7FFFFF);
    };
};

/**
 * struct for storing a NodeHandle together with its proximity value and an
 * optional timestamp
 */
struct PastryExtendedNode
{
    NodeHandle node;
    simtime_t rtt;
    simtime_t timestamp;

    PastryExtendedNode() : node(), rtt(-2), timestamp(0) {};
    PastryExtendedNode(const NodeHandle& node, simtime_t rtt,
                       simtime_t timestamp = 0)
    : node(node), rtt(rtt), timestamp(timestamp) {};
};


#endif
