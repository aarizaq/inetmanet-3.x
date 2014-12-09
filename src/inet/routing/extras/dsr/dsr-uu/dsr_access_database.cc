#include "inet/routing/extras/dsr/dsr-uu/DsrDataBase.h"
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"
#include <stack>          // std::stack

namespace inet {

namespace inetmanet {

// Convenience methods

struct dsr_srt *DSRUU::ph_srt_find_map(struct in_addr src, struct in_addr dst, unsigned int timeout)
{


    PathCacheRoute route;
    PathCacheCost vector_cost;
    struct in_addr myAddr = my_addr();

    /// ???????? must be ????????????????? if timeout > 0 the time out will be reset
    if (etxActive)
    {
        std::vector<PathCacheRoute> result;
        std::vector<PathCacheCost> resultCost;
        if (!pathCacheMap.getPaths(L3Address(IPv4Address(dst.s_addr)), result, resultCost))
            return nullptr;
        double totalCost = 1e100;
        for (unsigned int i = 0; i < result.size(); i++)
        {
            double costRoute = 0;
            if (!resultCost.empty())
            {
                double cost;
                if (!result[i].empty())
                    cost = getCost(result[i].front().toIPv4());
                else
                    cost = getCost(IPv4Address(myAddr.s_addr));
                resultCost[i].front() = cost;
                for (PathCacheCost::iterator it = resultCost[i].begin(); it != resultCost[i].end();++it)
                    costRoute += (*it);
            }
            else
                costRoute = result[i].size()+1;
            if (costRoute < totalCost)
            {
                totalCost = costRoute;
                route = result[i];
            }
        }
        // reset winner timeout
        if (timeout > 0)
            pathCacheMap.setPathsTimer(L3Address(IPv4Address(dst.s_addr)), route,timeout);
    }
    else
    {
        double cost;
        if (!pathCacheMap.getPath(L3Address(IPv4Address(dst.s_addr)),route,cost,timeout))
            return nullptr;
    }

    // should refresh routes to intermediate nodes?
    if (par("refreshIntermediate").boolValue())
    {
        for (unsigned int i = 0; i < route.size()-1;i++)
            pathCacheMap.setPathsTimer(route[i],PathCacheRoute(route.begin(),route.begin()+1),timeout);
    }

    dsr_srt *srt = new dsr_srt;

    if (!srt)
    {
        DEBUG("Could not allocate source route!!!\n");
        return nullptr;
    }

    srt->dst = dst;
    srt->src = src;
    srt->laddrs = route.size() * DSR_ADDRESS_SIZE;
    for (unsigned int i = 0; i < route.size();i++)
    {
        struct in_addr auxAddr;
        auxAddr.s_addr = route[i].toIPv4().getInt();
        srt->addrs.push_back(auxAddr);
    }

