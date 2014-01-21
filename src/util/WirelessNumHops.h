//
// Copyright (C) 2012 Univerdidad de Malaga.
// Author: Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef WIRELESSNUMHOPS_H_
#define WIRELESSNUMHOPS_H_

#include <cownedobject.h>
#include <vector>
#include <map>
#include <deque>
#include "Coord.h"
#include "MACAddress.h"
#include "IPv4Address.h"

class IInterfaceTable;
class IMobility;

class WirelessNumHops : public cOwnedObject
{
        struct nodeInfo
        {
            IMobility* mob;
            IInterfaceTable* itable;
            std::vector<MACAddress> macAddress;
            std::vector<IPv4Address> ipAddress;
        };
        struct LinkPair
        {
            int node1;
            int node2;
            double cost;
            LinkPair()
            {
                node1 = node2 = -1;
            }
            LinkPair(int i, int j)
            {
                node1 = i;
                node2 = j;
                cost = -1;
            }

            LinkPair(int i, int j,double c)
            {
                node1 = i;
                node2 = j;
                cost = c;
            }

            LinkPair& operator=(const LinkPair& val)
            {
                this->node1 = val.node1;
                this->node2 = val.node2;
                this->cost = val.cost;
                return *this;
            }
        };

        std::vector<nodeInfo> vectorList;
        std::map<MACAddress,int> related;
        std::map<IPv4Address,int> relatedIp;
        typedef std::map<MACAddress,std::deque<MACAddress> > RouteCache;
        typedef std::map<IPv4Address,std::deque<IPv4Address> > RouteCacheIp;

        typedef std::set<LinkPair> LinkCache;
        RouteCache routeCache;
        RouteCacheIp routeCacheIp;
        LinkCache linkCache;
    protected:
        enum StateLabel {perm,tent};
        class DijkstraShortest
        {
            public:
            class SetElem
            {
            public:
                int iD;
                unsigned int cost;
                double costAdd;
                double costMax;
                SetElem()
                {
                    iD = -1;
                }
                SetElem& operator=(const SetElem& val)
                {
                    this->iD=val.iD;
                    this->cost = val.cost;
                    this->costAdd = val.costAdd;
                    this->costMax = val.costMax;
                    return *this;
                }
                bool operator<(const SetElem& val)
                {
                    if (this->cost > val.cost)
                        return false;
                    if (this->cost < val.cost)
                        return true;
                    if (this->costAdd < val.costAdd)
                        return true;
                    if (this->costMax < val.costMax)
                       return true;
                    return false;
                }
                bool operator>(const SetElem& val)
                {
                    if (this->cost < val.cost)
                        return false;
                    if (this->cost > val.cost)
                        return true;
                    if (this->costAdd > val.costAdd)
                        return true;
                    if (this->costMax > val.costMax)
                       return true;
                    return false;
                }
            };
            class State
            {
            public:
                unsigned int cost;
                double costAdd;
                double costMax;
                int idPrev;
                StateLabel label;
                State();
                State(const unsigned int &cost);
                State(const unsigned int &cost, const double &cost1, const double &cost2);
                void setCostVector(const unsigned int &cost);
                void setCostVector(const unsigned int &cost, const double &cost1, const double &cost2);
            };

            struct Edge
            {
                int last_node_; // last node to reach node X
                unsigned int cost;
                double costAdd;
                double costMax;
                Edge()
                {
                    cost = 200000;
                    costAdd = 1e30;
                    costMax = 1e30;
                }
            };
        };

        typedef std::map<int,WirelessNumHops::DijkstraShortest::State> RouteMap;
        typedef std::vector<WirelessNumHops::DijkstraShortest::Edge*> LinkCon;

        typedef std::map<int, LinkCon> LinkArray;
        LinkArray linkArray;
        RouteMap routeMap;
        int rootNode;

