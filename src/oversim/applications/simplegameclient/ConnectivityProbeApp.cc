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
* @file ConnectivityProbeApp.cc
* @author Helge Backhaus
* @author Stephan Krause
*/

#include "ConnectivityProbeApp.h"

Define_Module(ConnectivityProbeApp);

void ConnectivityProbeApp::initialize()
{
    globalStatistics = GlobalStatisticsAccess().get();
    probeIntervall = par("connectivityProbeIntervall");
    probeTimer = new cMessage("probeTimer");

    if(probeIntervall > 0.0) {
        scheduleAt(simTime() + probeIntervall, probeTimer);

        cOV_NodeCount.setName("total node count");
        cOV_ZeroMissingNeighbors.setName("neighbor-error free nodes");
        cOV_AverageMissingNeighbors.setName("average missing neighbors per node");
        cOV_MaxMissingNeighbors.setName("largest missing neighbors count");
        cOV_AverageDrift.setName("average drift");
    }

}

void ConnectivityProbeApp::handleMessage(cMessage* msg)
{
    // fill topology with all modules
    extractTopology();

    if(Topology.size() == 0) {
        return;
    }

    // catch self timer messages
    if(msg->isName("probeTimer")) {
        //reset timer
        cancelEvent(probeTimer);
        scheduleAt(simTime() + probeIntervall, msg);

        int mnMax = 0;
        int mnZero = 0;
        int driftCount = 0;
        double mnAverage = 0.0;
        double drift = 0.0;

        for(std::map<NodeHandle, SimpleGameClient*>::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
            int missing = 0;
            Vector2D pos = itTopology->second->getPosition();
            double AOIWidth = itTopology->second->getAOI();
            for(std::map<NodeHandle, SimpleGameClient*>::iterator itI = Topology.begin(); itI != Topology.end(); ++itI) {
                if(itI != itTopology && pos.distanceSqr(itI->second->getPosition()) <= AOIWidth*AOIWidth) {
                    NeighborMap::iterator currentSite = itTopology->second->Neighbors.find(itI->second->getThisNode());
                    if(currentSite == itTopology->second->Neighbors.end()) {
                        ++missing;
                    }
                    else {
                        drift += sqrt(currentSite->second.position.distanceSqr(itI->second->getPosition()));
                        ++driftCount;
                    }
                }
            }

            mnAverage += missing;
            if(mnMax < missing) {
                mnMax = missing;
            }
            if(missing == 0) {
                ++mnZero;
            }
        }
        mnAverage /= (double)Topology.size();
        if(driftCount > 0) {
            drift /= (double)driftCount;
        }

        cOV_ZeroMissingNeighbors.record((double)mnZero);
        RECORD_STATS (
            globalStatistics->addStdDev("ConnectivityProbe: percentage zero missing neighbors", (double)mnZero * 100.0 / (double)Topology.size());
            globalStatistics->addStdDev("ConnectivityProbe: average drift", drift);
        );
        cOV_AverageMissingNeighbors.record(mnAverage);
        cOV_MaxMissingNeighbors.record((double)mnMax);
        cOV_AverageDrift.record(drift);
    }
    Topology.clear();
}

void ConnectivityProbeApp::extractTopology()
{
    for(int i=0; i<=simulation.getLastModuleId(); i++) {
        cModule* module = simulation.getModule(i);
        SimpleGameClient* client;
        if((client = dynamic_cast<SimpleGameClient*>(module))) {

            if(client->isOverlayReady()) {
                Topology.insert(std::make_pair(client->getThisNode(), client));
            }
        }
    }
}

ConnectivityProbeApp::~ConnectivityProbeApp()
{
    // destroy self timer messages
    cancelAndDelete(probeTimer);
}
