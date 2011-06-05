//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//

#include "ControlPlaneBase.h"
//Define_Module(ControlPlaneBase);

void ControlPlaneBase::initialize(int stage)
{
    if (stage == 0)
    {
        receiverCommonGateIn = findGate("receiverCommonGateIn");
        receiverCommonGateOut = findGate("receiverCommonGateOut");
        transceiverCommonGateIn = findGate("transceiverCommonGateIn");
        transceiverCommonGateOut = findGate("transceiverCommonGateOut");

/**
        transceiverQoSGateIn = findGate("transceiverQoSGateIn");
        transceiverQoSGateOut = findGate("transceiverQoSGateOut");
        receiverQoSGateIn = findGate("receiverQoSGateIn");
        receiverQoSGateOut = findGate("receiverQoSGateOut");
*/
        trafficClassificationGateIn = findGate("trafficClassificationGateIn");

        serviceFlowsGateIn = findGate("serviceFlowsGateIn");
        serviceFlowsGateOut = findGate("serviceFlowsGateOut");
    }
}

/**
* @brief Hauptfunktionen
***************************************************************************************/

void ControlPlaneBase::handleMessage(cMessage * msg)
{
/**
    if (msg->arrivalGateId()==transceiverCommonGateIn)
        handleHigherLayerMsg(msg);
    else if (msg->arrivalGateId()==receiverCommonGateIn)
        handleLowerLayerMsg(msg);
    else if (msg->isSelfMessage())
        handleSelfMsg(msg);
    else if (msg->arrivalGateId()==trafficClassificationGateIn)
        handleHigherLayerMsg(msg);
*/
    if (msg->getArrivalGateId() == transceiverCommonGateIn)
    {
        handleHigherLayerMsg(msg);
    }
    else if (msg->getArrivalGateId() == receiverCommonGateIn)
    {
        handleLowerLayerMsg(msg);
    }
    else if (msg->isSelfMessage())
    {
        handleSelfMsg(msg);
    }
    else if (msg->getArrivalGateId() == trafficClassificationGateIn)
    {
        handleClassificationCommand(check_and_cast<Ieee80216ClassificationCommand *>(msg));
    }
    else if (msg->getArrivalGateId() == serviceFlowsGateIn)
    {
        handleServiceFlowMessage(msg);
    }
    else
    {
    	error("unhandled message: %s (type: %s)\n", msg->getName(), msg->getClassName());
    	delete msg;
    }
}

  /**
  * @brief Hilfsfunktionen
  ***************************************************************************************/

bool ControlPlaneBase::isHigherLayerMsg(cMessage * msg)
{
    return msg->getArrivalGateId() == transceiverCommonGateIn;
}

bool ControlPlaneBase::isLowerLayerMsg(cMessage * msg)
{
    return msg->getArrivalGateId() == receiverCommonGateIn;
}

void ControlPlaneBase::sendtoLowerLayer(cMessage * msg)
{
    Ieee80216MacHeader *macFrame = dynamic_cast<Ieee80216MacHeader *>(msg); // Empfangendes Paket ist eine IEEE802.16e Frame
    if (macFrame)
    {
        macFrame->setByteLength(48);
        send(macFrame, transceiverCommonGateOut);
    }
    else
    {
        send(msg, transceiverCommonGateOut);
    }
}

void ControlPlaneBase::sendtoHigherLayer(cMessage * msg)
{
    send(msg, receiverCommonGateOut);
}

void ControlPlaneBase::sendRequest(Ieee80216PrimRequest * req)
{
    cMessage *msg = new cMessage(req->getClassName());
    msg->setControlInfo(req);
    sendtoLowerLayer(msg);
}

void ControlPlaneBase::storeBSInfo(double rcvdPower)
{
    ev << "Sendeleistung: " << rcvdPower << "\n";
}

void ControlPlaneBase::setSNR(double rcvdPower)
{
    ev << "Roland Control Plane MS Sendeleistung: " << rcvdPower << "\n";
    //recSnrVec.record(rcvdPower);
}