        virtual void cleanLinkArray();
        virtual void addEdge (const int & dest_node, const int & last_node,unsigned int cost);
        virtual void addEdge (const int & originNode, const int & last_node,unsigned int cost, double costAdd, double costMax);
        virtual bool getRoute(const int &nodeId,std::deque<int> &);
        virtual bool getRouteCost(const int &nodeId,std::deque<int> &,double &costAdd, double &costMax);
        virtual void setRoot(const int & dest_node);
        virtual void runUntil (const int &);
        virtual int getIdNode(const MACAddress &add);
        virtual int getIdNode(const  IPv4Address &add);
        virtual std::deque<int> getRoute(int i);

        virtual bool findRoutePath(const int &dest,std::deque<int> &pathNode);
        virtual bool findRoutePathCost(const int &nodeId,std::deque<int> &pathNode,double &costAdd, double &costMax);

    public:
        friend bool operator < (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y );
        friend bool operator > (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y );
        friend bool operator == (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y );
        friend bool operator < ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y );
        friend bool operator > ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y );
        friend bool operator == ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y );

        virtual void fillRoutingTables(const double &tDistance);
        virtual void fillRoutingTablesWitCost(const double &tDistance);

        WirelessNumHops();
        virtual ~WirelessNumHops();
        virtual void reStart();
        virtual bool findRouteWithCost(const double &, const MACAddress &,std::deque<MACAddress> &, bool withCost, double &costAdd, double &costMax);
        virtual bool findRouteWithCost(const double &, const IPv4Address &,std::deque<IPv4Address> &, bool withCost, double &costAdd, double &costMax);

        virtual bool findRoute(const double &dist, const MACAddress &dest,std::deque<MACAddress> &pathNode)
        {
            double cost1,cost2;
            return findRouteWithCost(dist, dest,pathNode, false, cost1, cost2);
        }
        virtual bool findRoute(const double &dist, const IPv4Address &dest,std::deque<IPv4Address> &pathNode)
        {
            double cost1,cost2;
            return findRouteWithCost(dist, dest,pathNode, false, cost1, cost2);
        }

        virtual void setRoot(const MACAddress & dest_node)
        {
            setRoot(getIdNode(dest_node));
        }

        virtual void setRoot(const IPv4Address & dest_node)
        {
            setRoot(getIdNode(dest_node));
        }

        void runUntil (const MACAddress &target)
        {
            runUntil(getIdNode(target));
        }

        virtual void run();

        virtual unsigned int getNumRoutes() {return routeMap.size();}

        virtual void getRoute(int ,std::deque<IPv4Address> &pathNode);

        virtual void getRoute(int ,std::deque<MACAddress> &pathNode);

        virtual Coord getPos(const int &node);

        virtual Coord getPos(const IPv4Address &node)
        {
            return getPos(getIdNode(node));
        }

        virtual Coord getPos(const MACAddress &node)
        {
            return getPos(getIdNode(node));
        }

        virtual void getNeighbours(const IPv4Address &node, std::vector<IPv4Address>&, const double &distance);
        virtual void getNeighbours(const MACAddress &node, std::vector<MACAddress>&, const double &distance);
};


inline bool operator < ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y )
{
    if (x.iD != y.iD && x.cost < y.cost)
        return true;
    return false;
}

inline bool operator > ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y )
{
    if (x.iD != y.iD && x.cost > y.cost)
        return true;
    return false;
}

inline bool operator == (const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y )
{
    if (x.iD == y.iD)
        return true;
    return false;
}

inline bool operator < ( const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y )
{
    if (x.node1 != y.node1)
        return x.node1 < y.node1;
    return x.node2 < y.node2;
}


inline bool operator > ( const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y )
{
    if (x.node1 != y.node1)
       return (x.node1 > y.node1);
    return (x.node2 > y.node2);
}

inline bool operator == (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y )
{
    return (x.node1 == y.node1 &&  x.node2 == y.node2);
}

#endif /* WIRELESSNUMHOPS_H_ */
