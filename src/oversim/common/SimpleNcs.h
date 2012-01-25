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
 * @file SimpleNcs.h
 * @author Bernhard Heep
 */

#ifndef _SIMPLENCS_
#define _SIMPLENCS_


#include <vector>

#include <omnetpp.h>

#include <NeighborCache.h>
#include <CoordinateSystem.h>


class SimpleNcs : public AbstractNcs
{
  private:
    // variable for storing the node coordinates / estimated error
    SimpleCoordsInfo* ownCoords;

    //variables for storing the parameter from ned file
    uint32_t dimension;

    simtime_t falsifyDelay(simtime_t oldDelay) const;

    enum delayFaultTypeNum {
        delayFaultUndefined,
        delayFaultLiveAll,
        delayFaultLivePlanetlab,
        delayFaultSimulation
    };
    static std::map<std::string, delayFaultTypeNum> delayFaultTypeMap;
    static std::string delayFaultTypeString;
    static bool faultyDelay;

  protected:
    NeighborCache* neighborCache;

  public:
    SimpleNcs() { ownCoords = NULL; };
    virtual ~SimpleNcs() { delete ownCoords; };

    virtual void init(NeighborCache* neighborCache);

    Prox getCoordinateBasedProx(const AbstractNcsNodeInfo& info) const;

    virtual AbstractNcsNodeInfo* getUnvalidNcsInfo() const { return new SimpleCoordsInfo(); };
    virtual AbstractNcsNodeInfo* createNcsInfo(const std::vector<double>& coords) const;

    const AbstractNcsNodeInfo& getOwnNcsInfo() const { return *ownCoords; };
    const std::vector<double>& getOwnCoordinates() const { return ownCoords->getCoords(); };
};

#endif