    if (!pathCacheMap.isPathCacheEmpty() && nextPurge <= simTime())
    {
        pathCacheMap.purgePathCache();
        nextPurge = simTime()+ (double) timeout/1000000.0;
    }


#if 0
    if (etxActive)
    {
        for (unsigned int i = 0 ; i < vector_cost.size(); i++)
            srt->cost.push_back(vector_cost[i]);

        if (myAddr.s_addr == src.s_addr)
        {
            if (!srt->cost.empty())
            {
                if (!route.empty())
                {
                    IPv4Address dstAddr((uint32_t)dst.s_addr);
                    if (srt->cost.empty())
                        srt->cost.push_back((int) getCost(dstAddr));
                    else
                        srt->cost[0] = getCost(dstAddr);
                }
                else
                {
                    if (!srt->cost.empty())
                    {
                        IPv4Address srtAddr((uint32_t)srt->addrs[0].s_addr);
                        srt->cost[0] = (int) getCost(srtAddr);
                    }
                }
            }
        }
    }
#endif
    return srt;
}

void DSRUU::ph_srt_add_map(struct dsr_srt *srt, usecs_t timeout, unsigned short flags, bool oportunity)
{

    if (!srt)
        return;

    struct in_addr myaddr;
    myaddr = my_addr();

    if (pathCacheMap.isPathCacheEmpty())
    {
        nextPurge = simTime()+ (double) timeout/1000000.0;
    }

    L3Address myAddress(IPv4Address(myaddr.s_addr));
    L3Address sourceAddress(IPv4Address(srt->src.s_addr));
    L3Address destinationAddress(IPv4Address(srt->dst.s_addr));

    PathCacheRoute route;
    PathCacheRoute route2;
    PathCacheCost costVect;
    PathCacheCost costVect2;

    bool is_first, is_last;

    is_first = is_last = false;
    int pos = -1;

    if (srt->src.s_addr==myaddr.s_addr)
    {
        is_first = true;
        for (unsigned int i = 0; i < srt->addrs.size(); i++)
        {
            route.push_back(L3Address(IPv4Address(srt->addrs[i].s_addr)));
        }
        for (unsigned int i = 0; i < srt->cost.size(); i++)
        {
            costVect.push_back(srt->cost[i]);
        }
    }
    else if (srt->dst.s_addr==myaddr.s_addr)
    {
        is_last = true;
        for (unsigned int i = srt->addrs.size()-1 ; i >= 0; i--)
        {
            route2.push_back(L3Address(IPv4Address(srt->addrs[i].s_addr)));
        }
        for (unsigned int i = srt->cost.size()-1 ; i >= 0; i--)
        {
            costVect2.push_back(srt->cost[i]);
        }
    }
    else
    {
        if (!is_first && !is_last)
        {
            for (unsigned int i = 0; i < srt->addrs.size(); i++)
            {
                if (srt->addrs[i].s_addr == myaddr.s_addr)
                {
                    pos = (int)i;
                    break;
                }
            }
        }
    }
    if (pos != -1)
    {
        for (int i = 0; i < pos; i++)
            route.push_back(L3Address(IPv4Address(srt->addrs[i].s_addr)));

        if (!srt->cost.empty())
        for (int i = 0; i <= pos; i++)
            costVect.push_back(srt->cost[i]);

        for (int i = pos-1 ; i >= 0; i--)
            route2.push_back(L3Address(IPv4Address(srt->addrs[i].s_addr)));

        if (!srt->cost.empty())
        for (int i = pos ; i >= 0; i--)
            costVect2.push_back(srt->cost[i]);
    }

    if (oportunity)
    {
        std::stack<PathCacheRoute> Direct;
        std::stack<PathCacheRoute> Reverse;

        std::stack<PathCacheCost> DirectCost;
        std::stack<PathCacheCost> ReverseCost;

        if (!is_first && !is_last && pos == -1)
        {
            std::vector<unsigned int> neigh; // store array positions
            // my address is not present
            // try to find a neighbor
            for (unsigned int i = 0; i < srt->addrs.size(); i++)
            {
                PathCacheRoute routeAux;
                PathCacheCost costVectAux;
                double costAux;

                L3Address addr(IPv4Address(srt->addrs[i].s_addr));
                if (pathCacheMap.getPathCosVect(addr, routeAux, costVectAux, costAux, 0))
                {
                    if (routeAux.empty()) // is neighbor
                        neigh.push_back(i);
                }
            }
            for (unsigned int i = 0; i < neigh.size(); i++)
            {
                PathCacheRoute val;
                for (unsigned int j = neigh[i]; j < srt->addrs.size(); j++)
                {
                    L3Address addr(IPv4Address(srt->addrs[j].s_addr));
                    val.push_back(addr);
                }
                // reverse path
                Direct.push(val);
                val.clear();
                for (unsigned int j = neigh[i]; j >= 0; j--)
                {
                    L3Address addr(IPv4Address(srt->addrs[j].s_addr));
                    val.push_back(addr);
                }
                Reverse.push(val);
                if (etxActive)
                {
                    PathCacheCost valCost;
                    IPv4Address addr(srt->addrs[i].s_addr);
                    valCost.resize(1);
                    valCost[0] = getCost(addr);
                    for (unsigned int j = neigh[i] + 1; j < srt->cost.size(); j++)
                        valCost.push_back(srt->cost[j]);
                    DirectCost.push(valCost);
                    // reverse path
                    valCost.resize(1);
                    for (unsigned int j = neigh[i] + 1; j >= 0; j--)
                        valCost.push_back(srt->cost[j]);
                    ReverseCost.push(valCost);
                }
            }
        }
    }
    for (unsigned int i = 0; i < route.size(); i++)
    {
        PathCacheRoute subRoute(route.begin(),route.begin()+i);
        PathCacheCost subCost;
        if (etxActive && i+1 < costVect.size())
        {
            subCost.resize(i+1);
            std::copy(costVect.begin(),costVect.begin()+i+1,subCost.begin());
        }
        pathCacheMap.setPath(route[i],subRoute,subCost,subRoute.size()+1,0,timeout);
    }
    // now the whole route
    if (myAddress != destinationAddress)
        pathCacheMap.setPath(destinationAddress,route,costVect,route.size()+1,0,timeout);

// now we have the information,
    if (is_first)
        return;

    if (srt->flags & SRT_BIDIR&flags)
    {

        for (unsigned int i = 0; i < route2.size(); i++)
        {
            PathCacheRoute subRoute(route2.begin(),route2.begin()+i);
            PathCacheCost subCost;
            if (etxActive && i+1 < costVect2.size())
            {
                subCost.resize(i+1);
                std::copy(costVect2.begin(),costVect2.begin()+i+1,subCost.begin());            }

            pathCacheMap.setPath(route[i],subRoute,subCost,subRoute.size()+1,0,timeout);
        }
        // now the whole route
        if (myAddress != sourceAddress)
            pathCacheMap.setPath(sourceAddress,route2,costVect2,route2.size()+1,0,timeout);
    }
}

void DSRUU::ph_srt_add_node_map(struct in_addr node, usecs_t timeout, unsigned short flags,unsigned int cost)
{
    L3Address destinationAddress(IPv4Address(node.s_addr));
    PathCacheRoute route;
    PathCacheCost costVect;
    if (etxActive)
    {
        costVect.push_back(getCost(destinationAddress.toIPv4()));
    }
    pathCacheMap.setPath(destinationAddress,route,costVect,1,0,timeout);
}


void DSRUU::ph_srt_delete_node_map(struct in_addr src)
{

    L3Address destinationAddress(IPv4Address(src.s_addr));
    pathCacheMap.erasePathWithNode(destinationAddress);
}

void DSRUU::ph_srt_delete_link_map(struct in_addr src1,struct in_addr src2)
{

    L3Address addr1(IPv4Address(src1.s_addr));
    L3Address addr2(IPv4Address(src2.s_addr));
    struct in_addr myAddr = my_addr();
    if (src1.s_addr == myAddr.s_addr)
        pathCacheMap.erasePathWithLink(addr1,addr2,true);
    else
        pathCacheMap.erasePathWithLink(addr1,addr2);
    pathCacheMap.deleteEdge(addr1,addr2,true);
}

void DSRUU::ph_add_link_map(struct in_addr src, struct in_addr dst, usecs_t timeout, int status, int cost)
{
    L3Address sourceAddress(IPv4Address(src.s_addr));
    L3Address destinationAddress(IPv4Address(dst.s_addr));
    pathCacheMap.addEdge(sourceAddress,destinationAddress,1,timeout);
    pathCacheMap.addEdge(destinationAddress,sourceAddress,1,timeout);
    return;
}


void DSRUU::ph_srt_add_link_map(struct dsr_srt *srt, usecs_t timeout)
{
    if (!srt)
        return;
    L3Address sourceAddress(IPv4Address(srt->src.s_addr));
    L3Address destinationAddress(IPv4Address(srt->dst.s_addr));

    if (srt->addrs.empty())
    {
        pathCacheMap.addEdge(sourceAddress,destinationAddress,1,timeout);
        pathCacheMap.addEdge(destinationAddress,sourceAddress,1,timeout);
        return;
    }

    pathCacheMap.addEdge(sourceAddress,L3Address(IPv4Address(srt->addrs.front().s_addr)),1,timeout);
    pathCacheMap.addEdge(L3Address(IPv4Address(srt->addrs.front().s_addr)),sourceAddress,1,timeout);
    pathCacheMap.addEdge(destinationAddress,L3Address(IPv4Address(srt->addrs.back().s_addr)),1,timeout);
    pathCacheMap.addEdge(L3Address(IPv4Address(srt->addrs.back().s_addr)),destinationAddress,1,timeout);

    for (unsigned int i = 0; i < srt->addrs.size()-1; i++)
    {
        L3Address addr1(IPv4Address(srt->addrs[i].s_addr));
        L3Address addr2(IPv4Address(srt->addrs[i+1].s_addr));
        pathCacheMap.addEdge(addr1,addr2,1,timeout);
        pathCacheMap.addEdge(addr2,addr1,1,timeout);
    }
}

struct dsr_srt *DSRUU::ph_srt_find_link_route_map(struct in_addr src, struct in_addr dst, unsigned int timeout)
{
    PathCacheRoute route;
    PathCacheCost vector_cost;

    struct in_addr myAddr = my_addr();
    pathCacheMap.setRoot(L3Address(IPv4Address(myAddr.s_addr)));

    if(!pathCacheMap.getRoute(L3Address(IPv4Address(dst.s_addr)),route,timeout))
        return nullptr;

    dsr_srt *srt = new dsr_srt;

    if (!srt)
    {
        DEBUG("Could not allocate source route!!!\n");
        return nullptr;
    }

    srt->dst = dst;
    srt->src = src;
    srt->laddrs = route.size() * DSR_ADDRESS_SIZE;
    for (unsigned int i = 0; i < route.size();i++)
    {
        struct in_addr auxAddr;
        auxAddr.s_addr = route[i].toIPv4().getInt();
        srt->addrs.push_back(auxAddr);
    }
    return srt;
}


} // namespace inetmanet

} // namespace inet
