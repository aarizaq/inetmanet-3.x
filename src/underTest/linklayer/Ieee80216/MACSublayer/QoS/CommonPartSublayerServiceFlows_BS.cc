#include "CommonPartSublayerServiceFlows_BS.h"

Define_Module(CommonPartSublayerServiceFlows_BS);

CommonPartSublayerServiceFlows_BS::CommonPartSublayerServiceFlows_BS()
{
    pending_requests = 0;

    availableDatarate[ldUPLINK] = 0;
    availableDatarate[ldDOWNLINK] = 0;
}

CommonPartSublayerServiceFlows_BS::~CommonPartSublayerServiceFlows_BS()
{

}

void CommonPartSublayerServiceFlows_BS::initialize()
{
    //CommonPartSublayerServiceFlows::initialize();

    controlPlaneIn = findGate("controlPlaneIn");
    controlPlaneOut = findGate("controlPlaneOut");

    allowed_connections = 1024; //TODO in omnetpp.ini übertragen!

    // assign initial lower boundaries for CID ranges according to standard
    cur_max_basic_cid = 1;
    cur_max_primary_cid = allowed_connections + 1;
    cur_max_secondary_cid = 2 * allowed_connections;

    cur_max_sfid = 1;

    lower_bound_for_BE_traffic = par("lower_bound_for_BE_traffic");
    upper_bound_for_BE_grant = par("upper_bound_for_BE_grant");

// cModule *module = parentModule()->submodule("cp_authorization");
// cps_auth = check_and_cast<CommonPartSublayerAuthorizationModule*>(module);

    updateDisplay();
}

void CommonPartSublayerServiceFlows_BS::handleMessage(cMessage *msg)
{
    Ieee80216ManagementFrame* manFrame = dynamic_cast<Ieee80216ManagementFrame*>(msg); // Empfangendes Paket ist eine IEEE802.16e Frame
    if (!manFrame)              // Wenn nicht Fehlermeldung ausgeben
        error("Message (%s) %s from ControlPlane is not an IEEE 802.16e MAC management frame",
              msg->getClassName(), msg->getName());

    switch (manFrame->getManagement_Message_Type())
    {
    case ST_DSA_REQ:
        handle_DSA_REQ(check_and_cast<Ieee80216_DSA_REQ*>(manFrame));
        break;

    case ST_DSA_RSP:
        handle_DSA_RSP(check_and_cast<Ieee80216_DSA_RSP*>(manFrame));
        break;

    case ST_DSA_ACK:
        handle_DSA_ACK(check_and_cast<Ieee80216_DSA_ACK*>(manFrame));
        break;

    default:
        ev << "\n\nunidentified frame\n\n";
    }
}

/**
 * Creates a DSA-REQ for BS-initiated ServiceFlow creation.
 */
void CommonPartSublayerServiceFlows_BS::createAndSendNewDSA_REQ(int prim_management_cid,
                                                                ServiceFlow* requested_sf,
                                                                ip_traffic_types type)
{
    Enter_Method("createAndSendNewDSA_REQ()");

    if (checkQoSParams(requested_sf->provisioned_parameters, ldDOWNLINK))
    {
        ev << "BS-DSA: Creating new connection...\n";

        Ieee80216_DSA_REQ *dsa_req = new Ieee80216_DSA_REQ("DSA_REQ");
        dsa_req->setCID(prim_management_cid);
        dsa_req->setTraffic_type(type);
        requested_sf->CID = getNewManagementCID(SECONDARY);
        requested_sf->SFID = getFreeSFID();
        requested_sf->traffic_type = type;
        dsa_req->setNewServiceFlow(*requested_sf);

        // adding the IDs directly to the maps makes creation of the IDs easier
        // map_connections[dsa_req->getNewServiceFlow().CID] = dsa_req->getNewServiceFlow().SFID;
        // map_serviceFlows[dsa_req->getNewServiceFlow().SFID] = *(new ServiceFlow());

        send(dsa_req, controlPlaneOut);
    }
    else
    {
        // what to do, if the BS rejects?
    }

    updateDisplay();
}

/**
 * Incoming Request from MS  (SS-initiated DSA).
 * Immediately sends DSX-RVD message and builds a DSA-RSP.
 */
