/*
 *\class       PASER_Routing_Table
 *@brief       Class  is an implementation of the routing table. The class provides a map of all existing routes. Each valid route will be automatically added to the kernel routing table.
 *
 *\authors     Eugen.Paul | Mohamad.Sbeiti \@paser.info
 *
 *\copyright   (C) 2012 Communication Networks Institute (CNI - Prof. Dr.-Ing. Christian Wietfeld)
 *                  at Technische Universitaet Dortmund, Germany
 *                  http://www.kn.e-technik.tu-dortmund.de/
 *
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ********************************************************************************
 * This work is part of the secure wireless mesh networks framework, which is currently under development by CNI
 ********************************************************************************/
#include "Configuration.h"
#ifdef OPENSSL_IS_LINKED
#include "PASER_Routing_Table.h"

PASER_Routing_Table::PASER_Routing_Table(PASER_Timer_Queue *tQueue,
        PASER_Neighbor_Table *nTable, PASER_Socket *pModul, PASER_Global *pGlobal) {
    timer_queue = tQueue;
    neighbor_table = nTable;
    paser_modul = pModul;
    paser_global = pGlobal;

//    route_to_gw = NULL;
}

PASER_Routing_Table::~PASER_Routing_Table() {
    for (std::map<ManetAddress, PASER_Routing_Entry*>::iterator it =
            route_table.begin(); it != route_table.end(); it++) {
        PASER_Routing_Entry *temp = it->second;
        delete temp;
    }
    route_table.clear();
}

void PASER_Routing_Table::init() {

}

void PASER_Routing_Table::destroy() {

}

PASER_Routing_Entry *PASER_Routing_Table::findAdd(struct in_addr addr) {
    for (std::map<ManetAddress, PASER_Routing_Entry*>::iterator it =
            route_table.begin(); it != route_table.end(); it++) {
        PASER_Routing_Entry *tempEntry = it->second;
        for (std::list<address_range>::iterator it2 = tempEntry->AddL.begin();
                it2 != tempEntry->AddL.end(); it2++) {
            address_range tempRange = (address_range) *it2;
            if (IPv4Address::maskedAddrAreEqual(tempRange.ipaddr.S_addr.getIPv4(),addr.S_addr.getIPv4(), tempRange.mask.S_addr.getIPv4()))
            {
                return tempEntry;
            }
        }
    }
    return NULL;
//    std::map<ManetAddress, PASER_Routing_Entry*>::iterator it = route_table.find(addr.s_addr);
//    if (it != route_table.end())
//    {
//        if (it->second)
//            return it->second;
//    }
//    return NULL;
}

/* Find a routing entry for a given destination address */
PASER_Routing_Entry *PASER_Routing_Table::findDest(struct in_addr dest_addr) {
    if (dest_addr.S_addr.getIPv4().getInt() == 0xFFFFFFFF) {
        return findBestGW();
//        return route_to_gw;
    }
    std::map<ManetAddress, PASER_Routing_Entry*>::iterator it = route_table.find(
            dest_addr.s_addr);
    if (it != route_table.end()) {
        if (it->second)
            return it->second;
    }
    return NULL;
}

PASER_Routing_Entry *PASER_Routing_Table::insert(struct in_addr dest_addr,
        struct in_addr nxthop_addr, PASER_Timer_Message * deltimer,
        PASER_Timer_Message * validtimer, u_int32_t seqnum, u_int32_t hopcnt,
        u_int32_t is_gw, std::list<address_range> AddL, u_int8_t *Cert) {
    EV<<"source_add"<<dest_addr.S_addr.getIPv4();
    EV<<"nexthop"<<nxthop_addr.S_addr.getIPv4();
    PASER_Routing_Entry *entry = new PASER_Routing_Entry();
    entry->AddL.assign(AddL.begin(), AddL.end());
    entry->Cert = Cert;
    entry->deleteTimer = deltimer;
    entry->validTimer = validtimer;
    entry->dest_addr = dest_addr;
    entry->hopcnt = hopcnt;
    entry->is_gw = is_gw;
    entry->isValid = 1;
    entry->nxthop_addr = nxthop_addr;
    EV << "seq INSERT = " << seqnum << "\n";
//    if(seqnum != 0){
//        EV << "Seq UPDATE\n";
    entry->seqnum = seqnum;
//    }

    route_table.insert(std::make_pair(dest_addr.s_addr, entry));
//	return entry;
//	if(route_to_gw != NULL && is_gw && route_to_gw->hopcnt<hopcnt){
//	    route_to_gw = entry;
//	}
//	else if(route_to_gw == NULL && is_gw){
//	    route_to_gw = entry;
//	}
    return entry;
}

