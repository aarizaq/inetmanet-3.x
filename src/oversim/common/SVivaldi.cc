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
 * @file SVivaldi.cc
 * @author Bernhard Heep
 */


#include <cfloat>

#include <NeighborCache.h>

#include "SVivaldi.h"


void SVivaldi::init(NeighborCache* neighborCache)
{
    Vivaldi::init(neighborCache);

    lossC = neighborCache->par("svivaldiLossConst");
    lossResetLimit =  neighborCache->par("svivaldiLossResetLimit");

    loss = 0;
    WATCH(loss);
}


double SVivaldi::calcError(const simtime_t& rtt, double dist, double weight)
{
    // get avg absolute prediction error
    double sum = neighborCache->getAvgAbsPredictionError();

    // update weighted moving average of local error
    return (sum * errorC) + ownCoords->getError() * (1 - errorC);
}


double SVivaldi::calcDelta(const simtime_t& rtt, double dist, double weight)
{
    // calculate loss factor
    loss = lossC + (1 - lossC) * loss;
    if(fabs(dist - SIMTIME_DBL(rtt)) > lossResetLimit) loss = 0.0;

    return coordC * weight * (1 - loss);
}
