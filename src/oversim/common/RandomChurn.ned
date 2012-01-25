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

package inet.oversim.common;

//
// Random churn generating module
//
// @author Markus Mauch
//
simple RandomChurn extends ChurnGenerator
{
    parameters:
        @class(RandomChurn);
        volatile double targetMobilityDelay @unit(s); // node creation/removal/migration interval after init phase
        volatile double targetMobilityDelay2 @unit(s); // used to change targetMean
        double creationProbability; // probability that a new node gets created
        double migrationProbability; // probability that a node migrates
        double removalProbability; // probability that a node gets removed
        double churnChangeInterval @unit(s); // time between churn changes
}
