//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file NiceCluster.h
 * @author Christian Huebsch
 */

#ifndef __NICECLUSTER_H_
#define __NICECLUSTER_H_

#include <BaseOverlay.h>
#include <omnetpp.h>

class NiceCluster
{

private:

    /* set of cluster members */
    std::set<TransportAddress> cluster;

    /* leader of the cluster */
    TransportAddress leader;
    
    /* last time of leader transfer */
    simtime_t lastLT;

    /* if we are reasonably sure that the cluster leader is really the above node */
    bool leaderConfirmed;

    /* how many times we've sent leader heartbeats to the nodes in this cluster */
    int leaderHeartbeatsSent;

    typedef std::set<TransportAddress>::const_iterator ConstClusterIterator;
    typedef std::set<TransportAddress>::iterator ClusterIterator;

public:

    NiceCluster();

    /**
     * Adds address to cluster.
     */
    void add(const TransportAddress& member);

    /**
     * Removes all addresses from cluster,
     * sets leader to unspecified address
     * and sets last lt to 0.
     */
    void clear();

    /**
     * Tests if given address is in cluster.
     */
    bool contains( const TransportAddress& member );

    /**
     * Returns number of addresses in cluster.
     */
    int getSize();

    /**
     * Returns i-th address in cluster, counting by order.
     */
    const TransportAddress& get( int i );

    /**
     * Removes address from cluster.
     * If address is leader, sets leader to unspecified address.
     */
    void remove(const TransportAddress& member);

    /**
     * Set leader to given address.
     * If leader is not in cluster, throws a cRuntimeError.
     * The new leader is unconfirmed, call confirmLeader(), when
     * we are sure that the leader info is correct.
     */
    void setLeader(const TransportAddress& leader);

    /**
     * Returns leader address.
     */
    const TransportAddress& getLeader();

    /**
     * Returns const iterator to first address.
     */
    std::set<TransportAddress>::const_iterator begin() const;

    /**
     * Returns const iterator to end of cluster.
     */
    std::set<TransportAddress>::const_iterator end() const;

    simtime_t getLastLT();
    void setLastLT();

    /**
     * The leader is "confirmed", when we can say reasonably sure that
     * all the nodes in the cluster think of the leader as the leader.
     */
    bool isLeaderConfirmed();

    /**
     * Confirm that we're sure that our leader information is correct.
     */
    void confirmLeader();

    int getLeaderHeartbeatsSent();

    void setLeaderHeartbeatsSent(int heartbeats);

}; // NiceCluster

#endif /* _NICECLUSTER_H_ */
