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
 * @file RandomChurn.h
 * @author Helge Backhaus
 */

#ifndef __RANDOMCHURN_H_
#define __RABDOMCHURN_H_

#include <ChurnGenerator.h>

/**
 * Random churn generating class
 */

class RandomChurn : public ChurnGenerator
{
    public:
        void handleMessage(cMessage* msg);
        void initializeChurn();
        ~RandomChurn();

    protected:
        void updateDisplayString();

    private:
        double creationProbability; //!< probability of creating a new overlay terminal
        double migrationProbability; //!< probability of migrating an overlay terminal
        double removalProbability; //!< probability of removing an overlay terminal
        double initialMean; //!< mean of update interval during initalization phase
        double initialDeviation; //!< deviation of update interval during initalization phase
        double targetMean; //!< mean of update interval after initalization phase
        double targetOverlayTerminalNum; //!< number of created terminals after init phase
        cMessage* churnTimer; /**< message to change the churn rate */
        cMessage* mobilityTimer; /**< message to schedule events */
        bool churnIntervalChanged; /**< indicates if targetMean changed. */
        double churnChangeInterval; /**< churn change interval */
        bool initAddMoreTerminals; //!< true, if we're still adding more terminals in the init phase

        GlobalStatistics* globalStatistics;
};

#endif