PASER_Routing_Entry *PASER_Routing_Table::update(PASER_Routing_Entry *entry,
        struct in_addr dest_addr, struct in_addr nxthop_addr,
        PASER_Timer_Message * deltimer, PASER_Timer_Message * validtimer,
        u_int32_t seqnum, u_int32_t hopcnt, u_int32_t is_gw,
        std::list<address_range> AddL, u_int8_t *Cert) {
    u_int32_t oldSeq = entry->seqnum;
    if (entry) {
        std::map<ManetAddress, PASER_Routing_Entry*>::iterator it = route_table.find(
                entry->dest_addr.s_addr);
        if (it != route_table.end()) {
            if ((*it).second == entry) {
                oldSeq = (*it).second->seqnum;
//                if(entry == route_to_gw){
//                    route_to_gw = NULL;
//                }
                route_table.erase(it);
            } else
                opp_error("Error in PASER routing table");

        }
        delete entry;
    }

    entry = new PASER_Routing_Entry();
    entry->AddL.assign(AddL.begin(), AddL.end());
    entry->Cert = Cert;
    entry->deleteTimer = deltimer;
    entry->validTimer = validtimer;
    entry->dest_addr = dest_addr;
    entry->hopcnt = hopcnt;
    entry->is_gw = is_gw;
    entry->nxthop_addr = nxthop_addr;
    entry->isValid = 1;
    EV << "seq UPDATE = " << seqnum << "\n";
    if (seqnum != 0) {
        EV << "Seq UPDATE\n";
        entry->seqnum = seqnum;
    } else {
        entry->seqnum = oldSeq;
    }

    route_table.insert(std::make_pair(dest_addr.s_addr, entry));
//    if(route_to_gw != NULL && is_gw && route_to_gw->hopcnt<hopcnt){
//        route_to_gw = entry;
//    }
//    else if(route_to_gw == NULL && is_gw){
//        route_to_gw = entry;
//    }
    return entry;
}

void PASER_Routing_Table::delete_entry(PASER_Routing_Entry *entry) {
    if (!entry)
        return;

    std::map<ManetAddress, PASER_Routing_Entry*>::iterator it = route_table.find(
            entry->dest_addr.s_addr);
    if (it != route_table.end()) {
        if ((*it).second == entry) {
//            if(entry == route_to_gw){
//                route_to_gw = findBestGW();
//            }
            route_table.erase(it);
        } else
            opp_error("Error in PASER routing table");
    }
}

PASER_Routing_Entry *PASER_Routing_Table::getRouteToGw() {
//    return route_to_gw;
    return findBestGW();
}

PASER_Routing_Entry *PASER_Routing_Table::findBestGW() {
//    std::map<ManetAddress, PASER_Routing_Entry*>::iterator it = route_table.find(entry->dest_addr.s_addr);
    PASER_Routing_Entry* tempBestRouteToGW = NULL;
    u_int32_t bestMetric = 0;
//    EV << "try to find best route to GW, routingTable.size = " << route_table.size() << "\n" ;
    for (std::map<ManetAddress, PASER_Routing_Entry*>::iterator it =
            route_table.begin(); it != route_table.end(); it++) {
//        EV << "IP: " << (*it).second->dest_addr.S_addr.getIPv4().str() << " isGW: " << (int)((*it).second->is_gw) << " isValid: " << (int)((*it).second->isValid) << " metric: " << (int)(*it).second->hopcnt << "\n";
        if ((*it).second->is_gw && (*it).second->isValid
                && (bestMetric == 0 || bestMetric > (*it).second->hopcnt)) {
            tempBestRouteToGW = (*it).second;
            bestMetric = tempBestRouteToGW->hopcnt;
        }
    }
//    route_to_gw = tempBestRouteToGW;
    return tempBestRouteToGW;
}

std::list<PASER_Routing_Entry*> PASER_Routing_Table::getListWithNextHop(
        struct in_addr nextHop) {
    std::list<PASER_Routing_Entry*> returnList;
    for (std::map<ManetAddress, PASER_Routing_Entry*>::iterator it =
            route_table.begin(); it != route_table.end(); it++) {
        PASER_Routing_Entry *tempEntry = (*it).second;
        if (tempEntry->nxthop_addr.S_addr == nextHop.S_addr) {
            returnList.push_back(tempEntry);
        }
    }
    return returnList;
}

