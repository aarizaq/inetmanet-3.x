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
 * This class implements the curn model proposed in
 *
 * Yao, Z.; Leonard, D.; Wang, X. & Loguinov, D.
 * "Modeling Heterogeneous User Churn and Local Resilience of Unstructured
 * P2P Networks", Proceedings of the 2006 14th IEEE International Conference
 * on Network Protocols, 2006. ICNP '06, pp. 32--41
 *
 * @file ParetoChurn.cc
 * @author Helge Backhaus
 * @author Ingmar Baumgart
 * @author Stephan Krause
 */

#include "ParetoChurn.h"

#include "GlobalStatisticsAccess.h"
#include "Churn_m.h"
#include "GlobalStatistics.h"
#include <UnderlayConfigurator.h>
#include <algorithm>
#include <deque>

Define_Module(ParetoChurn);

void ParetoChurn::initializeChurn()
{
    Enter_Method_Silent();

    initialMean = par("initPhaseCreationInterval");
    initialDeviation = initialMean / 3;
    lifetimeMean = par("lifetimeMean");
    deadtimeMean = par("deadtimeMean");

    WATCH(lifetimeMean);
    WATCH(deadtimeMean);

    lastCreatetime = 0;
    lastDeletetime = 0;

    globalStatistics = GlobalStatisticsAccess().get();

    double initFinishedTime = initialMean * targetOverlayTerminalNum;

    // try to create a stable equilibrium of nodes in init phase
    //
    // check for each node if he is present in initial state
    // and roll individual mean life+dead times
    int liveNodes = 0;
    double sum_l_i = 0;
    std::deque<node_stat> node_stats;

    for (int i = 0; liveNodes < (int)par("targetOverlayTerminalNum"); i++) {

        double nodeLifetimeMean = individualMeanTime(lifetimeMean);
        globalStatistics->recordOutVector("ParetoChurn: Node individual "
                                          "mean lifetime", nodeLifetimeMean);
        double nodeDeadtimeMean = individualMeanTime(deadtimeMean);
        globalStatistics->recordOutVector("ParetoChurn: Node individual "
                                          "mean deadtime", nodeDeadtimeMean);
        sum_l_i += 1.0/(nodeLifetimeMean + nodeDeadtimeMean);
        node_stat nodeStat;
        nodeStat.l = nodeLifetimeMean;
        nodeStat.d = nodeDeadtimeMean;
        double nodeAvailability = nodeLifetimeMean/(nodeLifetimeMean
                                                    + nodeDeadtimeMean);

        globalStatistics->recordOutVector("Node availability", nodeAvailability);

        nodeStat.alive = uniform(0, 1) < nodeAvailability;
        if (nodeStat.alive) {
            liveNodes++;
        }
        node_stats.push_back( nodeStat );
    }

    // compute "stretch" factor to reach the configured average lifetime
    // this is neccessary as "individual" lifetime mean has to be bigger than
    // "global" lifetime mean, as short-lived nodes will factor in more often
    double mean_life = 0;
    int numNodes = node_stats.size();
    for( int i = 0; i < numNodes; ++i ){
        node_stat& stat = node_stats[i];
        mean_life += stat.l/( (stat.l + stat.d) * sum_l_i );
    }
    double stretch = lifetimeMean/mean_life;
    liveNodes = 0;

    // schedule creation for all (alive or dead) nodes
    for( int i = 0; i < numNodes; ++i ){
        node_stat& stat = node_stats.front();
        stat.l *= stretch;
        stat.d *= stretch;

        if( stat.alive ){
            double scheduleTime = truncnormal(initialMean*liveNodes, initialDeviation);
            scheduleCreateNodeAt(scheduleTime, initFinishedTime - scheduleTime
                                 + residualLifetime(stat.l),
                                 stat.l, stat.d);
            liveNodes++;
        } else {
            scheduleCreateNodeAt(initFinishedTime
                                 + residualLifetime(stat.d),
                                 individualLifetime(stat.l),
                                 stat.l, stat.d);
        }
        node_stats.pop_front();
    }

    initFinishedTimer = new cMessage("initFinishTimer");
    scheduleAt(initFinishedTime, initFinishedTimer);
}

