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
 * @file NicePeerInfo.cc
 * @author Christian Huebsch
 */


#include "NicePeerInfo.h"

namespace oversim
{

NicePeerInfo::NicePeerInfo(Nice* _parent)
        : parent (_parent)
{

    distance_estimation_start = -1;
    distance = -1;
    last_sent_HB = 1;
    last_recv_HB = 0;
    backHBPointer = false;
    last_HB_arrival = 0;

    activity = simTime().dbl();

    subclustermembers = 0;

    WATCH_MAP(distanceTable);
    WATCH(last_sent_HB);
    WATCH(last_recv_HB);
    WATCH(last_HB_arrival);

} // NicePeerInfo

NicePeerInfo::~NicePeerInfo()
{

} // ~NicePeerInfo

void NicePeerInfo::set_distance_estimation_start(double value)
{

    distance_estimation_start = value;

} // set_distance_estimation_start

double NicePeerInfo::getDES()
{

    return distance_estimation_start;

} // getDES

void NicePeerInfo::set_distance(double value)
{

    distance = value;

} // set_distance

double NicePeerInfo::get_distance()
{

    return distance;

} // get_distance

cMessage* NicePeerInfo::getHbTimer()
{

    return hbTimer;

} // startHeartbeatTimeout

void NicePeerInfo::updateDistance(TransportAddress member, double distance)
{
    //get member out of map
    std::map<TransportAddress, double>::iterator it = distanceTable.find(member);

    if (it != distanceTable.end()) {

        it->second = distance;

    } else {

        distanceTable.insert(std::make_pair(member, distance));

    }


} // updateDistance

double NicePeerInfo::getDistanceTo(TransportAddress member)
{

    //std::cout << "getDistanceTo " << member.getIp() << "..." << endl;
    //get member out of map
    std::map<TransportAddress, double>::iterator it = distanceTable.find(member);

    if (it != distanceTable.end()) {

        //std::cout << "is in distanceTable" << endl;
        return it->second;

    } else {

        //std::cout << "is NOT in distanceTable" << endl;
        return -1;

    }


} // getDistanceTo

unsigned int NicePeerInfo::get_last_sent_HB()
{

    return last_sent_HB;

} // get_last_sent_HB

void NicePeerInfo::set_last_sent_HB(unsigned int seqNo)
{

    last_sent_HB = seqNo;

} // set_last_sent_HB

unsigned int NicePeerInfo::get_last_recv_HB()
{

    return last_recv_HB;

} // get_last_recv_HB

void NicePeerInfo::set_last_recv_HB(unsigned int seqNo)
{

    last_recv_HB = seqNo;

} // set_last_recv_HB

double NicePeerInfo::get_last_HB_arrival()
{

    return last_HB_arrival;

} // get_last_HB_arrival


void NicePeerInfo::set_last_HB_arrival(double arrival)
{

    last_HB_arrival = arrival;

} // set_last_HB_arrival

bool NicePeerInfo::get_backHBPointer()
{

    return backHBPointer;

} // get_backHBPointer

void NicePeerInfo::set_backHBPointer(bool _backHBPointer)
{

    backHBPointer = _backHBPointer;

} // set_backHBPointer

void NicePeerInfo::set_backHB(bool backHBPointer, unsigned int seqNo, double time)
{

    backHB[backHBPointer].first = seqNo;
    backHB[backHBPointer].second = time;

} // set_backHB

double NicePeerInfo::get_backHB(unsigned int seqNo)
{

    double time = -1;

    if (backHB[0].first == seqNo)
        time = backHB[0].second;
    else if (backHB[1].first == seqNo)
        time = backHB[1].second;

    return time;

} // get_backHB

unsigned int NicePeerInfo::get_backHB_seqNo(bool index)
{

    return backHB[index].first;

} // get_backHB_seqNo


void NicePeerInfo::touch()
{

    activity = simTime().dbl();

} // touch


double NicePeerInfo::getActivity()
{

    return activity;

} // getActivity


void NicePeerInfo::setSubClusterMembers( unsigned int members )
{

    subclustermembers = members;

}


unsigned int NicePeerInfo::getSubClusterMembers()
{

    return subclustermembers;

}


std::ostream& operator<<(std::ostream& os, NicePeerInfo& pi)
{
    os << "distance: " << pi.distance << endl;
    os << "des: " << pi.distance_estimation_start << endl;
    os << "last_rcv: " << pi.get_last_recv_HB() << endl;
    os << "last_sent: " << pi.get_last_sent_HB() << endl;
    os << "last_HB: " << pi.get_last_HB_arrival() << endl;
    os << "backHB[0].seqNo: " << pi.get_backHB_seqNo(0) << endl;
    os << "backHB[0].time: " << pi.get_backHB(pi.get_backHB_seqNo(0)) << endl;
    os << "backHB[1].seqNo: " << pi.get_backHB_seqNo(1) << endl;
    os << "backHB[1].time: " << pi.get_backHB(pi.get_backHB_seqNo(1)) << endl;
    os << "activity: " << pi.getActivity() << endl;

    std::map<TransportAddress, double>::iterator it = pi.distanceTable.begin();

    while (it != pi.distanceTable.end()) {
        os << it->first << " : " << it->second << endl;
        it++;
    }

    return os;
}

}; //namespace


