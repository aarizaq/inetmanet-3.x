/*****************************************************************************
 *
 * Copyright (C) 2007 Malaga University.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Alfonso Ariza Quintana.<aarizaq@uma.ea>
 *
 *****************************************************************************/
#define OMNETPP
#ifdef __KERNEL__
#include <linux/proc_fs.h>
#include <linux/module.h>
#undef DEBUG
#endif
#ifndef OMNETPP
#ifdef NS2
#include "ns-agent.h"
#endif
#else
#include <algorithm>    // std::equal
#include <vector>
#include "dsr-uu-omnetpp.h"
#endif
/* #include "debug.h" */
#include "dsr-rtc.h"
#include "dsr-srt.h"
#include "tbl.h"
#include "path-cache.h"

#ifdef __KERNEL__
#define DEBUG(f, args...)

static struct path_table PCH;

#define LC_PROC_NAME "dsr_pc"

#endif              /* __KERNEL__ */

#ifndef UINT_MAX
#define UINT_MAX 4294967295U   /* Max for 32-bit integer */
#endif

#ifndef INT_MAX
#define INT_MAX 2147483640L   /* Max for 32-bit integer */
#endif

/* LC_TIMER */

struct node_cache
{
    dsr_list_t l;
    struct tbl paths;
    struct in_addr addr;
};

struct path
{
    dsr_list_t l;
    unsigned int num_hop;
    int status;
    double cost;
    struct timeval expires;
    std::vector<struct in_addr> route;
    std::vector<unsigned int> vector_cost;
    path()
    {
        l.prev = l.next = NULL;
        num_hop = 0;
        status = 0;
        cost = 0;
        expires.tv_sec = 0;
        expires.tv_usec = 0;
    }
};

static inline void __ph_delete_route(struct path *rt)
{
    list_del(&rt->l);
}

static inline int crit_cache_query(void *pos, void *query)
{
    struct node_cache *p = (struct node_cache *)pos;
    struct in_addr *q = (struct in_addr *)query;

    if (p->addr.s_addr == q->s_addr)
        return 1;
    return 0;
}

static inline int crit_expire(void *pos, void *data)
{
    struct path *rt = (struct path *)pos;
    struct timeval now;

    gettime(&now);

    /* printf("ptr=0x%x exp_ptr=0x%x now_ptr=0x%x %s<->%s\n", (unsigned int)link, (unsigned int)&link->expires, (unsigned int)&now, print_ip(link->src->addr), print_ip(link->dst->addr)); */
    /*  fflush(stdout); */

    if (timeval_diff(&rt->expires, &now) <= 0)
    {
        __ph_delete_route(rt);
        return 1;
    }
    return 0;
}



static inline struct path *path_create()
{
    struct path *n;

    n = new path;

    if (!n)
        return NULL;
    return n;
};

static inline struct node_cache *node_cache_create(struct in_addr addr)
{
    struct node_cache *n;
    n = (struct node_cache *)MALLOC(sizeof(struct node_cache), GFP_ATOMIC);

    if (!n)
        return NULL;
    memset(n, 0, sizeof(struct node_cache));
    INIT_TBL(&n->paths,64);
    n->addr=addr;
    return n;
}

static inline struct node_cache *__node_cache_find(struct path_table *t, struct in_addr dest)
{
    struct tbl *n = GET_HASH(t,(uint32_t)dest.s_addr);
    return (struct node_cache *)__tbl_find(n, &dest, crit_cache_query);
}

bool mycompare (const struct in_addr &i, const struct in_addr &j) {
  return (i.s_addr == j.s_addr);
}

