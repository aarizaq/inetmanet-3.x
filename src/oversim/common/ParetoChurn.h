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
 * @file ParetoChurn.h
 * @author Helge Backhaus
 * @author Ingmar Baumgart
 * @author Stephan Krause
 */

#ifndef __PARETOCHURN_H_
#define __PARETOCHURN_H_

#include <ChurnGenerator.h>

class TransportAddress;
class GlobalStatistics;

/**
 * Lifetime churn based on shifted pareto distribution
 *
 * This class implements the curn model proposed in
 *
 * Yao, Z.; Leonard, D.; Wang, X. & Loguinov, D.
 * "Modeling Heterogeneous User Churn and Local Resilience of Unstructured P2P Networks"
 * Proceedings of the 2006 14th IEEE International Conference on Network Protocols, 2006. ICNP '06.
 * 2006, pp. 32--41
 *
 */

class ParetoChurn : public ChurnGenerator
{
public:
    void handleMessage(cMessage* msg);
    void initializeChurn();
    ParetoChurn() { initFinishedTimer = NULL; };
    ~ParetoChurn();

protected:
    struct node_stat {
        double l;
        double d;
        bool alive;
    };
    void updateDisplayString();
    void createNode(double lifetime, double meanLifetime, double meanDeadtime, bool initialize);
    void deleteNode(TransportAddress& addr, double meanLifetime, double meanDeadtime);

	/** implements a shifted pareto funcion
	 */
	double shiftedPareto(double a, double b, int rng=0);

	/** returns a shifted pareto function's beta param by given mean and alpha
	 *
	 * @param mean the wanted mean
	 * @param alpha the alph param of the shifted pareto (defaults to 3)
	 * @returns the beta parameter
	 */
	double betaByMean(double mean, double alpha=3);

	/** returns the individual mean life/dead time of a node
	 *
	 * @param mean the global mean life/dead time
	 * @returns the mean life/dead time of the node
	 */
	double individualMeanTime(double mean);

	/** returns a individual lifetime (or deadtime) based on a node's mean lifetime
	 *
	 * @param mean the node's men lifetime
	 * @returns the lifetime
	 */
	double individualLifetime(double mean);

	/** returns the resiidual lifetime (or deadtime) based on a node's mean lifetime
	 *
	 * @param mean the node's men lifetime
	 * @returns the residual lifetime
	 */
	double residualLifetime(double mean);

	void scheduleCreateNodeAt(double creationTime, double lifetime, double meanLifetime, double meanDeadtime);



private:
	GlobalStatistics* globalStatistics;         //*< pointer to GlobalStatistics module in this node */

	double initialMean; //!< mean of update interval during initalization phase
	double initialDeviation; //!< deviation of update interval during initalization phase
	double lifetimeMean; //!< mean node lifetime
	double deadtimeMean; //!< mean node deadtime

	simtime_t lastCreatetime;
	simtime_t lastDeletetime;

	cMessage* initFinishedTimer; /**< timer to signal end of init phase */
};

#endif
