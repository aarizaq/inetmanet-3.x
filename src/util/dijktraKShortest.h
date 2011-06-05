//
// Copyright (C) 2010 Alfonso Ariza, Malaga University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __DIJKSTRA_K_SHORTEST__H__
#define __DIJKSTRA_K_SHORTEST__H__

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "uint128.h"
typedef Uint128 NodeId;
#define InvalidId (Uint128)0

enum Metricts {aditiveMin,concaveMin,aditiveMax,concaveMax};
enum StateLabel {perm,tent};




class DijkstraKshortest
{
protected:
	class Cost
	{
	public:
	    double value;
	    Metricts metric;
	    Cost& operator=(const Cost& cost)
	    {
	        value=cost.value;
	        metric=cost.metric;
	        return *this;
	    }
	};
	typedef std::vector<Cost> CostVector;
    void addCost(CostVector &,const CostVector & a, const CostVector & b);
    static CostVector minimumCost;
    static CostVector maximumCost;
    friend bool operator < ( const DijkstraKshortest::CostVector& x, const DijkstraKshortest::CostVector& y );

    class SetElem
    {
    public:
        int iD;
        int idx;
        DijkstraKshortest::CostVector cost;
        SetElem()
        {
            iD=InvalidId;
            idx=-1;
        }
        SetElem& operator=(const SetElem& val)
        {
            this->iD=val.iD;
            this->idx=val.idx;
            this->cost.clear();
            for (unsigned int i=0;i<val.cost.size();i++)
                this->cost.push_back(val.cost[i]);
            return *this;
        }
    };
    friend bool operator < ( const DijkstraKshortest::SetElem& x, const DijkstraKshortest::SetElem& y );
    class State
    {
    public:
        CostVector cost;
        NodeId idPrev;
        int idPrevIdx;
        StateLabel label;
        State();
        State(const CostVector &cost);
        ~State();
        void setCostVector(const CostVector &cost);
    };
    struct Edge
    {
        NodeId last_node_; // last node to reach node X
        CostVector cost;
        Edge()
        {
            cost=maximumCost;
        }
        inline NodeId& last_node() {return last_node_;}
        inline double&   Cost()     {return cost[0].value;}
        inline double&   Delay()     {return cost[1].value;}
        inline double&   Bandwith()     {return cost[2].value;}
        inline double&   Quality()     { return cost[3].value;}
    };

    typedef std::vector<DijkstraKshortest::State> StateVector;
    typedef std::map<NodeId,DijkstraKshortest::StateVector> RouteMap;
    typedef std::map<NodeId, std::vector<DijkstraKshortest::Edge*> > LinkArray;
    LinkArray linkArray;
    RouteMap routeMap;
    NodeId rootNode;
    int K_LIMITE;
    CostVector limitsData;
public:
    DijkstraKshortest();
    ~DijkstraKshortest();
    virtual void setFromTopo(const cTopology *);
    virtual void setLimits(const std::vector<double> &);
    virtual void resetLimits(){limitsData.clear();}
    virtual void setKLimit(int val){if (val>0) K_LIMITE=val;}
    virtual void initMinAndMax();
    virtual void cleanLinkArray();
    virtual void addEdge (const NodeId & dest_node, const NodeId & last_node,double cost,double delay,double bw,double quality);
    virtual void setRoot(const NodeId & dest_node);
    virtual void run();
    virtual void runUntil (const NodeId &);
    virtual int getNumRoutes(const NodeId &nodeId);
    virtual bool getRoute(const NodeId &nodeId,std::vector<NodeId> &pathNode,int k=0);
};



inline bool operator < ( const DijkstraKshortest::SetElem& x, const DijkstraKshortest::SetElem& y )
{
    if (x.iD==y.iD && x.idx==y.idx)
        return false;
    for (unsigned int i =0;i<x.cost.size();i++)
    {
        switch(x.cost[i].metric)
        {
        case aditiveMin:
        case concaveMin:
            if (x.cost[i].value<y.cost[i].value)
                return true;
            break;
        case aditiveMax:
        case concaveMax:
            if (x.cost[i].value>y.cost[i].value)
                return true;
            break;
        }
    }
    return false;
}

inline bool operator < ( const DijkstraKshortest::CostVector& x, const DijkstraKshortest::CostVector& y )
{
    for (unsigned int i =0;i<x.size();i++)
    {
        switch(x[i].metric)
        {
        case aditiveMin:
        case concaveMin:
            if (x[i].value<y[i].value)
                return true;
            break;
        case aditiveMax:
        case concaveMax:
            if (x[i].value>y[i].value)
                return true;
            break;
        }
    }
    return false;
}

#endif
