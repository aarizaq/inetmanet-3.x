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
 * @file LifetimeChurn.h
 * @author Helge Backhaus, Ingmar Baumgart
 */

#ifndef __LIFETIMECHURN_H_
#define __LIFETIMECHURN_H_

#include <ChurnGenerator.h>

class GlobalStatistics;
class TransportAddress;

/**
 * Lifetime based churn generating class
 */

class LifetimeChurn: public ChurnGenerator
{
public:
    void handleMessage(cMessage* msg);
    void initializeChurn();
    LifetimeChurn() { initFinishedTimer = NULL; };
    ~LifetimeChurn();

protected:
    void updateDisplayString();
    void createNode(simtime_t lifetime, bool initialize, int contextPos);
    void deleteNode(TransportAddress& addr, int contextPos);
    double distributionFunction();
    void scheduleCreateNodeAt(simtime_t creationTime, simtime_t lifetime,
                              int contextPos);

private:
    GlobalStatistics* globalStatistics; //*< pointer to GlobalStatistics module in this node */

    double initialMean; //!< mean of update interval during initialization phase
    double initialDeviation; //!< deviation of update interval during initialization phase
    double targetMean; //!< mean of update interval after initialization phase
    std::string lifetimeDistName; //!< name of the distribution function
    double lifetimeMean; //!< mean node lifetime
    double lifetimeDistPar1; //!< distribution function parameter

    cMessage* initFinishedTimer; /**< timer to signal end of init phase */

    std::vector<cObject*> contextVector; /**< context pointer vector */

    simtime_t lastCreate;
    simtime_t lastDelete;
};

#endif