void ControlPlaneBase::setTime(double rcvdPower)
{
    ev << "Roland Control Plane MS Sendeleistung: " << rcvdPower << "\n";
}

void ControlPlaneBase::setDistance(double distance)
{
    ev << "Distanz zwischen MS und BS: " << distance << "\n";
    //recDistVec.record(distance);
}

void ControlPlaneBase::setBSID(double rcvdPower)
{
    ev << "Roland Control Plane MS Sendeleistung: " << rcvdPower << "\n";
}

double ControlPlaneBase::searchMinSnr()
{
    double snirMin = receiverRadio->getSNRlist().begin()->snr;
    for (SnrList::const_iterator iter = receiverRadio->getSNRlist().begin();
         iter != receiverRadio->getSNRlist().end(); iter++)
        if (iter->snr < snirMin)
            snirMin = iter->snr;

    return snirMin;
}

double ControlPlaneBase::pduDuration(Ieee80216MacHeader * msg)
{
    return pduDuration(msg->getByteLength(), bitrate);
}

double ControlPlaneBase::pduDuration(int bits, double bitrate)
{
    return bits / bitrate;
}

ServiceFlow *ControlPlaneBase::getServiceFlowForCID(int cid)
{

    ConnectionMap::iterator cid_it = map_connections->find(cid);
    if (cid_it != map_connections->end())
    {
        ServiceFlowMap::iterator sfid_it = map_serviceFlows->find((*cid_it).second);

        if (sfid_it != map_serviceFlows->end())
        {
            return &((*sfid_it).second);
        }
        else
            EV << "getServiceFlowForCID(" << cid << "): SFID not found!\n";
    }
    else
        EV << "getServiceFlowForCID(" << cid << "): CID not found!\n";
    return NULL;
}

int ControlPlaneBase::getSFIDForCID(int cid)
{

    ConnectionMap::iterator cid_it = map_connections->find(cid);
    if (cid_it != map_connections->end())
    {
        return cid_it->second;
    }
    else
    {
        EV << "getSFIDForCID(" << cid << "): CID not found!\n";
        return NULL;
    }
}

void ControlPlaneBase::listActiveServiceFlows()
{
    EV << "\n  (in  ControlPlaneBase::listActiveServiceFlows)\n";
    if (map_serviceFlows->size() > 0)
    {
        ServiceFlowMap::iterator sf_it;
        for (sf_it = map_serviceFlows->begin(); sf_it != map_serviceFlows->end(); sf_it++)
        {
            ServiceFlow *cur_sf = &(sf_it->second);
            if (cur_sf->state == SF_ACTIVE)
            {
                EV << " CID: " << cur_sf->CID << "    link type:" << cur_sf->link_type << "\n";
            }
        }
    }
    EV << "\n\n";
}

/**
 * Searches for a matching ServiceFlow for an incoming packet in the CS
 * and returns its associated CID
 * or -1 if no ServiceFlow was found for the type of incoming traffic.
 */
std::list<int> ControlPlaneBase::findMatchingServiceFlow(ip_traffic_types traffic_type,
                                                           link_direction link_type)
{
    Enter_Method("findMatchingServiceFlow");

    std::list<int> list_matching_cids;
    ConnectionMap::iterator cm_it;

    for (cm_it = map_connections->begin(); cm_it != map_connections->end(); cm_it++)
    {
        // if the CID does not belong to a management connection...
        if ((*cm_it).second != -1)
        {
            ServiceFlowMap::iterator sf_it = map_serviceFlows->find((*cm_it).second);
            if (sf_it != map_serviceFlows->end())
            {
                ServiceFlow sf = (*sf_it).second;

                EV << (*cm_it).first << " => " << (*cm_it).second << ", SFTrafficType=" <<
                    sf.traffic_type << ", link type=" << sf.link_type << "\n";

                if (sf.traffic_type == traffic_type && sf.link_type == link_type)
                {
                    EV << "Matching ServiceFlow found!\n";
                    list_matching_cids.push_back(sf.CID);
                }
            }
        }
    }

    return list_matching_cids;
}
