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
 * @file Vivaldi.h
 * @author Jesus Davila Marchena
 * @author Bernhard Heep
 */

#ifndef _VIVALDI_
#define _VIVALDI_


#include <vector>

#include <omnetpp.h>

#include <GlobalStatisticsAccess.h>
#include <NeighborCache.h>
#include <CoordinateSystem.h>

class TransportAddress;


class Vivaldi : public AbstractNcs
{
  private:
    //variables for storing the parameter from ned file
    bool enableHeightVector;
    uint32_t dimension;
  protected:
    // variable for storing the node coordinates / estimated error
    VivaldiCoordsInfo* ownCoords;

    double errorC;
    double coordC;

    bool showPosition;

    virtual void finishVivaldi();

    virtual void updateDisplay();

    virtual double calcError(const simtime_t& rtt, double dist, double weight);
    virtual double calcDelta(const simtime_t& rtt, double dist, double weight);

    //pointer to GlobalStatistics
    GlobalStatistics* globalStatistics;
    NeighborCache* neighborCache;

  public:
    virtual ~Vivaldi() { delete ownCoords; };

    virtual void init(NeighborCache* neighborCache);
    void processCoordinates(const simtime_t& rtt,
                            const AbstractNcsNodeInfo& nodeInfo);

    Prox getCoordinateBasedProx(const AbstractNcsNodeInfo& info) const;

    virtual AbstractNcsNodeInfo* getUnvalidNcsInfo() const { return new VivaldiCoordsInfo(enableHeightVector); };
    virtual AbstractNcsNodeInfo* createNcsInfo(const std::vector<double>& coords) const;

    const VivaldiCoordsInfo& getOwnNcsInfo() const { return *ownCoords; };
    const std::vector<double>& getOwnCoordinates() const { return ownCoords->getCoords(); };
    inline double getOwnError() const { return ownCoords->getError(); };
    inline double getOwnHeightVector() const { return ownCoords->getHeightVector(); };
};

#endif

