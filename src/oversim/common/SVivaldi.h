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
 * @file SVivaldi.h
 * @author Bernhard Heep
 */

#ifndef _SVIVALDI_
#define _SVIVALDI_


#include <omnetpp.h>

#include <Vivaldi.h>

class NeighborCache;


class SVivaldi : public Vivaldi
{
  private:
    double lossC;
    double effectiveSample;
    double loss;
    double lossResetLimit;

  protected:
    virtual double calcError(const simtime_t& rtt, double dist, double weight);
    virtual double calcDelta(const simtime_t& rtt, double dist, double weight);

  public:
    virtual void init(NeighborCache* neighborCache);
};

#endif
