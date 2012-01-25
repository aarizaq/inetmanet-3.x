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
 * @file NiceCluster.cc
 * @author Christian Huebsch
 */


#include "NiceCluster.h"

// Constructor *****************************************************************
NiceCluster::NiceCluster()
{
    leaderConfirmed = false;
    setLeader(TransportAddress::UNSPECIFIED_NODE);
    lastLT = 0.0;
    leaderHeartbeatsSent = 0;

    WATCH_SET(cluster);
    WATCH(leader);
    WATCH(leaderConfirmed);

} // NICECluster

// Adds member to cluster ******************************************************
void NiceCluster::add( const TransportAddress& member )
{

    cluster.insert(member);

} // add
//
// Clears all cluster contents *************************************************
void NiceCluster::clear()
{

    cluster.clear();
    setLeader(TransportAddress::UNSPECIFIED_NODE);

    lastLT = 0.0;

} // clear

// Check if cluster member *****************************************************
bool NiceCluster::contains( const TransportAddress& member )
{
    if (member.isUnspecified())
        return false;

    return cluster.find(member) != cluster.end();
} // contains
//
// Get cluster size ************************************************************
int NiceCluster::getSize()
{

    return cluster.size();

} // getSize

// Get address of specific member **********************************************
const TransportAddress& NiceCluster::get( int i )
{

    ConstClusterIterator it = cluster.begin();

    for (int j = 0; j < i; j++)
        it++;

    return *it;

} // get

void NiceCluster::remove(const TransportAddress& member)
{

    cluster.erase(member);
    if (!leader.isUnspecified() && !member.isUnspecified() && leader == member) {
        setLeader(TransportAddress::UNSPECIFIED_NODE);
    }

} // remove

// set cluster leader **********************************************************
void NiceCluster::setLeader(const TransportAddress& leader)
{

    if (!leader.isUnspecified() && !contains(leader)) {
        throw cRuntimeError("Leader not member of cluster");
    }
    this->leader = leader;

    leaderConfirmed = false;

//	//cout << "Leader set to " << leader << endl;

} // setLeader

// get current cluster leader **************************************************
const TransportAddress& NiceCluster::getLeader()
{

    return leader;

} // getLeader

std::set<TransportAddress>::const_iterator NiceCluster::begin() const
{
    return cluster.begin();
}

std::set<TransportAddress>::const_iterator NiceCluster::end() const
{
    return cluster.end();
}

simtime_t NiceCluster::getLastLT()
{

    return lastLT;

}

void NiceCluster::setLastLT()
{

    lastLT = simTime();

}

bool NiceCluster::isLeaderConfirmed()
{
    return leaderConfirmed;
}

void NiceCluster::confirmLeader()
{
    leaderConfirmed = true;
}

int NiceCluster::getLeaderHeartbeatsSent()
{
    return leaderHeartbeatsSent;
}

void NiceCluster::setLeaderHeartbeatsSent(int heartbeats)
{
    leaderHeartbeatsSent = heartbeats;
}