void ParetoChurn::handleMessage(cMessage* msg)
{
    if (!msg->isSelfMessage()) {
        delete msg;
        return;
    }

    // init phase finished
    if (msg ==  initFinishedTimer) {
        underlayConfigurator->initFinished();
        cancelEvent(initFinishedTimer);
        delete initFinishedTimer;
        initFinishedTimer = NULL;

        return;
    }

    ParetoChurnMessage* churnMsg = check_and_cast<ParetoChurnMessage*>(msg);

    if (churnMsg->getCreateNode() == true) {
        createNode(churnMsg->getLifetime(), churnMsg->getMeanLifetime(),
                   churnMsg->getMeanDeadtime(), false);
    } else {
        deleteNode(churnMsg->getAddr(), churnMsg->getMeanLifetime(),
                   churnMsg->getMeanDeadtime());
    }

    delete msg;
}

void ParetoChurn::createNode(double lifetime, double meanLifetime,
                             double meanDeadtime, bool initialize)
{
    ParetoChurnMessage* churnMsg = new ParetoChurnMessage("DeleteNode");
    TransportAddress* ta = underlayConfigurator->createNode(type, initialize);
    churnMsg->setAddr(*ta);
    delete ta;
    churnMsg->setCreateNode(false);
    churnMsg->setMeanLifetime(meanLifetime);
    churnMsg->setMeanDeadtime(meanDeadtime);
    scheduleAt(std::max(simTime(), simTime() + lifetime
                   - underlayConfigurator->getGracefulLeaveDelay()), churnMsg);

    RECORD_STATS(globalStatistics->recordOutVector("ParetoChurn: Session Time",
                                                   lifetime));

    RECORD_STATS(globalStatistics->recordOutVector("ParetoChurn: "
                 "Time between creates", SIMTIME_DBL(simTime() - lastCreatetime)));

    lastCreatetime = simTime();
}

void ParetoChurn::deleteNode(TransportAddress& addr, double meanLifetime,
                             double meanDeadtime)
{
    // Kill node
    underlayConfigurator->preKillNode(NodeType(), &addr);

    RECORD_STATS(globalStatistics->recordOutVector("ParetoChurn: "
               "Time between deletes", SIMTIME_DBL(simTime() - lastDeletetime)));
    lastDeletetime = simTime();
    scheduleCreateNodeAt(SIMTIME_DBL(simTime()+individualLifetime(meanDeadtime)),
                         individualLifetime(meanLifetime), meanLifetime,
                         meanDeadtime);
}

void ParetoChurn::scheduleCreateNodeAt(double creationTime, double lifetime,
                                       double meanLifetime, double meanDeadtime)
{
    ParetoChurnMessage* churnMsg = new ParetoChurnMessage("CreateNode");
    churnMsg->setCreateNode(true);
    churnMsg->setLifetime(lifetime);
    churnMsg->setMeanLifetime(meanLifetime);
    churnMsg->setMeanDeadtime(meanDeadtime);
    scheduleAt(creationTime, churnMsg);
}

double ParetoChurn::betaByMean(double mean, double alpha)
{
    return 1/(mean*(alpha -1));
}

double ParetoChurn::shiftedPareto(double a, double b, int rng)
{
    // What OMNET calles "pareto_shifted" in reality is a gerneralized pareto,
    // not a shifted pareto...
    return (pareto_shifted(a, b, 0, rng)/b - 1) / b;
}

double ParetoChurn::individualMeanTime(double mean)
{
//    return shiftedPareto(3, betaByMean(mean*exp(1)));
    return shiftedPareto(3, betaByMean(mean));
}

double ParetoChurn::individualLifetime(double mean)
{
    return shiftedPareto(3, betaByMean(mean));
}

// Residual lifetime is shifted pareto with the same beta, but
// decreased alpha i.e. beta calculation is based on "old alpha" (in
// this case 3), while "actual alpha" is 2
double ParetoChurn::residualLifetime(double mean)
{
    return shiftedPareto(2, betaByMean(mean));
}

void ParetoChurn::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "pareto churn");
    getDisplayString().setTagArg("t", 0, buf);
}

ParetoChurn::~ParetoChurn() {
    // destroy self timer messages
    cancelAndDelete(initFinishedTimer);
}