void CommonPartSublayerServiceFlows_BS::handle_DSA_REQ(Ieee80216_DSA_REQ* dsa_req)
{
    ev << "Incoming DSA-REQ from CID: [" << dsa_req->getCID() << "]\n";
    // answer with a DSA-RSP immediately!
    //ev << "CPin: "<< controlPlaneIn << ", CPout: "<< controlPlaneOut << "\n";
    ev << "Immediately sending DSX-RVD to Gate: " << controlPlaneOut << "\n";
    Ieee80216_DSX_RVD *dsx_rvd = new Ieee80216_DSX_RVD("DSX-RVD");
    dsx_rvd->setCID(dsa_req->getCID());

    send(dsx_rvd, controlPlaneOut);

    // the ServiceFlow has been authorized
    //if ( cps_auth->checkQoSParams( dsa_req->getNewServiceFlow().provisioned_parameters, ldUPLINK ) ) {
    if (checkQoSParams(dsa_req->getNewServiceFlow().provisioned_parameters, ldUPLINK))
    {
        Ieee80216_DSA_RSP *dsa_rsp;
        dsa_rsp =
            build_DSA_RSP(dsa_req->getCID(), &dsa_req->getNewServiceFlow(),
                          (ip_traffic_types) dsa_req->getTraffic_type());
        dsa_rsp->getNewServiceFlow().CID = getNewManagementCID(SECONDARY);
        dsa_rsp->getNewServiceFlow().SFID = getFreeSFID();
        dsa_rsp->getNewServiceFlow().traffic_type = (ip_traffic_types) dsa_req->getTraffic_type();
        dsa_rsp->getNewServiceFlow().link_type = ldUPLINK;

        // adding the IDs directly to the maps makes creation of the IDs easier
//  map_connections[dsa_rsp->getNewServiceFlow().CID] = dsa_rsp->getNewServiceFlow().SFID;
//  map_serviceFlows[dsa_rsp->getNewServiceFlow().SFID] = *(new ServiceFlow());

        send(dsa_rsp, controlPlaneOut);
    }

    // the ServiceFlow has been rejected
    else
    {
        Ieee80216_DSA_RSP *dsa_rsp = new Ieee80216_DSA_RSP("DSA-RSP reject");
        dsa_rsp->setCID(dsa_req->getCID());
        dsa_rsp->setTraffic_type(dsa_req->getTraffic_type());
        dsa_rsp->setRejected(true);

        send(dsa_rsp, controlPlaneOut);
    }

    delete dsa_req;

    updateDisplay();
}

/**
 * Incoming Response from MS  (BS-initiated DSA).
 * Stores the SS-accepted connection request and replies
 * with a DSA-ACK.
 */
void CommonPartSublayerServiceFlows_BS::handle_DSA_RSP(Ieee80216_DSA_RSP *dsa_rsp)
{
    ev << "Incoming DSA-RSP from CID: [" << dsa_rsp->getCID() << "]\n";

    ServiceFlow new_sf = dsa_rsp->getNewServiceFlow();
    new_sf.link_type = ldDOWNLINK;

    if (new_sf.state == SF_ACTIVE)
        new_sf.active_parameters = new sf_QoSParamSet(*(new_sf.admitted_parameters));

    // the SS accepts the connection, so we can add it to the maps
    map_connections[new_sf.CID] = new_sf.SFID;
    map_serviceFlows[new_sf.SFID] = new_sf;

    // also add the connection to the stations own connections map
    // this way, the station can later be identified by any CID
    structMobilestationInfo *str_ms = lookupMS(dsa_rsp->getMac_address());
    if (str_ms != NULL)
    {
        str_ms->map_own_connections[new_sf.CID] = new_sf.CID;
    }
    else
        error("Station not found by MAC address");

    Ieee80216_DSA_ACK *dsa_ack;
    ev << "DSA-RSP for traffic type: " << dsa_rsp->getTraffic_type() << "\n";
    dsa_ack =
        build_DSA_ACK(dsa_rsp->getCID(), &new_sf, (ip_traffic_types) dsa_rsp->getTraffic_type());
    send(dsa_ack, controlPlaneOut);

    ev << "BS-DSA: New connection established! (CID=" << new_sf.CID << " | SFID=" << new_sf.
        SFID << ")\n";

    delete dsa_rsp;
}

/**
 * Incoming ACK from MS  (SS-initiated DSA)
 */
