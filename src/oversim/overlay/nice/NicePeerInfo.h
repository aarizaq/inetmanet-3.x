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
 * @file NicePeerInfo.h
 * @author Christian Huebsch
 */

#ifndef __NICEPEERINFO_H_
#define __NICEPEERINFO_H_

#include <BaseOverlay.h>
#include <omnetpp.h>
#include <iostream>

namespace oversim
{

class Nice;

}

namespace oversim
{

typedef std::pair<unsigned int, double> HeartbeatEvaluator;

class NicePeerInfo
{
    friend std::ostream& operator<<(std::ostream& os, NicePeerInfo& pi);

public:

    NicePeerInfo(Nice* _parent);
    ~NicePeerInfo();

    void set_distance_estimation_start(double value);
    double getDES();
    void set_distance(double value);
    double get_distance();

    void startHeartbeatTimeout();
    cMessage* getHbTimer();

    void updateDistance(TransportAddress member, double distance);
    double getDistanceTo(TransportAddress member);

    unsigned int get_last_sent_HB();
    void set_last_sent_HB(unsigned int seqNo);

    unsigned int get_last_recv_HB();
    void set_last_recv_HB(unsigned int seqNo);

    double get_last_HB_arrival();
    void set_last_HB_arrival(double arrival);

    bool get_backHBPointer();
    void set_backHBPointer(bool _backHBPointer);

    void set_backHB(bool backHBPointer, unsigned int seqNo, double time);
    double get_backHB(unsigned int seqNo);
    unsigned int get_backHB_seqNo(bool index);

    void touch();
    double getActivity();

    void setSubClusterMembers( unsigned int members );
    unsigned int getSubClusterMembers();

protected:

    //virtual void handleMessage(cMessage* msg);

private:

    Nice* parent;
    double distance_estimation_start;
    double distance;
    cMessage* hbTimer;
    std::map<TransportAddress, double> distanceTable;

    double activity;

    unsigned int subclustermembers;

    HeartbeatEvaluator backHB[2];
    bool backHBPointer;

    unsigned int last_sent_HB;
    unsigned int last_recv_HB;
    double last_HB_arrival;


}; // NicePeerInfo

}; //namespace

#endif /* _NICEPEERINFO_H_ */