static int __ph_route_tbl_add(struct path_table *rt_t,
                              struct in_addr dst,int num_hops,const std::vector<struct in_addr> &rt_nodes, usecs_t timeout,
                              int status, double cost,const std::vector<unsigned int> &cost_vector)
{
    struct node_cache *n;
    struct path * rt;
    struct path * rt_aux;
    struct path * rt_aux2;
    int res;
    int exist=0;
    struct timeval now;
    long diff;
    int size;
    rt_aux2=NULL;
    dsr_list_t *pos;

    if (num_hops-1 != rt_nodes.size())
        opp_error("Size error");

    size = sizeof(struct in_addr)*(num_hops-1);
    n = __node_cache_find(rt_t,dst);

    if (n==NULL)
    {
        n=node_cache_create(dst);
        struct tbl *aux = GET_HASH(rt_t,(uint32_t)dst.s_addr);
        __tbl_add_tail(aux,&n->l);
    }

    gettime(&now);
    if (!tbl_empty(&n->paths))
    {
        list_for_each(pos, &n->paths.head)
        {
            rt = (struct path * ) pos;
            if ((int)rt->num_hop == num_hops)
            {
                if (rt->num_hop == 1)
                {
                    exist=-1;
                    break;
                }
                else if (std::equal(rt->route.begin(),rt->route.end(),rt_nodes.begin(),mycompare))
                {
                    exist=-1;
                    break;
                }
            }
        }
    }
    if (!exist)
    {
        rt= path_create();
        if (!rt)
            return -1;
        if (size>0)
            rt->route = rt_nodes;

        if (TBL_FULL(&n->paths)) // Table is full delete oldest
        {
            diff =0;
            list_for_each(pos, &n->paths.head)
            {
                rt_aux = (struct path * ) pos;
                if (diff<timeval_diff(&now,&rt_aux->expires))
                {
                    diff=timeval_diff(&now,&rt_aux->expires);
                    rt_aux2=rt_aux;
                }
            }
            if (rt_aux2)
            {
                __tbl_detach(&n->paths,&rt_aux2->l);
                delete (rt_aux2);
            }

        }
        __tbl_add_tail(&n->paths, &rt->l);
    }
    else
        res = 0;

    rt->status = status;

    if (cost!=-1)
        rt->cost = cost;

    rt->num_hop = num_hops;

    rt->vector_cost = cost_vector;

    gettime(&rt->expires);
    timeval_add_usecs(&rt->expires, timeout);
    return res;
}


struct dsr_srt *NSCLASS ph_srt_find(struct in_addr src,struct in_addr dst,int criteria,unsigned int timeout)
{
    dsr_list_t *tmp;
    dsr_list_t *pos;
    struct path * rt;
    struct timeval now;
    struct timeval *expires;

    std::vector<struct in_addr> *route;
    std::vector<unsigned int> *vector_cost;

    unsigned int num_hop;
    int i;
    double cost;
    bool find;
    struct dsr_srt *srt = NULL;
    struct in_addr myAddr = my_addr();

    struct node_cache *dst_node = __node_cache_find(&PCH,dst);
    if (dst_node==NULL)
        return NULL;


    if (tbl_empty(&dst_node->paths))
        return NULL;


    gettime(&now);
    route=NULL;
    list_for_each_safe(pos,tmp,&dst_node->paths.head)
    {
        rt = (struct path * ) pos;
        if (timeval_diff(&rt->expires, &now) <= 0)
        {
            __tbl_detach(&dst_node->paths,&rt->l);
            delete  rt;
            continue;
        }

        if (src.s_addr!=myAddr.s_addr)
        {
            // check
            find = false;
            for (i=0; i<(int)rt->num_hop; i++)
            {
                if (rt->route[i].s_addr==src.s_addr)
                    find = true;
            }

            if (!find)
                continue;
        }

        if (route==NULL && rt->status==VALID)
        {
            num_hop= rt->num_hop;
            cost= rt->cost;
            route = &rt->route;
            vector_cost = &rt->vector_cost;
            expires = &rt->expires;
        }
        else if (rt->cost<cost)
        {
            num_hop= rt->num_hop;
            cost= rt->cost;
            route = &rt->route;
            vector_cost = &rt->vector_cost;
            expires = &rt->expires;
        }
        else if (rt->cost==cost && (timeval_diff(&rt->expires, &now) <0) )
        {
            num_hop= rt->num_hop;
            cost= rt->cost;
            route = &rt->route;
            vector_cost = &rt->vector_cost;
            expires = &rt->expires;
        }
    }


    if (!route && num_hop!=1)
    {
        DEBUG("%s not found\n", print_ip(dst));
        return NULL;
    }

    /*  lc_print(&LC, lc_print_buf); */
    /*  DEBUG("Find SR to node %s\n%s\n", print_ip(dst_node->addr), lc_print_buf); */

    /*  DEBUG("Hops to %s: %u\n", print_ip(dst), dst_node->hops); */

    int k = (num_hop - 1);
#ifndef OMNETPP
    srt = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
                                   (k * sizeof(struct in_addr)),
                                   GFP_ATOMIC);

    if (!srt)
    {
        DEBUG("Could not allocate source route!!!\n");
        return NULL;
    }