void PASER_Routing_Table::updateKernelRoutingTable(struct in_addr dest_addr,
        struct in_addr forw_addr, struct in_addr netmask, u_int32_t metric,
        bool del_entry, int ifIndex) {
    if (!del_entry) {
        EV << " dest_addr = " << dest_addr.S_addr.getIPv4() << "\n";
        EV << " forw_addr = " << forw_addr.S_addr.getIPv4() << "\n";
        PASER_Routing_Entry *tempRout = getRouteToGw();
        if (tempRout && tempRout->dest_addr.S_addr == dest_addr.S_addr) {
            PASER_Neighbor_Entry *tempNeigh = neighbor_table->findNeigh(
                    tempRout->nxthop_addr);
            if (tempNeigh
                    && tempNeigh->neighbor_addr.S_addr == forw_addr.S_addr) {
                //add default Route to Kernel Routing Table
                EV << " dest_addr = " << dest_addr.S_addr.getIPv4() << "\n";
                EV << " forw_addr = " << forw_addr.S_addr.getIPv4() << "\n";
                struct in_addr destAdd;
                destAdd.S_addr.set(IPv4Address::UNSPECIFIED_ADDRESS);
                struct in_addr destAddMask;
                destAddMask.S_addr.set(IPv4Address::UNSPECIFIED_ADDRESS);
                EV << "update KernelTable: destAddr: "<< destAdd.S_addr << ", destMask: "<< destAddMask.S_addr << "\n";
                //paser_modul->MY_omnet_chg_rte(destAdd.S_addr, forw_addr.S_addr, destAddMask.S_addr, metric + 1, true, ifIndex);
                paser_modul->MY_omnet_chg_rte(destAdd.S_addr, forw_addr.S_addr, destAddMask.S_addr, metric + 1, false, ifIndex);
            }
        }
    }
    paser_modul->MY_omnet_chg_rte(dest_addr.S_addr, forw_addr.S_addr,
            netmask.S_addr, metric, del_entry, ifIndex);
    if (del_entry) {
        EV << "Loesche Entry: " << dest_addr.S_addr.getIPv4().str()
                << ", gw: " << forw_addr.S_addr.getIPv4().str() << "\n";
    }
    if (metric != 1 || del_entry) {
        return;
    }
//    std::list<PASER_Routing_Entry*> tempList = getListWithNextHop(dest_addr);
//    for(std::list<PASER_Routing_Entry*>::iterator it = tempList.begin(); it != tempList.end(); it++){
//        PASER_Routing_Entry* tempEntry = (PASER_Routing_Entry*)*it;
//        paser_modul->MY_omnet_chg_rte(tempEntry->dest_addr.S_addr, tempEntry->nxthop_addr.S_addr, netmask.S_addr, tempEntry->hopcnt, false, ifIndex);
//        std::list<address_range> tempAddList = tempEntry->AddL;
//        for(std::list<address_range>::iterator it2 = tempAddList.begin(); it2!=tempAddList.end(); it2++){
//            address_range tempRange = (address_range)*it2;
//            updateKernelRoutingTable(tempRange.ipaddr, tempEntry->nxthop_addr, tempRange.mask, tempEntry->hopcnt+1, false, ifIndex);
//            EV << "updating AddList Range\n";
//        }
//    }
}

void PASER_Routing_Table::updateRoutingTableAndSetTableTimeout(
        std::list<address_range> addList, struct in_addr src_addr, int seq,
        X509 *cert, struct in_addr nextHop, u_int32_t metric, int ifIndex,
        struct timeval now, u_int32_t gFlag, bool trusted) {
    PASER_Routing_Entry *entry = findDest(src_addr);

    EV<<"source_add"<<src_addr.S_addr.getIPv4();
    EV<<"nexthop"<<nextHop.S_addr.getIPv4();

    PASER_Timer_Message *deletePack = NULL;
    PASER_Timer_Message *validPack = NULL;
    if (entry) {
        deletePack = entry->deleteTimer;
        validPack = entry->validTimer;
        if (validPack == NULL) {
            validPack = new PASER_Timer_Message();
            validPack->data = NULL;
            validPack->destAddr.S_addr = src_addr.S_addr;
            validPack->handler = ROUTINGTABLE_VALID_ENTRY;
            entry->validTimer = validPack;
        }
    } else {
        deletePack = new PASER_Timer_Message();
        deletePack->data = NULL;
        deletePack->destAddr.S_addr = src_addr.S_addr;
        deletePack->handler = ROUTINGTABLE_DELETE_ENTRY;
        validPack = new PASER_Timer_Message();
        validPack->data = NULL;
        validPack->destAddr.S_addr = src_addr.S_addr;
        validPack->handler = ROUTINGTABLE_VALID_ENTRY;
    }
    deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
    validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deletePack->timeout.tv_sec << "\n";
    EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deletePack);
    timer_queue->timer_add(validPack);

    if (entry != NULL) {
        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
                entry->nxthop_addr);
        EV << "update Route in Routing Table for src: "
                << src_addr.S_addr.getIPv4().str() << " over: "
                << nextHop.S_addr.getIPv4().str() << "\n";
        if (nEntry && nEntry->neighFlag) {
//            if(entry->hopcnt <= (metric + 1) && entry->isValid && seq!=0 && seq<=entry->seqnum){
//            if(entry->hopcnt <= (metric + 1) && entry->isValid && seq!=0 && (paser_global->isSeqNew(seq, entry->seqnum) || seq == entry->seqnum)){
            if (entry->hopcnt <= (metric + 1) && entry->isValid && seq != 0
                    && !paser_global->isSeqNew(entry->seqnum, seq)) {
                if (seq != 0) {
                    entry->seqnum = seq;
                }
                if (entry->Cert) {
                    X509_free((X509*) entry->Cert);
                    entry->Cert = (u_int8_t*) cert;
                } else if (!entry->Cert && cert) {
                    entry->Cert = (u_int8_t*) cert;
                } else {
                    X509_free(cert);

                }
                struct in_addr netmask;
                netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
                updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr,
                        netmask, entry->hopcnt, false, nEntry->ifIndex);
                return;
            } else {
                nEntry = neighbor_table->findNeigh(nextHop);
                struct in_addr netmask;
                netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
//                if(trusted){
                if(!(entry->dest_addr.S_addr ==  src_addr.S_addr) || !(entry->nxthop_addr.S_addr== nextHop.S_addr) )
                               {
                updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr,
                        netmask, metric + 1, true, nEntry->ifIndex);
                updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,
                        false, nEntry->ifIndex);
                               }
                else
                {
                                updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,
                                                     false, nEntry->ifIndex);
                            }