void CommonPartSublayerServiceFlows_BS::handle_DSA_ACK(Ieee80216_DSA_ACK *dsa_ack)
{
    ev << "Incoming DSA-ACK from CID: [" << dsa_ack->getCID() << "]\n";

    ServiceFlow new_sf = dsa_ack->getNewServiceFlow();
    new_sf.link_type = ldUPLINK;

    if (new_sf.state == SF_ACTIVE)
        new_sf.active_parameters = new sf_QoSParamSet(*(new_sf.admitted_parameters));

    // the SS accepts the connection, so we can add it to the maps
    map_connections[new_sf.CID] = new_sf.SFID;
    map_serviceFlows[new_sf.SFID] = new_sf;

    // also add the connection to the stations own connections map
    // this way, the station can later be identified by any CID
    structMobilestationInfo *str_ms = lookupMS(dsa_ack->getMac_address());
    if (str_ms != NULL)
    {
        str_ms->map_own_connections[new_sf.CID] = new_sf.CID;
    }
    else
        error("Station not found by MAC address");

    ev << "CIDS: " << str_ms->Basic_CID << " " << str_ms->Primary_Management_CID << " " <<
        str_ms->Secondary_Management_CID << "\n";

    ev << "SS-DSA: New connection established! (CID=" << new_sf.CID << " | SFID=" << new_sf.
        SFID << ")\n";

    delete dsa_ack;
}

/* Creates initial connections. (See table 345 in standard)
 * The CID/SFID and ServiceFlow are created.
 * There are 3 initial MAC connections:
 * a) Basic Connection (0x0001 - allowed_connections )
 * b) Primary Management Connection (allowed_connections +1 - 2x allowed_connections )
 * c) Secondary Management Connection (2x allowed_connections +1 - 0xFE9F, Transport-CID and Managed Stations CID
 */
void CommonPartSublayerServiceFlows_BS::createManagementConnection(structMobilestationInfo *registered_ss,
                                                                   management_type type)
{
    // TODO WICHTIG! überschneidungen von CIDs vermeiden!!!

    Enter_Method("createManagementConnection() for SS");

    ev << "Creating ManagementConnection for " << registered_ss->MobileMacAddress<< "  (" << type << ")\n";

    ServiceFlow *management_sf = new ServiceFlow();
    management_sf->SFID = getFreeSFID();

    // state is set to SF_MANAGEMENT to identify the ServiceFlow as a dummy for management connections
    management_sf->state = SF_MANAGEMENT;
    management_sf->traffic_type = MANAGEMENT;
    management_sf->link_type = ldMANAGEMENT;

    switch (type)
    {
    case BASIC:
        management_sf->CID = getNewManagementCID(BASIC);
        registered_ss->Basic_CID = management_sf->CID;
        break;

    case PRIM_MAN_CID:
        management_sf->CID = getNewManagementCID(PRIMARY);
        registered_ss->Primary_Management_CID = management_sf->CID;
        break;

    case SECONDARY:
        management_sf->CID = getNewManagementCID(SECONDARY);
        registered_ss->Secondary_Management_CID = management_sf->CID;
        break;
    }

    // store the CID/SFID mapping in the connection-map
    map_connections[management_sf->CID] = management_sf->SFID;

    // store the newly created ServiceFlow in the serviceFlow-map
    map_serviceFlows[management_sf->SFID] = *management_sf;

    // store the created CIDs in the stations own connection map
    registered_ss->map_own_connections[management_sf->CID] = management_sf->CID;
}

int CommonPartSublayerServiceFlows_BS::getFreeSFID()
{
    int sfid = -1;

    // TODO WICHTIG! überschneidungen von SFIDs vermeiden!!!

    /**
    * preparations for deleted ServiceFlows done:
    * if upper bound for SFIDs is reached, one may try to get the next free SFID
    * from a list of removed ServiceFlows during runtime
    */
    if (list_removed_SFIDs.size() > 0)
    {
        if (list_removed_SFIDs.begin() != list_removed_SFIDs.end())
        {
            sfid = list_removed_SFIDs.front();
            list_removed_SFIDs.pop_front();
        }

        /*
        list<int>::iterator list_it = list_removed_SFIDs.begin();
        if ( list_it != list_removed_SFIDs.end() )
        {
            sfid = (int)&list_it;
            list_removed_SFIDs.pop_front();
        }
        */
    }
    // otherwise, simply increment the SFIDs
    else
    {
        sfid = cur_max_sfid++;

/*
        map<int,ServiceFlow>::iterator idsf_it = map_serviceFlows.find(cur_max_sfid);

        if ( idsf_it != map_serviceFlows.end() )
        {
            sfid = ++cur_max_sfid;
        }
        else
        {
            sfid = cur_max_sfid;
        }
*/
    }

    return sfid;
}

