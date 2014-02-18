/*
 * Copyright (C) Universidad de Málaga
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Alfonso Ariza,
 */

#ifndef _DSR_DATA_BASE_H
#define _DSR_DATA_BASE_H

#include <map>
#include <deque>
#include <omnetpp.h>
#include "ManetAddress.h"

typedef std::vector<ManetAddress> PathCacheRoute;
typedef std::vector<double> PathCacheCost;

class DsrDataBase : public cOwnedObject
{
    private:

        // path cache information
        class Path
        {
            public:
                unsigned int num_hop;
                int status;
                double cost;
                simtime_t expires;
                std::vector<ManetAddress> route;
                std::vector<double> vector_cost;
            public:
                void setNumHops(const unsigned int& p) {num_hop = p;}
                void setStatus(const int& p){status = p;}
                void setExpires(const simtime_t & p) {expires = p;}
                void setRoute(const std::vector<ManetAddress>& p) {route = p;}
                void setCostVector(const std::vector<double>& p) {vector_cost = p;}
                void setCost(double& p) {cost = p;}

                unsigned int getNumHops() {return num_hop;}
                int getStatus() {return status;}
                simtime_t getExpires() {return expires;}
                std::vector<ManetAddress> getRoute() {return route;}
                std::vector<double> getCostVector() {return vector_cost;}
                double getCost() {return cost;}

                Path()
                {
                    num_hop = 0;
                    status = 0;
                    cost = 0;
                }
                ~Path()
                {
                    route.clear();
                    vector_cost.clear();
                }
        };

        typedef std::deque<Path> PathsToDestination;
        typedef std::map<ManetAddress,PathsToDestination> PathsDataBase;
        PathsDataBase pathsCache;

        friend class DSRUU;
        friend std::ostream& operator<<(std::ostream& os, const PathsToDestination& e);

    public:
        bool getPaths(const ManetAddress &, std::vector<PathCacheRoute> &, std::vector<PathCacheCost>&);
        bool getPathCosVect(const ManetAddress &, PathCacheRoute &, PathCacheCost&, double  &,unsigned int = 0);
        bool getPath(const ManetAddress &, PathCacheRoute &, double  &,unsigned int = 0);
        void setPathsTimer(const ManetAddress &dest,const PathCacheRoute &route, const unsigned int &timeout);
        void setPathsTimerStatus(const ManetAddress &dest,const PathCacheRoute &route, int status, const unsigned int &timeout);
        void setPathStatus(const ManetAddress &dest,const PathCacheRoute &route, int status);
        void setPath(const ManetAddress &dest,const PathCacheRoute &route,const PathCacheCost& costVect,const double &cost, int status, const unsigned int &timeout);
        void deleteAddress(const ManetAddress &dest);
        void erasePathWithNode(const ManetAddress &dest);
        // if the last parameter is true also delete the routes with first node = addr2
        void erasePathWithLink(const ManetAddress &addr1,const ManetAddress &addr2,bool = false,bool = true);
        void cleanAllPaths() {pathsCache.clear();}
        DsrDataBase();
        virtual ~DsrDataBase();

        void purgePathCache();
        bool isPathCacheEmpty() {return pathsCache.empty();}

        void cleanAllDataBase() {
            routeCache.clear();
            pathsCache.clear();
            cleanLinkArray();
        }


        //////////////////////////////////////////
        // link cache data base
        /////////////////////////////////////////////
    protected:
        // link cache data base
        typedef std::map<ManetAddress,PathCacheRoute > RouteCache;
        RouteCache routeCache;

        enum StateLabel
        {
            perm, tent
        };
        class DijkstraShortest
        {
            public:
                struct Edge
                {
                        ManetAddress last_node_; // last node to reach node X
                        double costAdd;
                        simtime_t expires;
                        Edge()
                        {
                            costAdd = 1e30;
                        }
                };
                class SetElem
                {
                    public:
                        ManetAddress iD;
                        double costAdd;
                        SetElem()
                        {
                        }
                        SetElem& operator=(const SetElem& val)
                        {
                            this->iD = val.iD;
                            this->costAdd = val.costAdd;
                            return *this;
                        }
                        bool operator<(const SetElem& val)
                        {
                            if (this->costAdd < val.costAdd)
                                return true;
                            return false;
                        }
                        bool operator>(const SetElem& val)
                        {
                            if (this->costAdd > val.costAdd)
                                return true;
                            return false;
                        }
                };
                class State
                {
                    public:
                        double costAdd;
                        ManetAddress idPrev;
                        StateLabel label;
                        Edge *edge;
                        State();
                        State(const double &cost);
                        void setCostVector(const double &cost);
                };

        };
        typedef std::map<ManetAddress, DsrDataBase::DijkstraShortest::State> RouteMap;
        typedef std::deque<DsrDataBase::DijkstraShortest::Edge*> LinkCon;
        typedef std::map<ManetAddress, LinkCon> LinkArray;
        LinkArray linkArray;
        RouteMap routeMap;
        ManetAddress rootNode;
        void cleanLinkArray();
    public:
        virtual void cleanLinkChacheData();
        virtual void addEdge(const ManetAddress & dest_node, const ManetAddress & last_node, double cost, const unsigned int &);
        virtual void deleteEdge(const ManetAddress &, const ManetAddress &,bool bidirectional = false);
        virtual bool getRoute(const ManetAddress &nodeId, PathCacheRoute &, unsigned int exp);
        virtual bool getRouteCost(const ManetAddress &nodeId, PathCacheRoute &,double &,unsigned int);
        virtual void setRoot(const ManetAddress & dest_node);

        friend bool operator <(const DsrDataBase::DijkstraShortest::SetElem& x,
                const DsrDataBase::DijkstraShortest::SetElem& y);
        friend bool operator >(const DsrDataBase::DijkstraShortest::SetElem& x,
                const DsrDataBase::DijkstraShortest::SetElem& y);
        friend bool operator ==(const DsrDataBase::DijkstraShortest::SetElem& x,
                const DsrDataBase::DijkstraShortest::SetElem& y);

        virtual void run(const ManetAddress & =ManetAddress::ZERO);
};

inline bool operator <(const DsrDataBase::DijkstraShortest::SetElem& x,
        const DsrDataBase::DijkstraShortest::SetElem& y)
{
    if (x.iD != y.iD && x.costAdd < y.costAdd)

        return true;
    return false;
}

inline bool operator >(const DsrDataBase::DijkstraShortest::SetElem& x,
        const DsrDataBase::DijkstraShortest::SetElem& y)
{
    if (x.iD != y.iD && x.costAdd > y.costAdd)
        return true;
    return false;
}

inline bool operator ==(const DsrDataBase::DijkstraShortest::SetElem& x,
        const DsrDataBase::DijkstraShortest::SetElem& y)
{
    if (x.iD == y.iD)
        return true;
    return false;
}

#endif