//                else{
//                    PASER_Neighbor_Entry *tempNeiEntry = neighbor_table->findNeigh( nextHop );
//                    if(tempNeiEntry && tempNeiEntry->neighFlag){
//                        updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr, netmask, metric + 1, true, nEntry->ifIndex);
//                        updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,false,nEntry->ifIndex);
//                    }
//                    else{
//                        updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr, netmask, metric + 1, true, nEntry->ifIndex);
//                    }
//                }
                update(entry, src_addr, nextHop, deletePack, validPack, seq,
                        metric + 1, entry->is_gw | gFlag, addList,
                        (u_int8_t*) cert);
                return;
            }
        }
//        struct in_addr netmask;
//        netmask.S_addr = IPAddress::ALLONES_ADDRESS.getInt();
//        if(trusted){
//            updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr, netmask, metric + 1,true, ifIndex);
//            updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,false, ifIndex);
//        }
//        else{
//            updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr, netmask, metric + 1,true, ifIndex);
//        }

//        if(nEntry && entry->hopcnt > (metric + 1)){
//            update(
//                    entry,
//                    src_addr,
//                    nextHop,
//                    deletePack,
//                    validPack,
//                    seq,
//                    metric + 1,
//                    entry->is_gw | gFlag,
//                    addList,
//                    (u_int8_t*)cert
//                    );
//            struct in_addr netmask;
//            netmask.S_addr = IPAddress::ALLONES_ADDRESS.getInt();
//    //        updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr, netmask, metric + 1,true, ifIndex);
//            updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,true, ifIndex);
//            updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,false, ifIndex);
//        }
        if (nEntry && (int) entry->seqnum == seq) {
            return;
        }
//        if(!nEntry){
        update(entry, src_addr, nextHop, deletePack, validPack, seq, metric + 1,
                entry->is_gw | gFlag, addList, (u_int8_t*) cert);
        struct in_addr netmask;
        netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        //        updateKernelRoutingTable(entry->dest_addr, entry->nxthop_addr, netmask, metric + 1,true, ifIndex);

        if(nEntry == NULL || nEntry->isValid){
        updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1, false,
                ifIndex);
        }
        else
             {updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1, true,
                     ifIndex);
             }
//        }
    } else {
        EV << "add new Route to Routing Table\n";
        EV<<"source_add"<<src_addr.S_addr.getIPv4();
        EV<<"nexthop"<<nextHop.S_addr.getIPv4();
        insert(src_addr, nextHop, deletePack, validPack, seq, metric + 1, gFlag,
                addList, (u_int8_t*) cert);
//        if(trusted){
        struct in_addr netmask;
        netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        PASER_Neighbor_Entry *tempEntry = neighbor_table->findNeigh(nextHop);
        if (tempEntry) {
            EV << "neighbor found!";
            EV<<"source_add"<<src_addr.S_addr.getIPv4();
                  EV<<"nexthop"<<nextHop.S_addr.getIPv4();
            updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,
                    false, tempEntry->ifIndex);
        } else {
            EV << "neighbor not found!";
            updateKernelRoutingTable(src_addr, nextHop, netmask, metric + 1,
                    false, ifIndex);
        }
//        }
    }
    return;
}

void PASER_Routing_Table::updateRoutingTableTimeout(struct in_addr src_addr,
        struct timeval now, int ifIndex) {
    PASER_Routing_Entry *entry = findDest(src_addr);

    PASER_Timer_Message *deletePack = NULL;
    PASER_Timer_Message *validPack = NULL;
    if (entry) {
        deletePack = entry->deleteTimer;
        validPack = entry->validTimer;
        if (validPack == NULL) {
            validPack = new PASER_Timer_Message();
            validPack->data = NULL;
            validPack->destAddr.S_addr = src_addr.S_addr;
            validPack->handler = ROUTINGTABLE_VALID_ENTRY;
            entry->validTimer = validPack;
        }
    } else {
        EV << "not Found PASER_Routing_Entry\n";
        return;
    }
    entry->isValid = 1;
    deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
    validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);
    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deletePack->timeout.tv_sec << "\n";
    EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deletePack);
    timer_queue->timer_add(validPack);
    struct in_addr netmask;
    netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
 //   updateKernelRoutingTable(src_addr, src_addr, netmask, 1, true, ifIndex);
    updateKernelRoutingTable(src_addr, src_addr, netmask, 1, false, ifIndex);
}

void PASER_Routing_Table::updateRoutingTableTimeout(struct in_addr src_addr,
        u_int32_t seq, struct timeval now) {
    PASER_Routing_Entry *entry = findDest(src_addr);

    PASER_Timer_Message *deletePack = NULL;
    PASER_Timer_Message *validPack = NULL;
    if (entry) {
        deletePack = entry->deleteTimer;
        validPack = entry->validTimer;
        if (validPack == NULL) {
            validPack = new PASER_Timer_Message();
            validPack->data = NULL;
            validPack->destAddr.S_addr = src_addr.S_addr;
            validPack->handler = ROUTINGTABLE_VALID_ENTRY;
            entry->validTimer = validPack;
        }
    } else {
        EV << "not Found PASER_Routing_Entry\n";
        return;
    }
    deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
    validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);
    EV << "new seq = " << seq << "\n";
    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deletePack->timeout.tv_sec << "\n";
    EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deletePack);
    timer_queue->timer_add(validPack);

    entry->seqnum = seq;
    entry->isValid = 1;
}