/** (0x0001 - m)
 * Returns the next available CID in this BS.
 * There are 3 types of MAC connections:
 * a) Basic Connection (0x0001 - "allowed_connections" )
 * b) Primary Management Connection ("allowed_connections" +1 - 2x allowed_connections )
 * c) Secondary Management Connection (2x "allowed_connections" +1 - 0xFE9F, Transport-CID and Managed Stations CID
 */

//idee:  map mit key=CID erstellen -> ist einfach abzufragen und schnell
// lücken füllen bei überlauf, laufvariable der derzeit höchsten CID halten
int CommonPartSublayerServiceFlows_BS::getNewManagementCID(management_type mtype)
{
    int new_cid = -1;
    int range_start, range_end;
    int *reference_id;

    switch (mtype)
    {
    case BASIC:
        range_start = 1;
        range_end = allowed_connections;
        reference_id = &cur_max_basic_cid;
        break;

    case PRIMARY:
        range_start = allowed_connections + 1;
        range_end = 2 * allowed_connections;
        reference_id = &cur_max_primary_cid;
        break;

    case SECONDARY:
        range_start = 2 * allowed_connections + 1;
        range_end = (int) 0xFE9F;
        reference_id = &cur_max_secondary_cid;
        break;
    }

    if (*reference_id <= range_end)
    {

        new_cid = (*reference_id)++;

/*
        map<int,int>::iterator cid_it = map_connections.find(*reference_id);

        // *reference_id was not found in map
        if ( cid_it == map_connections.end() )
        {
            new_cid = *reference_id;
            // *reference_id += ++pending_requests;
        }
        // cur_max_id already exists
        else
        {
            new_cid = ++(*reference_id);
        }
*/
    }
    else
    {
        //TODO ab hier: QuickFind o.ä. implementieren, da alle CIDs ausgeschöpft sind und jetzt
        // vermutlich lücken im interval von 1 - m durch löschen von ServiceFlows enstanden sind..
        // diese suchen und neu vergeben!
        EV << "Connections exceed maximum connections limit of " << allowed_connections << "\n";
        error("Too many connections!");
    }

    EV << "   CID = " << new_cid << "\n";
    return new_cid;
}

/*
void CommonPartSublayerServiceFlows_BS::createConnection(int ms_cid, ip_traffic_types traffic_type)
{
}
*/

structMobilestationInfo *CommonPartSublayerServiceFlows_BS::lookupMS(MACAddress mac_addr)
{
    if (mssList)
    {
        MobileSubscriberStationList::iterator it;
        for (it = mssList->begin(); it != mssList->end(); it++)
        {
            if (it->MobileMacAddress == mac_addr)
                return &(*it);
        }
    }

    return NULL;
}

/**
 * PUBLIC GETTER/SETTER
 */
void CommonPartSublayerServiceFlows_BS::setMSSList(MobileSubscriberStationList *cp_mssList)
{
    Enter_Method("setMSSList()");
    mssList = cp_mssList;
}

void CommonPartSublayerServiceFlows_BS::setBSInfo(structBasestationInfo *bsinfo)
{
    Enter_Method("setBSInfo");
    localBSInfo = bsinfo;
}

/**
 * This is the main method for admission control.
 * When a ServiceFlow is requested its parameters are tested against, if there is
 * enough radio resource left in means of available datarate.
 * Corresponding to the QoS parameters, the max. sustained datarate
 * is chosen if there is enough datarate available.
 * Otherweise, the minimum reserved datarate or the maximum available datarate
 * in the interval [min reserved rate, max sustained rate[ is chosen.
 */