#else
//    srt = (struct dsr_srt *)MALLOC(sizeof(struct dsr_srt) +
//                                   (k * sizeof(struct in_addr))+size_cost,GFP_ATOMIC);
    srt = new dsr_srt;


    if (!srt)
    {
        DEBUG("Could not allocate source route!!!\n");
        return NULL;
    }

    srt->dst = dst;
    srt->src = src;
    srt->laddrs = k * DSR_ADDRESS_SIZE;

    srt->addrs = *route;


    if (vector_cost<=0)
    {
        srt->cost.clear();
    }
    else
    {
        srt->cost  = *vector_cost;
        if (myAddr.s_addr==src.s_addr && etxActive)
        {
            if (!srt->cost.empty())
            {
                if (num_hop==1)
                {
                    IPv4Address dstAddr((uint32_t)dst.s_addr);
                    srt->cost[0] = (int) getCost(dstAddr);
                }
                else
                {
                    IPv4Address srtAddr((uint32_t)srt->addrs[0].s_addr);
                    srt->cost[0] = (int) getCost(srtAddr);
                }
            }
        }
    }

    if (etxActive)
        if (k!=0 && srt->cost.empty() == 0)
            DEBUG("Could not allocate source route!!!\n");
#endif


    /// ???????? must be ?????????????????
    if (timeout>0)
    {
        gettime(expires);
        timeval_add_usecs(expires, timeout);
    }
    return srt;
}