void PASER_Routing_Table::updateRoutingTable(struct timeval now,
        std::list<address_list> addList, struct in_addr nextHop, int ifIndex) {
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(nextHop);
    if (!nEntry || !nEntry->neighFlag) {
        return;
    }

    u_int32_t hopCount = addList.size() + 1;
    for (std::list<address_list>::iterator it = addList.begin();
            it != addList.end(); it++) {
        hopCount--;
        address_list tempList = (address_list) *it;
        PASER_Routing_Entry *rEntry = findDest(tempList.ipaddr);
        if (rEntry && rEntry->hopcnt <= hopCount) {
            for (std::list<address_range>::iterator it2 =
                    tempList.range.begin(); it2 != tempList.range.end();
                    it2++) {
                address_range tempRange = (address_range) *it2;
                updateKernelRoutingTable(tempRange.ipaddr, nextHop,
                        tempRange.mask, hopCount + 1, false, ifIndex);
                EV << "updating AddList Range\n";
            }
            continue;
        }
        if (!rEntry) {
            PASER_Timer_Message *deletePack = NULL;
            PASER_Timer_Message *validPack = NULL;
            deletePack = new PASER_Timer_Message();
            deletePack->data = NULL;
            deletePack->destAddr.S_addr = tempList.ipaddr.S_addr;
            deletePack->handler = ROUTINGTABLE_DELETE_ENTRY;
            validPack = new PASER_Timer_Message();
            validPack->data = NULL;
            validPack->destAddr.S_addr = tempList.ipaddr.S_addr;
            validPack->handler = ROUTINGTABLE_VALID_ENTRY;
            deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
            validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

            EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
                    << deletePack->timeout.tv_sec << "\n";
            EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
            timer_queue->timer_add(deletePack);
            timer_queue->timer_add(validPack);
            insert(tempList.ipaddr, nextHop, deletePack, validPack, 0, hopCount,
                    0, tempList.range, NULL);
        } else {
            PASER_Timer_Message *deletePack = NULL;
            PASER_Timer_Message *validPack = NULL;
            deletePack = rEntry->deleteTimer;
            validPack = rEntry->validTimer;
            if (validPack == NULL) {
                validPack = new PASER_Timer_Message();
                validPack->data = NULL;
                validPack->destAddr.S_addr = tempList.ipaddr.S_addr;
                validPack->handler = ROUTINGTABLE_VALID_ENTRY;
                rEntry->validTimer = validPack;
            }
            deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
            validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

            EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
                    << deletePack->timeout.tv_sec << "\n";
            EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
            timer_queue->timer_add(deletePack);
            timer_queue->timer_add(validPack);
            update(rEntry, tempList.ipaddr, nextHop, deletePack, validPack,
                    rEntry->seqnum, hopCount, rEntry->is_gw, tempList.range,
                    NULL);
        }

        struct in_addr netmask;
        netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        EV << "updating Kernel Routing Table from AddL, dest: "
                << tempList.ipaddr.S_addr.getIPv4().str() << ", metric: "
                << hopCount << "\n";
        updateKernelRoutingTable(tempList.ipaddr, nextHop, netmask, hopCount,
                false, ifIndex);

        for (std::list<address_range>::iterator it2 = tempList.range.begin();
                it2 != tempList.range.end(); it2++) {
            address_range tempRange = (address_range) *it2;
            updateKernelRoutingTable(tempRange.ipaddr, nextHop, tempRange.mask,
                    hopCount + 1, false, ifIndex);
            EV << "updating AddList Range\n";
        }
    }
}

