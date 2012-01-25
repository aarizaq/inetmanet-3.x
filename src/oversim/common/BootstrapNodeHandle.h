//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file BootstrapNodeHandle.h
 * @author Bin Zheng
 */

#ifndef __BOOTSTRAP_NODE_HANDLE_H
#define __BOOTSTRAP_NODE_HANDLE_H

enum  BootstrapNodePrioType{
    DNSSD = 0,
    MDNS = 1,
    CACHE = 2,
};

class BootstrapNodeHandle : public NodeHandle {

private:
    BootstrapNodePrioType nodePrio; //1: uDNS, 2: mDNS, 3: saved nodes from last run
    simtime_t lastPing;

public:
    BootstrapNodeHandle() : NodeHandle()
    {
        /* lowest priority */
        nodePrio = CACHE;
    }

    BootstrapNodeHandle(const BootstrapNodeHandle &handle) : NodeHandle(handle)
    {
        nodePrio = handle.nodePrio;
    }

    BootstrapNodeHandle(const NodeHandle &handle,
                        BootstrapNodePrioType prio = CACHE)
    {
        this->ip = handle.getIp();
        this->port = handle.getPort();
        this->key = handle.getKey();
        nodePrio = prio;
    }

    BootstrapNodeHandle(const OverlayKey &key,
                        const IPvXAddress &ip,
                        int port,
                        BootstrapNodePrioType prio = CACHE)
                        : NodeHandle(key, ip, port)   {
        nodePrio = prio;
    }

    inline BootstrapNodePrioType getNodePrio() const {
        return nodePrio;
    }

    inline void setNodePrio(BootstrapNodePrioType nodePrio) {
        this->nodePrio = nodePrio;
    }

    inline simtime_t getLastPing() const {
        return lastPing;
    }

    inline void setLastPing(simtime_t lastPing) {
        this->lastPing = lastPing;
    }
};
#endif