void NSCLASS
ph_srt_add(struct dsr_srt *srt, usecs_t timeout, unsigned short flags)
{
    int i, n;
    struct in_addr addr1, addr2;

    int j=0;
    int l;
    struct in_addr myaddr;
    struct dsr_srt *dsr_aux;
    double cost;
    unsigned int init_cost;
    bool is_first,is_last;
    int size_cost=0;

    if (!srt)
        return;

    n = srt->addrs.size();

    addr1 = srt->src;
    myaddr = my_addr();

    is_first = (srt->src.s_addr==myaddr.s_addr)?true:false;
    is_last = (srt->dst.s_addr==myaddr.s_addr)?true:false;

    if (!is_first && !is_last)
    {
        for (i = 0; i < n; i++)
        {
            j=i+1;
            if (srt->addrs[i].s_addr == myaddr.s_addr)
                break;
        }
    }

    if (!is_last)
    {
#ifdef OMNETPP
        if (etxActive && !srt->cost.empty())
        {
            if (j<n)
            {
                IPv4Address srtAddr((uint32_t)srt->addrs[j].s_addr);
                double cost = getCost(srtAddr);
                if (cost<0)
                    init_cost= INT_MAX;
                else
                    init_cost= (unsigned int) cost;
                srt->cost[j]= init_cost;
            }
            else if (n==0)
            {
                IPv4Address srtAddr((uint32_t)srt->dst.s_addr);
                double cost  = getCost(srtAddr);
                if (cost<0)
                    init_cost= INT_MAX;
                else
                    init_cost= (unsigned int) cost;
                srt->cost[0]= init_cost;
            }
        }
#endif
        size_cost = 0;

        for (i = j; i < n; i++)
        {
            addr2 = srt->addrs[i];
            cost = i+1-j;
#ifdef OMNETPP
            if (etxActive)
            {
                if (srt->cost.size()>0)
                {
                    cost = init_cost;
                    if (cost<0)
                        cost = 1e100;
                    for (l=j; l<i; l++)
                        cost += srt->cost[l];
                    size_cost = i-j+1;
                }
                if (i+1-j!=0 && size_cost==0)
                    DEBUG("Error !!!\n");
            }
#endif



            std::vector<unsigned int>auxCostVec;
            std::vector<struct in_addr>auxVect;

            if (etxActive)
            {
                for(int pos = j; pos <= i ; pos++)
                {
                    auxCostVec.push_back(srt->cost[pos]);
                }
            }
            for(int pos = j; pos < i; pos++)
                auxVect.push_back(srt->addrs[pos]);

            __ph_route_tbl_add(&PCH,addr2,i+1-j,auxVect, timeout, 0,cost, auxCostVec);
        }

        if ((j<n && n!=0) || (n==0))
        {
            addr2 = srt->dst;
            cost = n+1-j;
            size_cost = 0;
#ifdef OMNETPP
            if (etxActive)
            {
                if (srt->cost.size()>0)
                {
                    cost = init_cost;
                    for (l=j; l<n; l++)
                        cost += srt->cost[l];
                    size_cost = n-j+1;
                }
                if (n+1-j!=0 && size_cost==0)
                    DEBUG("Error !!!\n");
            }
#endif
            std::vector<unsigned int>auxCostVec;
            std::vector<struct in_addr>auxVect;

            if (etxActive)
            {
                for(unsigned int pos = j; pos < srt->cost.size(); pos++)
                {
                    auxCostVec.push_back(srt->cost[pos]);
                }
            }
            for(unsigned int pos = j; pos < srt->addrs.size(); pos++)
                auxVect.push_back(srt->addrs[pos]);


            __ph_route_tbl_add(&PCH,addr2,n+1-j,auxVect, timeout, 0, cost,auxCostVec);
        }
    }

    if (is_first)
        return;

    if (srt->flags & SRT_BIDIR&flags)
    {
        j=0;
        size_cost=0;
        if (!is_first)
        {
            dsr_aux  = dsr_srt_new_rev(srt);
            addr1 = dsr_aux->src;

            if (!is_last)
                for (i = 0; i < n; i++)
                {
                    j=i+1;
                    if (dsr_aux->addrs[i].s_addr == myaddr.s_addr)
                        break;
                }

            if (j<n)
            {

                IPv4Address srtAddr((uint32_t)dsr_aux->addrs[j].s_addr);
                double cost = getCost(srtAddr);
                if (cost<0)
                    init_cost= INT_MAX;
                else
                    init_cost= (unsigned int) cost;

                dsr_aux->cost[j]= init_cost;
            }

            for (i = j; i < n; i++)
            {
                addr2 = dsr_aux->addrs[i];
                cost = i+1-j;
#ifdef OMNETPP
                if (dsr_aux->cost.size()>0 && etxActive)
                {
                    cost = init_cost;
                    if (cost<0)
                        cost = 1e100;
                    for (l=j; l<i; l++)
                        cost += dsr_aux->cost[l];
                    size_cost = i-j;
                }
#endif
                std::vector<unsigned int>auxCostVec;
                std::vector<struct in_addr>auxVect;

                if (etxActive)
                 {
                     for(int pos = j; pos <=i; pos++)
                     {
                         auxCostVec.push_back(dsr_aux->cost[pos]);
                     }
                 }
                 for(int pos = j; pos < i ; pos++)
                     auxVect.push_back(dsr_aux->addrs[pos]);

                if (!etxActive)
                    auxVect.clear();
                __ph_route_tbl_add(&PCH,addr2,i+1-j,auxVect, timeout, 0, cost,auxCostVec);
            }
            cost = n+1-j;
#ifdef OMNETPP
            if (etxActive)
                if (dsr_aux->cost.size()>0)
                {
                    cost = init_cost;
                    for (l=j; l<n; l++)
                        cost += dsr_aux->cost[l];
                }
#endif
            addr2 = dsr_aux->dst;

            std::vector<unsigned int>auxCostVec;
            std::vector<struct in_addr>auxVect;

            if (etxActive)
            {
                for(unsigned int pos = j; pos < dsr_aux->cost.size(); pos++)
                {
                    auxCostVec.push_back(dsr_aux->cost[pos]);
                }
            }
            for(unsigned int pos = j; pos < dsr_aux->addrs.size(); pos++)
                auxVect.push_back(dsr_aux->addrs[pos]);

            __ph_route_tbl_add(&PCH,addr2,n+1-j,dsr_aux->addrs, timeout, 0, cost,auxCostVec);
            delete dsr_aux;
        }
    }
}

void NSCLASS
ph_srt_add_node(struct in_addr node, usecs_t timeout, unsigned short flags,unsigned int cost)
{

    struct dsr_srt *srt;
    struct in_addr myAddr;
    myAddr=my_addr();
    if (node.s_addr==myAddr.s_addr)
        return;
#ifndef OMNETPP
    srt = new dsr_srt;
#else
    //srt = (struct dsr_srt *) MALLOC((sizeof(struct dsr_srt) + size_cost), GFP_ATOMIC);
    srt = new dsr_srt;

    if (cost!=0)
    {
        srt->cost.push_back(cost);
    }
#endif

    if (!srt)
        return;
    srt->laddrs =0;
    srt->dst=node;
    srt->src=my_addr();

    ph_srt_add(srt,timeout,flags);
    delete srt;

}


