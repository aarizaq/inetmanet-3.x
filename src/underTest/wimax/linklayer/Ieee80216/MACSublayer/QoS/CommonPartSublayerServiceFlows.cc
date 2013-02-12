#include "CommonPartSublayerServiceFlows.h"
//Define_Module(CommonPartSublayerServiceFlows);

CommonPartSublayerServiceFlows::CommonPartSublayerServiceFlows()
{
}

CommonPartSublayerServiceFlows::~CommonPartSublayerServiceFlows()
{
}

void CommonPartSublayerServiceFlows::initialize()
{
}

bool serviceFlowAvailable(short *type)
{
/**
    switch (type)
    {
    case FTP:
        break;

    case VOICE:
        break;

    case VIDEO:
        break;
    }
*/
    if (true)
    {
        return true;
    }
    return false;
}

/**
 * Builds the DSA-XXX messages.s
 *
 * "type"
 *              Defines which type of IP-Traffic shall be trasporten along this connection
 *
 * "sf"
 *              Is the new ServiceFlow that shall be added with parameters for the given "type"
 *
 * "prim_management_cid"
 *              This is the unique CID between BS and MS over which the Request is sent
 */
Ieee80216_DSA_REQ *CommonPartSublayerServiceFlows::build_DSA_REQ(int prim_management_cid,
                                                                 ServiceFlow *sf,
                                                                 ip_traffic_types type)
{
    Ieee80216_DSA_REQ *dsa_req = new Ieee80216_DSA_REQ("DSA_REQ");
    EV << "Building DSA-REQ for " << prim_management_cid << ", Type: " << type << "\n";

    dsa_req->setNewServiceFlow(*sf);
    dsa_req->setTraffic_type(type);

    dsa_req->setCID(prim_management_cid);

    return dsa_req;
}

Ieee80216_DSA_RSP *CommonPartSublayerServiceFlows::build_DSA_RSP(int prim_management_cid,
                                                                 ServiceFlow *sf,
                                                                 ip_traffic_types type)
{
    Ieee80216_DSA_RSP *dsa_rsp = new Ieee80216_DSA_RSP("DSA_RSP");
    EV << "Building DSA-RSP for " << prim_management_cid << ", IP-Traffic-Type: " << type << "\n";

    dsa_rsp->setTraffic_type(type);

    sf->admitted_parameters = new sf_QoSParamSet(*(sf->provisioned_parameters));
    // strange BUG:
    // all parameters are transfered via the copy constructor except the packtInterval parameter..
    sf->admitted_parameters->packetInterval = sf->provisioned_parameters->packetInterval;

    dsa_rsp->setNewServiceFlow(*sf);

    dsa_rsp->setCID(prim_management_cid);

    return dsa_rsp;
}

Ieee80216_DSA_ACK *CommonPartSublayerServiceFlows::build_DSA_ACK(int prim_management_cid,
                                                                 ServiceFlow *sf,
                                                                 ip_traffic_types type)
{
    Ieee80216_DSA_ACK *dsa_ack = new Ieee80216_DSA_ACK("DSA_ACK");
    EV << "Building DSA-ACK for " << prim_management_cid << ", IP-Traffic-Type: " << type << "\n";

//  dsa_ack->setTraffic_type( type );
    dsa_ack->setNewServiceFlow(*sf);

    dsa_ack->setCID(prim_management_cid);

    return dsa_ack;
}

/**
 * PUBLIC GETTER/SETTER
 */
ConnectionMap *CommonPartSublayerServiceFlows::getConnectionMap()
{
    Enter_Method("getConnectionMap()");
    return &map_connections;
}

ServiceFlowMap *CommonPartSublayerServiceFlows::getServiceFlowMap()
{
    Enter_Method("getServiceFlowMap()");
    return &map_serviceFlows;
}