void PASER_Routing_Table::deleteFromKernelRoutingTableNodesWithNextHopAddr(
        struct in_addr nextHop) {
    EV << "Loesche alle Knoten, die ueber "
            << nextHop.S_addr.getIPv4().str() << " erreichbar sind\n";
    std::list<PASER_Routing_Entry*> EntryList = getListWithNextHop(nextHop);
    EV << "Gefunden " << EntryList.size() << " Knoten, die geloescht werden\n";
    for (std::list<PASER_Routing_Entry*>::iterator it = EntryList.begin();
            it != EntryList.end(); it++) {
        PASER_Routing_Entry *tempEntry = (PASER_Routing_Entry *) *it;
        EV << "delete addr: "
                << tempEntry->dest_addr.S_addr.getIPv4().str() << "\n";
        for (std::list<address_range>::iterator it2 = tempEntry->AddL.begin();
                it2 != tempEntry->AddL.end(); it2++) {
            address_range addList = (address_range) *it2;
            EV << "    subnetz: " << addList.ipaddr.S_addr.getIPv4().str()
                    << "\n";
            updateKernelRoutingTable(addList.ipaddr, nextHop, addList.mask,
                    tempEntry->hopcnt + 1, true, 0);
        }
        in_addr tempMask;
        tempMask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
        updateKernelRoutingTable(tempEntry->dest_addr, tempEntry->nxthop_addr,
                tempMask, tempEntry->hopcnt, true, 0);
        PASER_Timer_Message *validTimer = tempEntry->validTimer;
        if (validTimer) {
            EV << "loesche Timer\n";
            if (timer_queue->timer_remove(validTimer)) {
                EV << "Timer geloescht\n";
            } else {
                EV << "Timer wurde nicht geloescht\n";
            }
            delete validTimer;
            tempEntry->validTimer = NULL;
        }
        tempEntry->isValid = 0;
    }

    PASER_Neighbor_Entry *nEntry = paser_global->getNeighbor_table()->findNeigh(
            nextHop);
    if (nEntry) {
        EV << "Loesche den Nachbar " << nextHop.S_addr.getIPv4().str()
                << " aus neighborTable\n";
        nEntry->isValid = 0;
        PASER_Timer_Message *validTimer = nEntry->validTimer;
        if (validTimer) {
            EV << "loesche Timer\n";
            if (timer_queue->timer_remove(validTimer)) {
                EV << "Timer geloescht\n";
            } else {
                EV << "Timer wurde nicht geloescht\n";
            }
            delete validTimer;
            nEntry->validTimer = NULL;
        }
    }

    PASER_Routing_Entry *rEntry = findDest(nextHop);
    if (!rEntry) {
        return;
    }
    EV << "Loesche den Knoten " << nextHop.S_addr.getIPv4().str()
            << " selbst\n";
    EV << "delete addr: " << rEntry->dest_addr.S_addr.getIPv4().str()
            << "\n";
    for (std::list<address_range>::iterator it2 = rEntry->AddL.begin();
            it2 != rEntry->AddL.end(); it2++) {
        address_range addList = (address_range) *it2;
        EV << "    subnetz: " << addList.ipaddr.S_addr.getIPv4().str()
                << "\n";
        updateKernelRoutingTable(addList.ipaddr, nextHop, addList.mask,
                rEntry->hopcnt + 1, true, 0);
    }
    in_addr tempMask;
    tempMask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
    updateKernelRoutingTable(rEntry->dest_addr, rEntry->nxthop_addr, tempMask,
            rEntry->hopcnt, true, 0);
    PASER_Timer_Message *validTimer = rEntry->validTimer;
    if (validTimer) {
        EV << "loesche Timer\n";
        if (timer_queue->timer_remove(validTimer)) {
            EV << "Timer geloescht\n";
        } else {
            EV << "Timer wurde nicht geloescht\n";
        }
        delete validTimer;
        rEntry->validTimer = NULL;
    }
    rEntry->isValid = 0;
}

void PASER_Routing_Table::updateRouteLifetimes(struct in_addr dest_addr) {
    // Update timer of source node
    PASER_Routing_Entry *rEntry = findDest(dest_addr);
    if (!rEntry) {
        rEntry = findAdd(dest_addr);
        EV << "suche Addr\n";
        if (!rEntry) {
            return;
        }
    }
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    PASER_Timer_Message *deletePack = rEntry->deleteTimer;
    PASER_Timer_Message *validPack = rEntry->validTimer;
    if (rEntry->isValid == 0) {
        return;
    }
    rEntry->isValid = 1;
    if (validPack == NULL) {
        EV << "validPack == NULL\n";
        validPack = new PASER_Timer_Message();
        validPack->data = NULL;
        validPack->destAddr.S_addr = dest_addr.S_addr;
        validPack->handler = ROUTINGTABLE_VALID_ENTRY;
        rEntry->validTimer = validPack;
    }
    deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
    validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

    EV << "ip Addr: " << validPack->destAddr.S_addr.getIPv4().str()
            << "\n";
    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deletePack->timeout.tv_sec << "\n";
    EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deletePack);
    timer_queue->timer_add(validPack);

    // Update NextHop information if and only if HELLO messages are not activated
    if (paser_global->isHelloActive()) {
        return;
    }

    // Update Timer of forwarding node entry in neighbor table
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
            rEntry->nxthop_addr);
    if (!nEntry || nEntry->isValid == 0) {
        return;
    }
    PASER_Timer_Message *NdeletePack = nEntry->deleteTimer;
    PASER_Timer_Message *NvalidPack = nEntry->validTimer;
    if (NvalidPack == NULL) {
        EV << "NvalidPack == NULL\n";
        PASER_Timer_Message* tempValidTime = new PASER_Timer_Message();
        tempValidTime->data = NULL;
        tempValidTime->destAddr.S_addr = nEntry->neighbor_addr.S_addr;
        tempValidTime->handler = NEIGHBORTABLE_VALID_ENTRY;
        nEntry->setValidTimer(tempValidTime);
        NvalidPack = tempValidTime;
    }
    nEntry->isValid = 1;
    NdeletePack->timeout = timeval_add(now, PASER_NEIGHBOR_DELETE_TIME);
    NvalidPack->timeout = timeval_add(now, PASER_NEIGHBOR_VALID_TIME);

    EV << "ip Addr: " << NvalidPack->destAddr.S_addr.getIPv4().str()
            << "\n";
    EV << "now: " << now.tv_sec << "\nNeighbor delete timeout: "
            << NdeletePack->timeout.tv_sec << "\n";
    EV << "Neighbor valid timeout: " << NvalidPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(NdeletePack);
    timer_queue->timer_add(NvalidPack);

    // Update Timer of forwarding node entry in the routing table
    PASER_Routing_Entry *rNeighborEntry = findDest(rEntry->nxthop_addr);
    if (!rNeighborEntry || rNeighborEntry->isValid == 0) {
        return;
    }
    PASER_Timer_Message *deleteRoutingPack = rNeighborEntry->deleteTimer;
    PASER_Timer_Message *validRoutingPack = rNeighborEntry->validTimer;
    if (validRoutingPack == NULL) {
        validRoutingPack = new PASER_Timer_Message();
        validRoutingPack->data = NULL;
        validRoutingPack->destAddr.S_addr = rNeighborEntry->nxthop_addr.S_addr;
        validRoutingPack->handler = ROUTINGTABLE_VALID_ENTRY;
        rNeighborEntry->validTimer = validRoutingPack;
    }
    rNeighborEntry->isValid = 1;
    deleteRoutingPack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
    validRoutingPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

    EV << "ip Addr: " << validRoutingPack->destAddr.S_addr.getIPv4().str()
            << "\n";
    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deleteRoutingPack->timeout.tv_sec << "\n";
    EV << "Route valid timeout: " << validRoutingPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deleteRoutingPack);
    timer_queue->timer_add(validRoutingPack);
}