bool CommonPartSublayerServiceFlows_BS::checkQoSParams(sf_QoSParamSet *req_params,
                                                       link_direction link_type)
{

    int max_sustained_traffic_rate = req_params->max_sustained_traffic_rate;
    int min_reserved_traffic_rate = req_params->min_reserved_traffic_rate;
    //int max_latency = req_params->max_latency;
    //int tolerated_jitter = req_params->tolerated_jitter;
    //int priority = req_params->traffic_priority;
    req_tx_policy tx_policy = req_params->request_transmission_policy;

    if (link_type == ldMANAGEMENT)
        error("Wrong link direction given! Must be either ldUPLINK or ldDOWNLINK!");

    switch ((int) getTypeOfServiceFlow(req_params))
    {
    case UGS:
        if (availableDatarate[link_type] - max_sustained_traffic_rate >= 0)
        {
            req_params->granted_traffic_rate = max_sustained_traffic_rate;
            availableDatarate[link_type] -= req_params->granted_traffic_rate;
            return true;
        }
        else
            // try to kick a less prioritized flow
        if (kickFlowBelow(UGS, link_type, max_sustained_traffic_rate))
        {
            //breakpoint("kick");
            req_params->granted_traffic_rate = max_sustained_traffic_rate;
            return true;
        }

        else
            return false;
        break;

    case RTPS:                 // and currently ERTPS, too, until tx_policy is handled
        if (availableDatarate[link_type] - max_sustained_traffic_rate >= 0)
        {
            req_params->granted_traffic_rate = max_sustained_traffic_rate;
            availableDatarate[link_type] -= req_params->granted_traffic_rate;
            return true;
        }
        else if (availableDatarate[link_type] - min_reserved_traffic_rate >= 0)
        {
            // grant the highest possible traffic rate above the minimum
            req_params->granted_traffic_rate = availableDatarate[link_type];
            availableDatarate[link_type] = 0;
            return true;
        }
        else
            // try to kick a less prioritized flow
        if (kickFlowBelow(RTPS, link_type, min_reserved_traffic_rate))
        {
            //breakpoint("kick");
            req_params->granted_traffic_rate = min_reserved_traffic_rate;
            return true;
        }
        else
            return false;
        break;

    case NRTPS:
        // TODO aufs maximum anpassen
        if (availableDatarate[link_type] - min_reserved_traffic_rate >= 0)
        {
            // grant the highest possible traffic rate above the minimum (still TODO)
            req_params->granted_traffic_rate = min_reserved_traffic_rate;
            availableDatarate[link_type] -= req_params->granted_traffic_rate;
            return true;
        }
        else
            // try to kick a less prioritized flow
        if (kickFlowBelow(NRTPS, link_type, min_reserved_traffic_rate))
        {
            //breakpoint("kick");
            req_params->granted_traffic_rate = min_reserved_traffic_rate;
            return true;
        }
        else
            return false;
        break;

    case BE:
        if (availableDatarate[link_type] > lower_bound_for_BE_traffic)
        {
            // grant the highest possible traffic rate above the minimum (still TODO)
            // until further implementation: grant a fourth of the still available datarate
            req_params->granted_traffic_rate =
                (availableDatarate[link_type] - upper_bound_for_BE_grant > 0
                 ? upper_bound_for_BE_grant : availableDatarate[link_type]);
            //req_params->granted_traffic_rate = availableDatarate[link_type];
            availableDatarate[link_type] -= req_params->granted_traffic_rate;
            return true;
        }
        else
            return false;
        break;
    }

    updateDisplay();

    return false;
}

/**
 * Method to reduce the granted traffic rate of a ServiceFlow or completely remove it, if a higher
 * prioritized ServiceFlow is requested.
 */
bool CommonPartSublayerServiceFlows_BS::kickFlowBelow(ip_traffic_types needy_flow,
                                                      link_direction link_type,
                                                      int demanded_traffic_rate)
{
    sortServiceFlowsByPriority();

    int needed_datarate = demanded_traffic_rate - availableDatarate[link_type];
    EV << "needed datarate: " << needed_datarate << "\n";
    short prio = BE;
    while (prio > needy_flow && needed_datarate > 0)
    {
        list<ServiceFlow *>::iterator it;
        for (it = serviceFlowsSortedByPriority[prio].begin();
             it != serviceFlowsSortedByPriority[prio].end(); it++)
        {
            if ((*it)->state == SF_ACTIVE)
            {
                // if the current flow has more granted datarate than the needed rate of the higher priority flow,
                // try to change the ServiceFlow to a lower granted bitrate => DSC-REQ
                if (needed_datarate > 0
                    && (*it)->active_parameters->granted_traffic_rate - needed_datarate > 0)
                {
                    (*it)->active_parameters->granted_traffic_rate -= needed_datarate;
                    needed_datarate = 0;

                    EV << "reduced flow: " << (*it)->CID << "  prio: " << prio << "\n";
                }
                // if the current flow has less datarate than needed, kick it completely
                // ==> DSD-REQ
                else if (needed_datarate > 0
                         && (*it)->active_parameters->granted_traffic_rate - needed_datarate < 0)
                {
                    needed_datarate -= (*it)->active_parameters->granted_traffic_rate;
                    (*it)->active_parameters->granted_traffic_rate = 0;

                    (*it)->state = SF_PROVISIONED;
                    EV << "kicked flow: " << (*it)->CID << "  prio: " << prio << "  and set back to provisioned\n";
                }
            }
        }

        prio--;
    }

    if (needed_datarate == 0)
    {
        availableDatarate[link_type] = 0;
        return true;
    }
    else
        return false;
}

