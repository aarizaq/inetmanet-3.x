//
// Copyright (C) 2010 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file SimpleNcs.cc
 * @author Daniel Lienert, Bernhard Heep
 */

#include "SimpleNcs.h"

#include <SHA1.h>
#include <GlobalNodeListAccess.h>
#include <OverlayAccess.h>
#include <SimpleNodeEntry.h>
#include <SimpleInfo.h>

std::string SimpleNcs::delayFaultTypeString;
std::map<std::string, SimpleNcs::delayFaultTypeNum> SimpleNcs::delayFaultTypeMap;
bool SimpleNcs::faultyDelay;

void SimpleNcs::init(NeighborCache* neighborCache)
{
    delayFaultTypeMap["live_all"] = delayFaultLiveAll;
    delayFaultTypeMap["live_planetlab"] = delayFaultLivePlanetlab;
    delayFaultTypeMap["simulation"] = delayFaultSimulation;

    this->neighborCache = neighborCache;

    delayFaultTypeString =
        neighborCache->par("simpleNcsDelayFaultType").stdstringValue();

    switch (delayFaultTypeMap[delayFaultTypeString]) {
        case SimpleNcs::delayFaultLiveAll:
        case SimpleNcs::delayFaultLivePlanetlab:
        case SimpleNcs::delayFaultSimulation:
            faultyDelay = true;
            break;
        default:
            faultyDelay = false;
    }

    PeerInfo* peerInfo =
        GlobalNodeListAccess().get()
        ->getPeerInfo(this->neighborCache->getOverlayThisNode().getIp());

    if(peerInfo == NULL) {
        throw cRuntimeError("No PeerInfo Found");
    }

    SimpleNodeEntry* entry = dynamic_cast<SimpleInfo*>(peerInfo)->getEntry();

    SimpleCoordsInfo::setDimension(entry->getDim());
    ownCoords = new SimpleCoordsInfo();

    for (uint8_t i = 0; i < entry->getDim(); i++) {
        ownCoords->setCoords(i, entry->getCoords(i) * 0.001);
    }
    ownCoords->setAccessDelay((entry->getRxAccessDelay() +
                               800 / entry->getRxBandwidth() +
                               entry->getTxAccessDelay() +
                               800 / entry->getTxBandwidth()) / 2.0);
}

AbstractNcsNodeInfo* SimpleNcs::createNcsInfo(const std::vector<double>& coords) const
{
    assert(coords.size() > 1);
    SimpleCoordsInfo* info = new SimpleCoordsInfo();

    uint8_t i;
    for (i = 0; i < coords.size() - 1; ++i) {
        info->setCoords(i, coords[i]);
    }
    info->setAccessDelay(coords[i]);

    return info;
}


Prox SimpleNcs::getCoordinateBasedProx(const AbstractNcsNodeInfo& abstractInfo) const
{
    if (faultyDelay) {
        return falsifyDelay(ownCoords->getDistance(abstractInfo) /* + Rx */);
    }
    return ownCoords->getDistance(abstractInfo) /* + Rx */;
}

simtime_t SimpleNcs::falsifyDelay(simtime_t oldDelay) const {

    // hash over string of oldDelay
    char delaystring[35];
    sprintf(delaystring, "%.30f", SIMTIME_DBL(oldDelay));

    CSHA1 sha1;
    uint8_t hashOverDelays[20];
    sha1.Reset();
    sha1.Update((uint8_t*)delaystring, 32);
    sha1.Final();
    sha1.GetHash(hashOverDelays);

    // get the hash's first 4 bytes == 32 bits as one unsigned integer
    uint32_t decimalhash = 0;
    for (int i = 0; i < 4; i++) {
        decimalhash += (unsigned int) hashOverDelays[i] *
                       (2 << (8 * (3 - i) - 1));
    }

    // normalize decimal hash value onto 0..1 (decimal number / 2^32-1)
    double fraction = (double) decimalhash / (uint32_t) ((2 << 31) - 1);

    // flip a coin if faulty rtt is larger or smaller
    char sign = (decimalhash % 2 == 0) ? 1 : -1;

    // get the error ratio according to the distributions in
    // "Network Coordinates in the Wild", Figure 7
    double errorRatio = 0;

    switch (delayFaultTypeMap[delayFaultTypeString]) {
        case delayFaultLiveAll:
            // Kumaraswamy, a=2.03, b=14, moved by 0.04 to the right
            errorRatio = pow((1.0 - pow(fraction, 1.0 / 14.0)), 1.0 / 2.03) + 0.04;
            break;

        case delayFaultLivePlanetlab:
            // Kumaraswamy, a=1.95, b=50, moved by 0.105 to the right
            errorRatio = pow((1.0 - pow(fraction, 1.0 / 50.0)), 1.0 / 1.95) + 0.105;
            break;

        case delayFaultSimulation:
            // Kumaraswamy, a=1.96, b=23, moved by 0.02 to the right
            errorRatio = pow((1.0 - pow(fraction, 1.0 / 23.0)), 1.0 / 1.96) + 0.02;
            //std::cout << "ErrorRatio: " << errorRatio << std::endl;
            break;

        default:
            break;
    }

    // If faulty rtt is smaller, set errorRatio to max 0.6
    errorRatio = (sign == -1 && errorRatio > 0.6) ? 0.6 : errorRatio;

    return oldDelay + sign * errorRatio * oldDelay;
}