std::list<address_list> PASER_Routing_Table::getNeighborAddressList(int ifNr) {
    std::list<address_list> liste;
    for (std::map<ManetAddress, PASER_Routing_Entry*>::iterator it =
            route_table.begin(); it != route_table.end(); it++) {
        PASER_Routing_Entry *rEntry = it->second;
        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
                rEntry->nxthop_addr);
        if (rEntry->hopcnt == 1 && nEntry != NULL && nEntry->neighFlag
                && nEntry->isValid) {
            address_list temp;
//            EV << "add IP to NeighborListe: " << nEntry->neighbor_addr.S_addr.getIPv4().str() << "\n";
            temp.ipaddr.S_addr = nEntry->neighbor_addr.S_addr;
            for (std::list<address_range>::iterator inIt = rEntry->AddL.begin();
                    inIt != rEntry->AddL.end(); inIt++) {
                temp.range.push_back((address_range) *inIt);
            }
            liste.push_back(temp);
        }
    }
    in_addr WlanAddrStruct;
//    int ifId = paser_modul->getIfIdFromIfIndex(nEntry->ifIndex);
    network_device *tempDevice = paser_global->getNetDevice();
    WlanAddrStruct.S_addr = tempDevice[ifNr].ipaddr.S_addr;
    address_list myAddrList;
    myAddrList.ipaddr = WlanAddrStruct;
    myAddrList.range = paser_global->getAddL();
    liste.push_back(myAddrList);
    EV << "listSize = " << liste.size() << "\n";
    return liste;
}

void PASER_Routing_Table::updateNeighborFromHELLO(address_list liste,
        u_int32_t seq, int ifIndex) {
    PASER_Routing_Entry *rEntry = findDest(liste.ipaddr);
    if (rEntry == NULL) {
        return;
    }
    PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
            rEntry->nxthop_addr);
    if (nEntry == NULL) {
        return;
    }
    //Get current time
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);

    //Update RouteTimeout of all nodes in "liste"
    PASER_Timer_Message *deletePack = NULL;
    PASER_Timer_Message *validPack = NULL;
    deletePack = rEntry->deleteTimer;
    validPack = rEntry->validTimer;
    if (validPack == NULL) {
        validPack = new PASER_Timer_Message();
        validPack->data = NULL;
        validPack->destAddr.S_addr = liste.ipaddr.S_addr;
        validPack->handler = ROUTINGTABLE_VALID_ENTRY;
        rEntry->validTimer = validPack;
    }
    deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
    validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

    EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
            << deletePack->timeout.tv_sec << "\n";
    EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
    timer_queue->timer_add(deletePack);
    timer_queue->timer_add(validPack);
    rEntry = update(rEntry, liste.ipaddr, rEntry->nxthop_addr, deletePack,
            validPack, rEntry->seqnum, 1, rEntry->is_gw, liste.range, NULL);

    struct in_addr netmask;
    netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
    EV << "updating Kernel Routing Table from AddL, dest: "
            << liste.ipaddr.S_addr.getIPv4().str() << ", metric: " << 1
            << "\n";
    updateKernelRoutingTable(liste.ipaddr, rEntry->nxthop_addr, netmask, 1,
            false, ifIndex);

    //update NeighborTimeout
    neighbor_table->updateNeighborTableTimeout(rEntry->nxthop_addr, now);