//void CommonPartSublayerAuthorizationModule::setServiceFlowMap( ServiceFlowMap *sf_map ) {
// Enter_Method_Silent();
//
// map_serviceFlows = sf_map;
//}

void CommonPartSublayerServiceFlows_BS::setAvailableUplinkDatarate(int datarate)
{
    Enter_Method_Silent();

    availableDatarate[ldUPLINK] = datarate;
}

void CommonPartSublayerServiceFlows_BS::setAvailableDownlinkDatarate(int datarate)
{
    Enter_Method_Silent();

    availableDatarate[ldDOWNLINK] = datarate;
}

/**
 * Here we try to identify the type of traffic by given QoS parameters.
 * The used conditions below are taken from the 802.16 standard.
 */
ip_traffic_types CommonPartSublayerServiceFlows_BS::getTypeOfServiceFlow(sf_QoSParamSet *req_params)
{
    int max_sustained_traffic_rate = req_params->max_sustained_traffic_rate;
    int min_reserved_traffic_rate = req_params->min_reserved_traffic_rate;
    int max_latency = req_params->max_latency;
    int tolerated_jitter = req_params->tolerated_jitter;
    int priority = req_params->traffic_priority;
    req_tx_policy tx_policy = req_params->request_transmission_policy;

    if (max_sustained_traffic_rate > 0 &&
        min_reserved_traffic_rate == max_sustained_traffic_rate && tolerated_jitter > 0)
        return UGS;
    else if (max_sustained_traffic_rate > 0 &&
             min_reserved_traffic_rate<max_sustained_traffic_rate && max_latency>0)
        return RTPS;

    // commented out by reason: handling of tx_policy not yet implemented
// else if ( max_sustained_traffic_rate > 0 &&
//    min_reserved_traffic_rate < max_sustained_traffic_rate &&
//    max_latency > 0 &&
//    tx_policy-> )
//  return ERTPS;

    else if (min_reserved_traffic_rate > 0 && max_sustained_traffic_rate == 0 && priority > 0)
        return NRTPS;
    else if (min_reserved_traffic_rate == 0 && max_sustained_traffic_rate == 0 && max_latency == 0)
        return BE;
    throw cRuntimeError("unknown ip_traffic_types in getTypeOfServiceFlow");
}

void CommonPartSublayerServiceFlows_BS::sortServiceFlowsByPriority()
{
    // initialize the lists (changes in ServiceFlow configuration may have happened in the meanwhile)
    serviceFlowsSortedByPriority[MANAGEMENT].clear();
    serviceFlowsSortedByPriority[UGS].clear();
    serviceFlowsSortedByPriority[RTPS].clear();
    serviceFlowsSortedByPriority[ERTPS].clear();
    serviceFlowsSortedByPriority[NRTPS].clear();
    serviceFlowsSortedByPriority[BE].clear();

    // sort
    if (map_serviceFlows.size() > 0)
    {
        ServiceFlowMap::iterator it;
        for (it = map_serviceFlows.begin(); it != map_serviceFlows.end(); it++)
        {
            ServiceFlow *sf = &(it->second);
            serviceFlowsSortedByPriority[sf->traffic_type].push_back(sf);
        }
    }
}

void CommonPartSublayerServiceFlows_BS::updateDisplay()
{
    char buf[90];
    sprintf(buf, "Available Downlink/s: %d \nAvailable Uplink/s: %d",
            (int) availableDatarate[ldDOWNLINK], (int) availableDatarate[ldUPLINK]);

    getDisplayString().setTagArg("t", 0, buf);
    getDisplayString().setTagArg("t", 1, "t");
}
