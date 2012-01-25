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
 * @file ConnectivityProbeQuon.h
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#ifndef __CONNECTIVITYPROBEQUON_H__
#define __CONNECTIVITYPROBEQUON_H__

#include <omnetpp.h>
#include <NodeHandle.h>
#include <QuonHelper.h>
#include <Quon.h>
#include <fstream>
#include <sstream>
#include "GlobalStatisticsAccess.h"

class QuonTopologyNode
{
    public:
        QuonTopologyNode(int moduleID);
        Quon* getModule() const;

        bool visited;
        int moduleID;
};

typedef std::map<OverlayKey, QuonTopologyNode> QuonTopology;

class ConnectivityProbeQuon : public cSimpleModule
{
    public:
        void initialize();
        void handleMessage(cMessage* msg);
        ~ConnectivityProbeQuon();

    private:
        std::fstream pltNetwork, pltData, pltVector;
        void extractTopology();
        void resetTopologyNodes();
        unsigned int getComponentSize(OverlayKey key);

        simtime_t probeIntervall;
        simtime_t plotIntervall;
        simtime_t startPlotTime;
        simtime_t plotPeriod;
        bool plotConnections;
        bool plotBindings;
        bool plotMissing;
        cMessage* probeTimer;
        cMessage* plotTimer;
        QuonTopology Topology;
        GlobalStatistics* globalStatistics;

        // statistics
        cOutVector cOV_NodeCount;
        cOutVector cOV_MaximumComponent;
        cOutVector cOV_MaxConnectivity;
        cOutVector cOV_ZeroMissingNeighbors;
        cOutVector cOV_AverageMissingNeighbors;
        cOutVector cOV_MaxMissingNeighbors;
        cOutVector cOV_AverageDrift;
};

#endif