void NSCLASS ph_srt_delete_node(struct in_addr src)
{
    dsr_list_t *tmp;
    dsr_list_t *tmp2;
    dsr_list_t *pos1;
    dsr_list_t *pos2;
    struct path * rt;
    struct timeval now;
    struct node_cache *n_cache;

    int i,j;

    gettime(&now);

    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        if (tbl_empty(&PCH.hash[i])) continue;
        list_for_each_safe(pos1,tmp,&PCH.hash[i].head)
        {
            n_cache = (struct node_cache *) pos1;
            if (tbl_empty(&n_cache->paths)) continue;
            if (n_cache->addr.s_addr==src.s_addr)
            {
                list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
                {
                    rt = (struct path*) pos2;
                    __tbl_detach(&n_cache->paths,&rt->l);
                    delete (rt);
                }
                continue;
            }
            list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
            {
                rt = (struct path*) pos2;
                if (timeval_diff(&rt->expires, &now) <= 0)
                {
                    __tbl_detach(&n_cache->paths,&rt->l);
                    delete (rt);
                    continue;
                }
                int long_route=rt->num_hop-1;
                for (j=0; j<long_route; j++)
                {
                    if (rt->route[j].s_addr==src.s_addr)
                    {
                        __tbl_detach(&n_cache->paths,&rt->l);
                        delete (rt);
                        break;
                    }
                }
            }
        }
    }
}



void NSCLASS ph_srt_delete_link(struct in_addr src,struct in_addr dst)
{
    dsr_list_t *tmp;
    dsr_list_t *tmp2;
    dsr_list_t *pos1;
    dsr_list_t *pos2;
    struct path * rt;
    struct timeval now;
    struct node_cache *n_cache;
    int i,j;

    gettime(&now);

    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        if (tbl_empty(&PCH.hash[i])) continue;
        list_for_each_safe(pos1,tmp,&PCH.hash[i].head)
        {
            n_cache = (struct node_cache *) pos1;
            if (tbl_empty(&n_cache->paths)) continue;
            list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
            {
                rt = (struct path*) pos2;
                if (timeval_diff(&rt->expires, &now) <= 0)
                {
                    __tbl_detach(&n_cache->paths,&rt->l);
                    delete (rt);
                    continue;
                }
                if (rt->num_hop==1 )
                {
                    if (n_cache->addr.s_addr==dst.s_addr)
                    {
                        __tbl_detach(&n_cache->paths,&rt->l);
                        delete (rt);
                    }
                    continue;
                }
                if (src.s_addr==my_addr().s_addr && rt->route[0].s_addr== dst.s_addr)
                {
                    __tbl_detach(&n_cache->paths,&rt->l);
                    delete (rt);
                    continue;
                }
                int long_route=rt->num_hop-2;
                int nextIt = 1;
                for (j=0; j<long_route; j++)
                {
                    if (rt->route[j].s_addr==src.s_addr && rt->route[j+1].s_addr==dst.s_addr)
                    {
                        __tbl_detach(&n_cache->paths,&rt->l);
                        delete (rt);
                        nextIt=0;
                        break;
                    }
                }

                if (nextIt && (rt->route[long_route].s_addr==src.s_addr && n_cache->addr.s_addr==dst.s_addr))
                {
                    __tbl_detach(&n_cache->paths,&rt->l);
                    delete (rt);
                }
            }
        }
    }
}



int NSCLASS path_cache_init(void)
{
    /* Initialize Graph */
    int i;
    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        INIT_TBL(&PCH.hash[i],100 );
    }
    return 0;
}

void NSCLASS path_cache_cleanup(void)
{
    dsr_list_t *tmp;
    dsr_list_t *tmp2;
    dsr_list_t *pos1;
    dsr_list_t *pos2;
    struct path * rt;
    struct node_cache *n_cache;
    int i;

    for (i=0; i<MAX_TABLE_HASH; i++)
    {
        if (tbl_empty(&PCH.hash[i])) continue;
        list_for_each_safe(pos1,tmp,&PCH.hash[i].head)
        {
            n_cache=(struct node_cache *)pos1;
            if (tbl_empty(&n_cache->paths))
            {
                __tbl_detach(&PCH.hash[i],&n_cache->l);
                FREE(n_cache);
                continue;
            }
            list_for_each_safe(pos2,tmp2,&n_cache->paths.head)
            {
                rt = (struct path *) pos2;
                __tbl_detach(&n_cache->paths,&rt->l);
                delete (rt);
            }
            __tbl_detach(&PCH.hash[i],&n_cache->l);
            FREE(n_cache);
        }
    }
}


