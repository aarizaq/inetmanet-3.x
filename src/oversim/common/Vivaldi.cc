// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file Vivaldi.cc
 * @author Jesus Davila Marchena
 * @author Bernhard Heep
 */


#include <cfloat>

#include <NeighborCache.h>

#include "Vivaldi.h"


void Vivaldi::init(NeighborCache* neighborCache)
{
    this->neighborCache = neighborCache;

    errorC = neighborCache->par("vivaldiErrorConst");
    coordC = neighborCache->par("vivaldiCoordConst");
    dimension = neighborCache->par("vivaldiDimConst");
    enableHeightVector = neighborCache->par("vivaldiEnableHeightVector");
    showPosition = neighborCache->par("vivaldiShowPosition");

    // init variables
    VivaldiCoordsInfo::setDimension(dimension);
    ownCoords = new VivaldiCoordsInfo();

    for (uint32_t i = 0; i < dimension; i++) {
        ownCoords->setCoords(i, uniform(-.2, .2));
    }
    if (enableHeightVector) ownCoords->setHeightVector(0.0);

    WATCH(*ownCoords);

    globalStatistics = GlobalStatisticsAccess().get();
};

void Vivaldi::processCoordinates(const simtime_t& rtt,
                                 const AbstractNcsNodeInfo& nodeInfo)
{
    if (rtt <= 0.0) {
        std::cout << "Vivaldi::processCoordinates() called with rtt = "
                  << rtt << std::endl;
        return;
    }

    if (!dynamic_cast<const VivaldiCoordsInfo*>(&nodeInfo)) {
        throw cRuntimeError("Vivaldi coords needed!");
    }
    const VivaldiCoordsInfo& info =
        *(static_cast<const VivaldiCoordsInfo*>(&nodeInfo));

    // calculate weight
    double weight = (((ownCoords->getError() + info.getError()) == 0) ? 0 :
        (ownCoords->getError() / (ownCoords->getError() + info.getError())));

    // calculate distance
    double dist = ownCoords->getDistance(info).proximity;

    // ... own error
    ownCoords->setError(calcError(rtt, dist, weight));

    // delta
    double delta = calcDelta(rtt, dist, weight);

    // update local coordinates
    if (dist > 0) {
        for (uint8_t i = 0; i < dimension; i++) {
            ownCoords->setCoords(i, ownCoords->getCoords(i) +
                                    (delta * (SIMTIME_DBL(rtt) - dist)) *
                                    ((ownCoords->getCoords(i) - info.getCoords(i)) /
                                     dist));
        }
        if(enableHeightVector) {
            ownCoords->setHeightVector(ownCoords->getHeightVector() +
                                      (delta * (SIMTIME_DBL(rtt) - dist)));
        }
    }

    updateDisplay();
}


double Vivaldi::calcError(const simtime_t& rtt, double dist, double weight)
{
    double relErr = 0;
    if (rtt != 0) {
        //eSample computes the relative error for this sample
        relErr = fabs(dist - rtt) / rtt;
    }
    // update weighted moving average of local error
    return (relErr * errorC * weight) +
           ownCoords->getError() * (1 - errorC * weight);
}


double Vivaldi::calcDelta(const simtime_t& rtt, double dist, double weight)
{
    // estimates the delta factor
    return coordC * weight;
}


Prox Vivaldi::getCoordinateBasedProx(const AbstractNcsNodeInfo& abstractInfo) const
{
    return ownCoords->getDistance(abstractInfo);
}


AbstractNcsNodeInfo* Vivaldi::createNcsInfo(const std::vector<double>& coords) const
{
    assert(coords.size() > 1);
    VivaldiCoordsInfo* info = new VivaldiCoordsInfo();

    uint8_t i;
    for (i = 0; i < coords.size() - (enableHeightVector ? 2 : 1); ++i) {
        info->setCoords(i, coords[i]);
    }
    info->setError(coords[i++]);

    if (enableHeightVector) {
        info->setHeightVector(coords[i]);
    }

    return info;
}


void Vivaldi::updateDisplay()
{
    char buf[60];
    sprintf(buf, "xi[0]: %f xi[1]: %f ", ownCoords->getCoords(0),
            ownCoords->getCoords(1));
    neighborCache->getDisplayString().setTagArg("t", 0, buf);

    // show nodes at estimated position TODO
    if (showPosition) {
        for (uint32_t i = 0; i < dimension; i++)
            neighborCache->getParentModule()
                ->getDisplayString().setTagArg("p", i,
                                               ownCoords->getCoords(i) * 1000);
    }
}

void Vivaldi::finishVivaldi()
{
    globalStatistics->addStdDev("Vivaldi: Errori(ei)", ownCoords->getError());
}
