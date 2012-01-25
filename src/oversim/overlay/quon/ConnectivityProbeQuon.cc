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
* @file ConnectivityProbeQuon.cc
* @author Helge Backhaus
* @author Stephan Krause
*/

#include "ConnectivityProbeQuon.h"

Define_Module(ConnectivityProbeQuon);

QuonTopologyNode::QuonTopologyNode(int moduleID)
{
    this->moduleID = moduleID;
    visited = false;
}

Quon* QuonTopologyNode::getModule() const
{
    return check_and_cast<Quon*>(simulation.getModule(moduleID));
}

void ConnectivityProbeQuon::initialize()
{
    globalStatistics = GlobalStatisticsAccess().get();
    probeIntervall = par("connectivityProbeIntervall");
    plotIntervall = par("visualizeNetworkIntervall");
    startPlotTime = par("startPlotTime");
    plotPeriod = par("plotPeriod");
    probeTimer = new cMessage("probeTimer");
    plotTimer = new cMessage("plotTimer");
    plotConnections = par("plotConnections");
    plotBindings = par("plotBindings");
    plotMissing = par("plotMissing");

    if(probeIntervall > 0.0) {
        scheduleAt(simTime() + probeIntervall, probeTimer);

        cOV_NodeCount.setName("total node count");
        cOV_MaximumComponent.setName("largest connected component");
        cOV_MaxConnectivity.setName("connectivity in percent");
        cOV_ZeroMissingNeighbors.setName("neighbor-error free nodes");
        cOV_AverageMissingNeighbors.setName("average missing neighbors per node");
        cOV_MaxMissingNeighbors.setName("largest missing neighbors count");
        cOV_AverageDrift.setName("average drift");
    }

    if(plotIntervall > 0.0) {
        if(startPlotTime == 0.0) {
            scheduleAt(simTime() + plotIntervall, plotTimer);
        }
        else {
            scheduleAt(simTime() + startPlotTime, plotTimer);
        }
    }
}

