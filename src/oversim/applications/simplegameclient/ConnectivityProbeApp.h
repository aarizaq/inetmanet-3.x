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
 * @file ConnectivityProbeApp.h
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#ifndef __CONNECTIVITYPROBEAPP_H__
#define __CONNECTIVITYPROBEAPP_H__

#include <SimpleGameClient.h>
#include <NodeHandle.h>
#include <Vector2D.h>

#include "GlobalStatisticsAccess.h"

class ConnectivityProbeApp : public cSimpleModule
{
    public:
        void initialize();
        void handleMessage(cMessage* msg);
        ~ConnectivityProbeApp();

    private:
        simtime_t probeIntervall;
        cMessage* probeTimer;
        GlobalStatistics* globalStatistics;
        std::map<NodeHandle, SimpleGameClient*> Topology;

        void extractTopology();

        // statistics
        cOutVector cOV_NodeCount;
        cOutVector cOV_ZeroMissingNeighbors;
        cOutVector cOV_AverageMissingNeighbors;
        cOutVector cOV_MaxMissingNeighbors;
        cOutVector cOV_AverageDrift;
};

#endif