//    PASER_Timer_Message *deletePackNei = NULL;
//    PASER_Timer_Message *validPackNei = NULL;
//    deletePackNei = nEntry->deleteTimer;
//    validPackNei  = nEntry->validTimer;
//    if(validPackNei == NULL){
//        validPackNei = new PASER_Timer_Message();
//        validPackNei->data = NULL;
//        validPackNei->destAddr.S_addr = liste.ipaddr.S_addr;
//        validPackNei->handler = NEIGHBORTABLE_VALID_ENTRY;
//        nEntry->validTimer = validPackNei;
//    }
//    deletePackNei->timeout = timeval_add(now, PASER_NEIGHBOR_DELETE_TIME);
//    validPackNei->timeout = timeval_add(now, PASER_NEIGHBOR_VALID_TIME);
//
//    EV << "now: " <<now.tv_sec << "\nNeighbor delete timeout: " << deletePackNei->timeout.tv_sec << "\n";
//    EV << "Neighbor valid timeout: " << validPackNei->timeout.tv_sec << "\n";
//    timer_queue->timer_add( deletePackNei );
//    timer_queue->timer_add( validPackNei );
//    neighbor_table->update(nEntry, liste.ipaddr, deletePackNei, validPackNei, 1, 1, 0, liste.range, 0,NULL);

//Update AddList
    for (std::list<address_range>::iterator it2 = liste.range.begin();
            it2 != liste.range.end(); it2++) {
        address_range tempRange = (address_range) *it2;
        updateKernelRoutingTable(tempRange.ipaddr, rEntry->nxthop_addr,
                tempRange.mask, 2, false, ifIndex);
        EV << "updating AddList Range\n";
    }

}

void PASER_Routing_Table::updateRouteFromHELLO(address_list liste, int ifIndex,
        struct in_addr nextHop) {
    PASER_Routing_Entry *rEntry = findDest(liste.ipaddr);
    if (rEntry && rEntry->hopcnt == 1 && rEntry->isValid) {
        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
                rEntry->nxthop_addr);
        if (nEntry && nEntry->isValid && nEntry->neighFlag) {
            return;
        }
    }
//    if (rEntry && rEntry->hopcnt == 2 && rEntry->isValid) {
//        PASER_Neighbor_Entry *nEntry = neighbor_table->findNeigh(
//                rEntry->nxthop_addr);
//        if (nEntry && nEntry->isValid && nEntry->neighFlag) {
//            return;
//        }
//    }
    struct timeval now;
    paser_modul->MYgettimeofday(&now, NULL);
    if (!rEntry) {
        PASER_Timer_Message *deletePack = NULL;
        PASER_Timer_Message *validPack = NULL;
        deletePack = new PASER_Timer_Message();
        deletePack->data = NULL;
        deletePack->destAddr.S_addr = liste.ipaddr.S_addr;
        deletePack->handler = ROUTINGTABLE_DELETE_ENTRY;
        validPack = new PASER_Timer_Message();
        validPack->data = NULL;
        validPack->destAddr.S_addr = liste.ipaddr.S_addr;
        validPack->handler = ROUTINGTABLE_VALID_ENTRY;
        deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
        validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

        EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
                << deletePack->timeout.tv_sec << "\n";
        EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
        timer_queue->timer_add(deletePack);
        timer_queue->timer_add(validPack);
        insert(liste.ipaddr, nextHop, deletePack, validPack, 0, 2, 0,
                liste.range, NULL);
    } else {
        PASER_Timer_Message *deletePack = NULL;
        PASER_Timer_Message *validPack = NULL;
        deletePack = rEntry->deleteTimer;
        validPack = rEntry->validTimer;
        if (validPack == NULL) {
            validPack = new PASER_Timer_Message();
            validPack->data = NULL;
            validPack->destAddr.S_addr = liste.ipaddr.S_addr;
            validPack->handler = ROUTINGTABLE_VALID_ENTRY;
            rEntry->validTimer = validPack;
        }
        deletePack->timeout = timeval_add(now, PASER_ROUTE_DELETE_TIME);
        validPack->timeout = timeval_add(now, PASER_ROUTE_VALID_TIME);

        EV << "now: " << now.tv_sec << "\nRoute delete timeout: "
                << deletePack->timeout.tv_sec << "\n";
        EV << "Route valid timeout: " << validPack->timeout.tv_sec << "\n";
        timer_queue->timer_add(deletePack);
        timer_queue->timer_add(validPack);
        update(rEntry, liste.ipaddr, nextHop, deletePack, validPack,
                rEntry->seqnum, 2, rEntry->is_gw, liste.range, NULL);
    }

    struct in_addr netmask;
    netmask.S_addr.set(IPv4Address::ALLONES_ADDRESS);
    EV << "updating Kernel Routing Table from AddL, dest: "
            << liste.ipaddr.S_addr.getIPv4().str() << ", metric: " << 2
            << "\n";
    updateKernelRoutingTable(liste.ipaddr, nextHop, netmask, 2, false, ifIndex);

    for (std::list<address_range>::iterator it2 = liste.range.begin();
            it2 != liste.range.end(); it2++) {
        address_range tempRange = (address_range) *it2;
        updateKernelRoutingTable(liste.ipaddr, nextHop, tempRange.mask, 3,
                false, ifIndex);
        EV << "updating AddList Range\n";
    }
}
#endif
