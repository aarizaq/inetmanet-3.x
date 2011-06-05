/**
 * Create and manage ServiceFlows
 */

#include "CommonPartSublayerServiceFlows_MS.h"

Define_Module(CommonPartSublayerServiceFlows_MS);

CommonPartSublayerServiceFlows_MS::CommonPartSublayerServiceFlows_MS()
{
}

CommonPartSublayerServiceFlows_MS::~CommonPartSublayerServiceFlows_MS()
{
}

void CommonPartSublayerServiceFlows_MS::initialize()
{
    controlPlaneIn = findGate("controlPlaneIn");
    controlPlaneOut = findGate("controlPlaneOut");
}

void CommonPartSublayerServiceFlows_MS::handleMessage(cMessage *msg)
{
    Ieee80216ManagementFrame* manFrame = dynamic_cast<Ieee80216ManagementFrame *>(msg); // Empfangendes Paket ist eine IEEE802.16e Frame
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

    case ST_DSX_RVD:
        delete msg;
        break;

    default:
        EV << "\n\nunidentified frame\n\n";
    }
}

void CommonPartSublayerServiceFlows_MS::createAndSendNewDSA_REQ(int prim_management_cid,
                                                                ServiceFlow* requested_sf,
                                                                ip_traffic_types type)
{
    Enter_Method("createAndSendNewDSA_REQ()");

    Ieee80216_DSA_REQ *dsa_req = new Ieee80216_DSA_REQ("DSA_REQ");
    dsa_req->setCID(prim_management_cid);
    requested_sf->traffic_type = type;
    dsa_req->setNewServiceFlow(*requested_sf);
    dsa_req->setTraffic_type(type);

    send(dsa_req, controlPlaneOut);
}

/**
 * Incoming Request ( BS-initiated DSA )
 */
void CommonPartSublayerServiceFlows_MS::handle_DSA_REQ(Ieee80216_DSA_REQ* dsa_req)
{
    // TODO jetzt ServiceFlow auf Gültigkeit prüfen, evt. ABÄNDERN ==>  DSA-RSP bauen und schicken
    if (true)
    {
        Ieee80216_DSA_RSP *dsa_rsp;
        EV << "DSA-REQ for traffic type: " << dsa_req->getTraffic_type() << "\n";
        dsa_rsp =
            build_DSA_RSP(dsa_req->getCID(), &dsa_req->getNewServiceFlow(),
                          (ip_traffic_types) dsa_req->getTraffic_type());

        /** The BS-initiated DSA-REQ already contains IDs + the MS is not allowed to propose CIDs...
        dsa_rsp->getNewServiceFlow().CID = getNewManagementCID( SECONDARY );
        dsa_rsp->getNewServiceFlow().SFID = getFreeSFID();
        dsa_rsp->getNewServiceFlow().traffic_type = (ip_traffic_types)dsa_req->getTraffic_type();
        */
        send(dsa_rsp, controlPlaneOut);
    }

    delete dsa_req;
}

/**
 * Incoming Response ( SS-initiated DSA )
 */
void CommonPartSublayerServiceFlows_MS::handle_DSA_RSP(Ieee80216_DSA_RSP* dsa_rsp)
{
    // TODO ServiceFlow akzeptieren ODER angepassten ServiceFlow in neuem REQ zurückschicken
    // die BS könnte ihn ja gemessen an der Auslastung angepasst haben...

    ServiceFlow new_sf = dsa_rsp->getNewServiceFlow();

    // maybe the ServiceFlow was rejected:
    if (dsa_rsp->getRejected())
    {
        EV << "ServiceFlow request for traffic type [" << dsa_rsp->getTraffic_type() <<
            "] has been rejected by BS\n";
        delete dsa_rsp;

        return;
    }

    // store the connection information assigned by the BS in the local maps, e.g for traffic classification
    map_connections[new_sf.CID] = new_sf.SFID;
    map_serviceFlows[new_sf.SFID] = new_sf;

    Ieee80216_DSA_ACK* dsa_ack = new Ieee80216_DSA_ACK();
    dsa_ack->setCID(dsa_rsp->getCID());
    dsa_ack->setNewServiceFlow(dsa_rsp->getNewServiceFlow());

    send(dsa_ack, controlPlaneOut);

    EV << "New connection established by MobileStation! (CID=" << new_sf.CID << " | SFID=" <<
        new_sf.SFID << " ,TrafficType: " << dsa_rsp->getTraffic_type() << ")\n";

    delete dsa_rsp;
}

/*
 * Incoming ACK ( BS-initiated DSA )
 */
void CommonPartSublayerServiceFlows_MS::handle_DSA_ACK(Ieee80216_DSA_ACK* dsa_ack)
{
    ServiceFlow new_sf = dsa_ack->getNewServiceFlow();

    // store the connection information in the local maps
    map_connections[new_sf.CID] = new_sf.SFID;
    map_serviceFlows[new_sf.SFID] = new_sf;

    EV << "New connection established by BaseStation! (CID=" << new_sf.CID << " | SFID=" <<
        new_sf.SFID << " ,TrafficType: " << new_sf.traffic_type << ")\n";

    delete dsa_ack;
}
