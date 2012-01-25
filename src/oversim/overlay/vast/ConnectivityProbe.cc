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
* @file ConnectivityProbe.cc
* @author Helge Backhaus
*/

#include "ConnectivityProbe.h"

Define_Module(ConnectivityProbe);

VTopologyNode::VTopologyNode(int moduleID)
{
    this->moduleID = moduleID;
    visited = false;
}

Vast* VTopologyNode::getModule() const
{
    return check_and_cast<Vast*>(simulation.getModule(moduleID));
}

void ConnectivityProbe::initialize()
{
    globalStatistics = GlobalStatisticsAccess().get();
    probeIntervall = par("connectivityProbeIntervall");
    plotIntervall = par("visualizeNetworkIntervall");
    probeTimer = new cMessage("probeTimer");
    plotTimer = new cMessage("plotTimer");
    plotConnections = par("plotConnections");
    plotMissing= par("plotMissing");

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
        scheduleAt(simTime() + plotIntervall, plotTimer);
    }
}

void ConnectivityProbe::handleMessage(cMessage* msg)
{
    // fill topology with all VAST modules
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
        for(VTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
            unsigned int count = getComponentSize(itTopology->second.getModule()->getHandle().getKey());
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

        for(VTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
            double AOIWidthSqr = itTopology->second.getModule()->getAOI();
            AOIWidthSqr *= AOIWidthSqr;
            Vector2D vastPosition = itTopology->second.getModule()->getPosition();
            int missing = 0;
            for(VTopology::iterator itI = Topology.begin(); itI != Topology.end(); ++itI) {
                if(itI != itTopology && vastPosition.distanceSqr(itI->second.getModule()->getPosition()) <= AOIWidthSqr) {
                    SiteMap::iterator currentSite = itTopology->second.getModule()->Sites.find(itI->second.getModule()->getHandle());
                    if(currentSite == itTopology->second.getModule()->Sites.end()) {
                        ++missing;
                    }
                    else {
                        drift += sqrt(currentSite->second->coord.distanceSqr(itI->second.getModule()->getPosition()));
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
        scheduleAt(simTime() + plotIntervall, msg);

        bool missingFound = false;
        if(plotMissing) {
            for(VTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                double AOIWidthSqr = itTopology->second.getModule()->getAOI();
                AOIWidthSqr *= AOIWidthSqr;
                Vector2D vastPosition = itTopology->second.getModule()->getPosition();
                for(VTopology::iterator itI = Topology.begin(); itI != Topology.end(); ++itI) {
                    if(itI != itTopology && vastPosition.distanceSqr(itI->second.getModule()->getPosition()) <= AOIWidthSqr) {
                        SiteMap::iterator currentSite = itTopology->second.getModule()->Sites.find(itI->second.getModule()->getHandle());
                        if(currentSite == itTopology->second.getModule()->Sites.end()) {
                            missingFound=true;
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

            // open/ write plot file
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
            for(VTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                pltData << itTopology->second.getModule()->getPosition().x << "\t" << itTopology->second.getModule()->getPosition().y << endl;
            }
            pltData.close();

            //write arrow data file
            if(!plotMissing) {
                for(VTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                    for(SiteMap::iterator itSites = itTopology->second.getModule()->Sites.begin(); itSites != itTopology->second.getModule()->Sites.end(); ++itSites) {
                        if(plotConnections) {
                            VTopology::iterator destNode = Topology.find(itSites->second->addr.getKey());
                            if(destNode != Topology.end()) {
                                Vector2D relPos = destNode->second.getModule()->getPosition() - itTopology->second.getModule()->getPosition();
                                pltVector << itTopology->second.getModule()->getPosition().x << "\t" << itTopology->second.getModule()->getPosition().y << "\t"
                                    << relPos.x << "\t" << relPos.y << endl;
                            }
                        }
                        else {
                            Vector2D relPos = itSites->second->coord - itTopology->second.getModule()->getPosition();
                            pltVector << itTopology->second.getModule()->getPosition().x << "\t" << itTopology->second.getModule()->getPosition().y << "\t"
                                << relPos.x << "\t" << relPos.y << endl;
                        }
                    }
                }
            } else {
                for(VTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
                    double AOIWidthSqr = itTopology->second.getModule()->getAOI();
                    AOIWidthSqr *= AOIWidthSqr;
                    Vector2D vastPosition = itTopology->second.getModule()->getPosition();
                    for(VTopology::iterator itI = Topology.begin(); itI != Topology.end(); ++itI) {
                        if(itI != itTopology && vastPosition.distanceSqr(itI->second.getModule()->getPosition()) <= AOIWidthSqr) {
                            SiteMap::iterator currentSite = itTopology->second.getModule()->Sites.find(itI->second.getModule()->getHandle());
                            if(currentSite == itTopology->second.getModule()->Sites.end()) {
                                Vector2D relPos = itI->second.getModule()->getPosition() - itTopology->second.getModule()->getPosition();
                                pltVector << itTopology->second.getModule()->getPosition().x << "\t"
                                          << itTopology->second.getModule()->getPosition().y << "\t"
                                          << relPos.x << "\t" << relPos.y <<  "\t"
                                          << itTopology->second.getModule()->getParentModule()->getParentModule()->getFullName() << ":"
                                          << itTopology->second.getModule()->thisSite.addr.getKey().toString(16) << "\t"
                                          << itI->second.getModule()->getParentModule()->getParentModule()->getFullName() << ":"
                                          << itI->second.getModule()->thisSite.addr.getKey().toString(16) << endl;
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

void ConnectivityProbe::extractTopology()
{
    for(int i=0; i<=simulation.getLastModuleId(); i++) {
        cModule* module = simulation.getModule(i);
        if(module && dynamic_cast<Vast*>(module)) {
            Vast* vast = check_and_cast<Vast*>(module);
            if(vast->getState() == BaseOverlay::READY) {
                VTopologyNode temp(i);
                Topology.insert(std::make_pair(vast->getHandle().getKey(), temp));
            }
        }
    }
}

void ConnectivityProbe::resetTopologyNodes()
{
    for(VTopology::iterator itTopology = Topology.begin(); itTopology != Topology.end(); ++itTopology) {
        itTopology->second.visited = false;
    }
}

unsigned int ConnectivityProbe::getComponentSize(OverlayKey key)
{
    VTopology::iterator itEntry = Topology.find(key);
    if(itEntry != Topology.end() && itEntry->second.visited == false) {
        int count = 1;
        itEntry->second.visited = true;
        Vast* vast = itEntry->second.getModule();
        for(SiteMap::iterator itSites = vast->Sites.begin(); itSites != vast->Sites.end(); ++itSites) {
            count += getComponentSize(itSites->first.getKey());
        }
        return count;
    }
    return 0;
}

ConnectivityProbe::~ConnectivityProbe()
{
    // destroy self timer messages
    cancelAndDelete(probeTimer);
    cancelAndDelete(plotTimer);
}