void ConnectivityProbeQuon::handleMessage(cMessage* msg)
{
    // fill topology with all QUONP modules
    extractTopology();

    if(Topology.size() == 0) {
        return;
    }

    // catch self timer messages
    if(msg->isName("probeTimer")) {
        //reset timer
        cancelEvent(probeTimer);
        scheduleAt(simTime() + probeIntervall, msg);

        unsigned int maxComponent = 0;
        for(QuonTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
            unsigned int count = getComponentSize(itTopology->second.getModule()->getKey());
            if(count > maxComponent) {
                maxComponent = count;
            }
            resetTopologyNodes();
            if(count == Topology.size()) {
                break;
            }
        }

        cOV_NodeCount.record((double)Topology.size());
        cOV_MaximumComponent.record((double)maxComponent);
        cOV_MaxConnectivity.record((double)maxComponent * 100.0 / (double)Topology.size());
        RECORD_STATS (
            globalStatistics->addStdDev("ConnectivityProbe: max connectivity", (double)maxComponent * 100.0 / (double)Topology.size());
        );

        int mnMax = 0;
        int mnZero = 0;
        int driftCount = 0;
        double mnAverage = 0.0;
        double drift = 0.0;

        for(QuonTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
            QuonAOI AOI(itTopology->second.getModule()->getPosition(), itTopology->second.getModule()->getAOI());
            int missing = 0;
            for(QuonTopology::iterator itI = Topology.begin(); itI != Topology.end(); ++itI) {
                if(itI != itTopology && AOI.collide(itI->second.getModule()->getPosition())) {
                    QuonSiteMap::iterator currentSite = itTopology->second.getModule()->Sites.find(itI->second.getModule()->getKey());
                    if(currentSite == itTopology->second.getModule()->Sites.end()) {
                        ++missing;
                    }
                    else {
                        drift += sqrt(currentSite->second->position.distanceSqr(itI->second.getModule()->getPosition()));
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
    else if(msg->isName("plotTimer")) {
        //reset timer
        cancelEvent(plotTimer);
        if(plotPeriod == 0.0 || simTime() <= startPlotTime + plotPeriod) {
            scheduleAt(simTime() + plotIntervall, msg);
        }

        bool missingFound = false;
        if(plotMissing) {
            for(QuonTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                QuonAOI AOI(itTopology->second.getModule()->getPosition(), itTopology->second.getModule()->getAOI());
                for(QuonTopology::iterator itI = Topology.begin(); itI != Topology.end(); ++itI) {
                    if(itI != itTopology && AOI.collide(itI->second.getModule()->getPosition())) {
                        QuonSiteMap::iterator currentSite = itTopology->second.getModule()->Sites.find(itI->second.getModule()->getKey());
                        if(currentSite == itTopology->second.getModule()->Sites.end()) {
                            missingFound = true;
                        }
                    }
                }
            }
        }

        if(!plotMissing || missingFound) {
            int range = (int)Topology.begin()->second.getModule()->getAreaDimension();
            std::stringstream oss;
            std::string filename;
            int simTimeInt, stellen = 1;
            simTimeInt = (int)SIMTIME_DBL(simTime());
            oss << "plot";
            for(int i=0; i<6; i++) {
                if(!(simTimeInt / stellen)) {
                    oss << "0";
                }
                stellen *= 10;
            }
            oss << simTimeInt;

            // open / write plot file
            filename = oss.str() + ".plot";
            pltNetwork.open(filename.c_str(), std::ios::out);
            pltNetwork << "set xrange [0:" << range << "]" << endl;
            pltNetwork << "set yrange [0:" << range << "]" << endl;
            pltNetwork << "set nokey" << endl;

            // open point file
            filename = oss.str() + ".point";
            pltData.open(filename.c_str(), std::ios::out);

            pltNetwork << "plot '" << filename << "' using 1:2 with points pointtype 7,\\" << endl;

            // open vector file
            filename = oss.str() + ".arrow";
            pltVector.open(filename.c_str(), std::ios::out);

            pltNetwork << "     '" << filename << "' using 1:2:3:4 with vectors linetype 1" << endl;
            pltNetwork.close();

            // write point data file
            for(QuonTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                pltData << itTopology->second.getModule()->getPosition().x << "\t" << itTopology->second.getModule()->getPosition().y << endl;
            }
            pltData.close();

            //write arrow data file
            if(!plotMissing) {
                for(QuonTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                    for(QuonSiteMap::iterator itSites = itTopology->second.getModule()->Sites.begin(); itSites != itTopology->second.getModule()->Sites.end(); ++itSites) {
                        if(plotBindings && itSites->second->type != QBINDING && !itSites->second->softNeighbor) {
                            continue;
                        }
                        if(plotConnections) {
                            QuonTopology::iterator destNode = Topology.find(itSites->second->address.getKey());
                            if(destNode != Topology.end()) {
                                Vector2D relPos = destNode->second.getModule()->getPosition() - itTopology->second.getModule()->getPosition();
                                pltVector << itTopology->second.getModule()->getPosition().x << "\t" << itTopology->second.getModule()->getPosition().y << "\t"
                                        << relPos.x << "\t" << relPos.y << endl;
                            }
                        }
                        else {
                            Vector2D relPos = itSites->second->position - itTopology->second.getModule()->getPosition();
                            pltVector << itTopology->second.getModule()->getPosition().x << "\t" << itTopology->second.getModule()->getPosition().y << "\t"
                                    << relPos.x << "\t" << relPos.y << endl;
                        }
                    }
                }
            }
            else {
                for(QuonTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                    QuonAOI AOI(itTopology->second.getModule()->getPosition(), itTopology->second.getModule()->getAOI());
                    for(QuonTopology::iterator itI = Topology.begin(); itI != Topology.end(); ++itI) {
                        if(itI != itTopology && AOI.collide(itI->second.getModule()->getPosition())) {
                            QuonSiteMap::iterator currentSite = itTopology->second.getModule()->Sites.find(itI->second.getModule()->getKey());
                            if(currentSite == itTopology->second.getModule()->Sites.end()) {
                                Vector2D relPos = itI->second.getModule()->getPosition() - itTopology->second.getModule()->getPosition();
                                pltVector << itTopology->second.getModule()->getPosition().x << "\t"
                                          << itTopology->second.getModule()->getPosition().y << "\t"
                                          << relPos.x << "\t" << relPos.y <<  "\t"
                                          << itTopology->second.getModule()->getParentModule()->getParentModule()->getFullName() << ":"
                                          << itTopology->second.getModule()->getKey().toString(16) << "\t"
                                          << itI->second.getModule()->getParentModule()->getParentModule()->getFullName() << ":"
                                          << itI->second.getModule()->getKey().toString(16) << endl;
                            }
                        }
                    }
                }
            }
            pltVector.close();
        }
    }
    Topology.clear();
}

void ConnectivityProbeQuon::extractTopology()
{
    for(int i=0; i<=simulation.getLastModuleId(); i++) {
        cModule* module = simulation.getModule(i);
        if(module && dynamic_cast<Quon*>(module)) {
            Quon* quonp = check_and_cast<Quon*>(module);
            if(quonp->getState() == QREADY) {
                QuonTopologyNode temp(i);
                Topology.insert(std::make_pair(quonp->getKey(), temp));
            }
        }
    }
}

void ConnectivityProbeQuon::resetTopologyNodes()
{
    for(QuonTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
        itTopology->second.visited = false;
    }
}

unsigned int ConnectivityProbeQuon::getComponentSize(OverlayKey key)
{
    QuonTopology::iterator itEntry = Topology.find(key);
    if(itEntry != Topology.end() && itEntry->second.visited == false) {
        int count = 1;
        itEntry->second.visited = true;
        Quon* quonp = itEntry->second.getModule();
        for(QuonSiteMap::iterator itSites = quonp->Sites.begin(); itSites != quonp->Sites.end(); ++itSites) {
            count += getComponentSize(itSites->first);
        }
        return count;
    }
    return 0;
}

ConnectivityProbeQuon::~ConnectivityProbeQuon()
{
    // destroy self timer messages
    cancelAndDelete(probeTimer);
    cancelAndDelete(plotTimer);
}
