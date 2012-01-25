//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file Nps.cc
 * @author Christian Fickinger, Bernhard Heep
 */

#include <GlobalNodeList.h>
#include <PeerInfo.h>
#include <RpcMacros.h>
#include <NeighborCache.h>
#include <GlobalNodeListAccess.h>
#include <Landmark.h>
#include <UnderlayConfigurator.h>
#include <SimpleNodeEntry.h>
#include <SimpleInfo.h>
#include <CoordBasedRoutingAccess.h>
#include <GlobalStatistics.h>

#include <Nps_m.h>

#include <Nps.h>


void Nps::init(NeighborCache* neighborCache)
{
    this->neighborCache = neighborCache;
    overlay = neighborCache->overlay;

    maxLayer = neighborCache->par("npsMaxLayer");
    dimensions = neighborCache->par("gnpDimensions");
    landmarkTimeout = neighborCache->par("gnpLandmarkTimeout");

    GnpNpsCoordsInfo::setDimension(dimensions);
    ownCoords = new GnpNpsCoordsInfo();

    receivedCalls = 0;
    pendingRequests = 0;
    coordCalcRuns = neighborCache->par("npsCoordCalcRuns");

    WATCH(*ownCoords);
    WATCH_VECTOR(landmarkSet);

    coordBasedRouting = CoordBasedRoutingAccess().get();
    globalNodeList = GlobalNodeListAccess().get();

    if (neighborCache->getParentModule()->getModuleByRelativePath("tier1")
        ->getModuleByRelativePath("landmark") == NULL) {
        landmarkTimer = new cMessage("landmarkTimer");
        neighborCache->scheduleAt(simTime() + landmarkTimeout, landmarkTimer);
    } else {
        // GNP-landmark or NPS-Layer-0-landmark
        ownCoords->setLayer(0); //TODO double, see Landmark.cc
    }
}

void Nps::handleTimerEvent(cMessage* msg)
{
    // process landmark timer message
    if (msg == landmarkTimer) {
        if (enoughLandmarks()) {
            delete msg;
            //std::cout << "[" << getThisNode().getIp() << "] (Re-)Trying to contact landmarks" << std::endl;
            sendCoordRequests();
        } else {
            neighborCache->scheduleAt(simTime() + landmarkTimeout, msg);
        }
    }
}

void Nps::handleRpcResponse(BaseResponseMessage* msg,
                            cPolymorphic* context,
                            int rpcId, simtime_t rtt)
{
    // call rpc stubs
    RPC_SWITCH_START( msg );
        RPC_ON_RESPONSE( CoordsReq ) {
            coordsReqRpcResponse(_CoordsReqResponse, context, rpcId, rtt);
        }
#ifdef EXTJOIN_DISCOVERY
        RPC_ON_RESPONSE( RttToNode ) {
            rttToNodeRpcResponse(_RttToNodeResponse, context, rpcId, rtt);
        }
#endif
    RPC_SWITCH_END( );

    return;
}

void Nps::handleRpcTimeout(BaseCallMessage* msg,
                           const TransportAddress& dest,
                           cPolymorphic* context, int rpcId,
                           const OverlayKey& destKey)
{
    RPC_SWITCH_START( msg ) {

        RPC_ON_CALL( CoordsReq ) {
            if (true/*doingNodeMeasurement()*/) {//TODO
#ifdef EXTJOIN_DISCOVERY
                if (getPendingRttsReq(dest) == -1) {
                    RttToNodeCall* call = getNodeMessage(dest);

                    RttToNodeResponse* rttRes = new RttToNodeResponse("RttToNodeXRes");
                    rttRes->setPingedNode(dest);
                    rttRes->setRttToNode(0);
                    std::vector<double> tempOwnCoords;
                    tempOwnCoords = getOwnCoordinates();
                    rttRes->setOwnCoordinatesArraySize(tempOwnCoords.size());
                    for (uint i = 0; i < tempOwnCoords.size(); i++) {
                        rttRes->setOwnCoordinates(i, tempOwnCoords[i]);
                    }
                    sendRpcResponse(call, rttRes);
                    deleteNodeMeasurement(dest);
                } else {
#endif
                    updateNodeMeasurement(dest, -1);
#ifdef EXTJOIN_DISCOVERY
                }
#endif

            } else {
                pendingRequests--;
            }
        }
#ifdef EXTJOIN_DISCOVERY
        RPC_ON_CALL( RttToNode ) {
            updateNodeMeasurement(dest, -1);
        }
#endif
    }
    RPC_SWITCH_END( )
}

bool Nps::handleRpcCall(BaseCallMessage* msg)
{
    RPC_SWITCH_START( msg );
        RPC_DELEGATE( CoordsReq, coordsReqRpc );
        //if (discovery)
    RPC_SWITCH_END( );

    return RPC_HANDLED;
}

void Nps::coordsReqRpc(CoordsReqCall* msg)
{
    receivedCalls++;
    CoordsReqResponse* coordResp = new CoordsReqResponse("CoordsReqResp");
    coordResp->setLayer(getOwnLayer()); //TODO
    coordResp->setNcsInfoArraySize(dimensions + 1);

    //if (getOwnLayer() != 0) {
    // ordinary node

    const std::vector<double>& ownCoordinates = getOwnCoordinates();

    uint8_t i;
    for (i = 0; i < ownCoordinates.size(); ++i) {
        coordResp->setNcsInfo(i, ownCoordinates[i]);
    }
    coordResp->setNcsInfo(i, getOwnLayer());

    /*} else {
        // landmark node
        Landmark* landmark = check_and_cast<Landmark*>(neighborCache->getParentModule()
            ->getModuleByRelativePath("tier1.landmark"));
        assert(landmark);
        const std::vector<double>& ownCoordinates = landmark->getOwnNpsCoords();

        uint8_t i;
        for (i = 0; i < ownCoordinates.size(); ++i) {
            coordResp->setNcsInfo(i, ownCoordinates[i]);
        }
        coordResp->setNcsInfo(i, getOwnLayer());
    }*/

    coordResp->setBitLength(COORDSREQRESPONSE_L(coordResp));
    neighborCache->sendRpcResponse(msg, coordResp);
}

#ifdef EXTJOIN_DISCOVERY
void Nps::rttToNodeRpc(RttToNodeCall* msg)
{
    incReceivedCalls();
    TransportAddress dest = msg->getNodeToPing();
//std::cout << "Set Node Message";
    setNodeMessage(dest, msg);
    sendCoordsReqCall(dest, -1);
}
#endif

void Nps::coordsReqRpcResponse(CoordsReqResponse* response,
                               cPolymorphic* context, int rpcId, simtime_t rtt)
{
    pendingRequests--;
    NodeHandle& srcNode = response->getSrcNode();

    std::vector<double> tempCoords;
    for (uint8_t i = 0; i < response->getNcsInfoArraySize(); i++) {
        tempCoords.push_back(response->getNcsInfo(i));
    }
    GnpNpsCoordsInfo* coordsInfo =
        static_cast<GnpNpsCoordsInfo*>(createNcsInfo(tempCoords));

    EV << "[Nps::coordsReqRpcResponse() @ " << neighborCache->thisNode.getIp()
       << " (" << neighborCache->thisNode.getKey().toString(16) << ")]\n    received landmark coords: "
       << tempCoords[0];

    for (uint8_t i = 1; i < dimensions; i++) {
        EV << ", " << tempCoords[i];
    }
    EV  << " (rtt = " << rtt << ")" << endl;

#ifdef EXTJOIN_DISCOVERY
    if (doingDiscovery()) {
        //if in Discovery insert RTT only if lower then already set RTT
        if ((isEntry(srcNode) && rtt < getNodeRtt(srcNode))
            || (isEntry(srcNode) && getNodeRtt(srcNode) < 0) ) {
            updateNode(srcNode, rtt, tempCoords, 0);
        } else if (!(isEntry(srcNode))) {
            updateNode(srcNode, rtt, tempCoords, 0);
        } else {
            updateNode(srcNode, getNodeRtt(srcNode), tempCoords, 0);
        }
        setNodeLayer(srcNode, tempLayer);
    }
    else if (doingNodeMeasurement()) {
        if (getPendingRttsReq(srcNode) == -1) {
            updateNode(srcNode, rtt, tempCoords, 0);
            setNodeLayer(srcNode, tempLayer);
            RttToNodeCall* prevCall = getNodeMessage(srcNode);

            RttToNodeResponse* rttRes = new RttToNodeResponse("RttToNodeXRes");
            rttRes->setPingedNode(srcNode);
            rttRes->setRttToNode(rtt);
            std::vector<double> tempOwnCoords;
            tempOwnCoords = getOwnCoordinates();
            rttRes->setOwnCoordinatesArraySize(tempOwnCoords.size());
            for (uint i = 0; i < tempOwnCoords.size(); i++) {
                rttRes->setOwnCoordinates(i, tempOwnCoords[i]);
            }

            sendRpcResponse(prevCall, rttRes);
            deleteNodeMeasurement(srcNode);
        } else {
            updateNode(srcNode, rtt, tempCoords, 0);
            setNodeLayer(srcNode, tempLayer);
            if (checkCoordinates(getOwnCoordinates(), tempCoords, rtt)) {
                updateNodeMeasurement(srcNode, -1, 0, 1);
            } else {
                updateNodeMeasurement(srcNode, -1, 0, 0);
            }
        }
    }
    else
#endif
    if (pendingRequests == 0) {
        // got all responses, now compute own coordinates and join overlay
        neighborCache->updateNode(srcNode, rtt, NodeHandle::UNSPECIFIED_NODE,
                                  coordsInfo);

        std::vector<LandmarkDataEntry> probedLandmarks;
        if (getLandmarkSetSize() < dimensions + 1) {
            setLandmarkSet(dimensions + 1, maxLayer, &landmarkSet);
        }

        for (std::vector<TransportAddress>::iterator it = landmarkSet.begin();
             it != landmarkSet.end(); ++it) {
            const GnpNpsCoordsInfo* tempInfo =
                dynamic_cast<const GnpNpsCoordsInfo*>(neighborCache->getNodeCoordsInfo(*it));

            assert(tempInfo);

            LandmarkDataEntry temp;
            temp.coordinates.resize(tempInfo->getDimension());
            temp.rtt = neighborCache->getNodeRtt(*it).first;
            temp.layer = tempInfo->getLayer();
            for (uint8_t i = 0; i < tempInfo->getDimension(); ++i) {
                temp.coordinates[i] = tempInfo->getCoords(i);
            }
            temp.ip = NULL;

            probedLandmarks.push_back(temp);
        }
        assert(probedLandmarks.size() > 0);

        computeOwnCoordinates(probedLandmarks);
        computeOwnLayer(probedLandmarks);

        std::vector<double> coords = getOwnCoordinates();
        EV << "[Nps::coordsReqRpcResponse() @ " << neighborCache->thisNode.getIp()
           << " (" << neighborCache->thisNode.getKey().toString(16) << ")]\n    setting own coords: "
           << coords[0];
        for (uint8_t i = 1; i < dimensions; i++) {
            EV << ", " << coords[i];
        }
        EV << endl;

        //test
        /*
        ChurnGenerator* lmChurnGen = NULL;
        for (uint8_t i = 0; i < neighborCache->underlayConfigurator->getChurnGeneratorNum(); i++) {
            ChurnGenerator* searchedGen;
            searchedGen = neighborCache->underlayConfigurator->getChurnGenerator(i);
            if (searchedGen->getNodeType().overlayType != "oversim.common.cbr.LandmarkModules") {
                lmChurnGen = searchedGen;
            }
        }
        */

        SimpleNodeEntry* entry =
                dynamic_cast<SimpleInfo*>(globalNodeList->
                                          getPeerInfo(neighborCache->thisNode.getIp()))->getEntry();

        double error = 0;
        for (uint8_t i = 1; i < entry->getDim(); i++) {
            error += pow(coords[i] - entry->getCoords(i), 2);
        }
        error = sqrt(error);

        neighborCache->globalStatistics
          ->addStdDev("NPS: Coordinate difference", error);

        neighborCache->neighborCache.clear(); //TODO
        neighborCache->neighborCacheExpireMap.clear(); //TODO

        neighborCache->getParentModule()
            ->bubble("GNP/NPS coordinates calculated -> JOIN overlay!");

        if (coordBasedRouting) {
            int bitsPerDigit = neighborCache->overlay->getBitsPerDigit();
            neighborCache->thisNode.setKey(
                coordBasedRouting->getNodeId(coords, bitsPerDigit,
                                             OverlayKey::getLength()));

            EV << "[Nps::coordsReqRpcResponse() @ "
               << neighborCache->thisNode.getIp()
               << " (" << neighborCache->thisNode.getKey().toString(16) << ")]"
               << "\n    -> nodeID ( 2): "
               << neighborCache->thisNode.getKey().toString(2)
               << "\n    -> nodeID (16): "
               << neighborCache->thisNode.getKey().toString(16) << endl;

            // returning to BaseOverlay
            neighborCache->overlay->join(neighborCache->thisNode.getKey());
        } else {
            // returning to BaseOverlay
            neighborCache->overlay->join();
        }
    } else {
        neighborCache->updateNode(srcNode, rtt, NodeHandle::UNSPECIFIED_NODE,
                                  coordsInfo);
    }
}

#ifdef EXTJOIN_DISCOVERY
void Nps::rttToNodeRpcResponse(RttToNodeResponse* response,
                                               cPolymorphic* context, int rpcId, simtime_t rtt)
{
    uint dim = coordBasedRouting->getXmlDimensions();
    TransportAddress nodeToCheck = response->getPingedNode();
    std::vector<double> tempCoords;
    tempCoords.resize(dim);
    for (uint i = 0; i < dim; i++) {
        tempCoords[i] = response->getOwnCoordinates(i);
    }
    if (checkCoordinates(tempCoords, getNodeCoords(nodeToCheck), response->getRttToNode())) {
        updateNodeMeasurement(nodeToCheck, -1, 0, 1);
    } else {
        updateNodeMeasurement(nodeToCheck, -1, 0, 0);
    }
    delete context;
}
#endif


bool Nps::setLandmarkSet(uint8_t howManyLM, uint8_t maxLayer,
                         std::vector<TransportAddress>* landmarkSet)
{
    landmarkSet->clear();
    NeighborCache::NeighborCacheIterator it;
    uint availableLM = 0;
    TransportAddress landmark;
    for(it = neighborCache->neighborCache.begin(); it != neighborCache->neighborCache.end(); it++ ) {
        if (dynamic_cast<GnpNpsCoordsInfo*>(it->second.coordsInfo) &&
            static_cast<GnpNpsCoordsInfo*>(it->second.coordsInfo)->getLayer()
                < maxLayer) {
            landmark.setIp(it->first.getIp());
            landmark.setPort(it->second.nodeRef.getPort());
            landmarkSet->push_back(landmark);
            availableLM++;
        }
    }
    if (availableLM < howManyLM) {
        return false;
    } else {
        uint i = availableLM;
        while (i > howManyLM) {
            uint randomNumber = (intuniform(0, landmarkSet->size()));
            landmarkSet->erase(landmarkSet->begin() + randomNumber);
            i--;
        }
        return true;
    }
}

void Nps::sendCoordRequests()
{
    std::vector <TransportAddress> landmarks;
    landmarks = getLandmarks(dimensions + 1);

    simtime_t timeout = -1;

    if (landmarks.size() > 0) {
        for (size_t i = 0; i < landmarks.size(); i++) {
            const TransportAddress& tolm = landmarks[i];
            sendCoordsReqCall(tolm, timeout);
        }
        setLandmarkSet(dimensions + 1, maxLayer, &landmarkSet);
    }
}

void Nps::sendCoordsReqCall(const TransportAddress& dest,
                            simtime_t timeout)
{
    CoordsReqCall* coordReq = new CoordsReqCall("CoordsReq");
    coordReq->setBitLength(COORDSREQCALL_L(coordReq));
    neighborCache->sendRouteRpcCall(neighborCache->getThisCompType(), dest,
                                    coordReq, NULL, NO_OVERLAY_ROUTING,
                                    timeout, 0, -1, this);
    pendingRequests++;
}

void Nps::computeOwnLayer(const std::vector<LandmarkDataEntry>& landmarks)
{
    int8_t computedLayer = getOwnLayer();
    for (uint i = 0; i < landmarks.size(); i++) {
        if (computedLayer <= landmarks[i].layer) {
            computedLayer = landmarks[i].layer + 1;
        }
    }
    setOwnLayer(computedLayer);
}

void Nps::setOwnLayer(int8_t layer)
{
    ownCoords->setLayer(layer);

    // Update in BootstrapOracle
    PeerInfo* thisInfo = globalNodeList->getPeerInfo(neighborCache->getThisNode());
    thisInfo->setNpsLayer(layer);

    // Workaround against -1 ports in BS oracle
    if (layer > 0) globalNodeList->refreshEntry(neighborCache->overlay->getThisNode());
    if (layer < maxLayer) {
        globalNodeList->incLandmarkPeerSize();
        globalNodeList->incLandmarkPeerSizePerType(thisInfo->getTypeID());
    }
}

#ifdef EXTJOIN_DISCOVERY
bool Nps::checkCoordinates(std::vector<double> coordsOK, std::vector<double> coordsToCheck, simtime_t dist)
{
    simtime_t predDist = 0.0;

    for (uint i = 0; i < coordsOK.size(); i++) {
        predDist += pow(coordsOK[i] - coordsToCheck[i], 2);
    }
    simtime_t predDistLow = sqrt(SIMTIME_DBL(predDist)) * 2 * (1 - /*(coordBasedRouting->getCoordCheckPercentage() / 100)*/0.3) / 1000;
    simtime_t predDistHigh = sqrt(SIMTIME_DBL(predDist)) * 2 * (1 + /*(coordBasedRouting->getCoordCheckPercentage() / 100)*/0.3) / 1000;
    if (dist > predDistLow && dist < predDistHigh) {
        return true;
    } else {
//        std::cout << "check of Coordinates Failed! Intervall: [ " << predDistLow << " ; " << predDistHigh << " ] and dist: " << dist << endl;
        return false;
    }
}
#endif

void Nps::computeOwnCoordinates(const std::vector<LandmarkDataEntry>& landmarks)
{
    CoordCalcFunction coordcalcf(landmarks);

    Vec_DP initCoordinates(dimensions);
    Vec_DP bestCoordinates(dimensions);
    std::vector<double> computedCoordinatesStdVector(dimensions);

    double bestval;
    double resval;

    for (uint runs = 0; runs < coordCalcRuns; runs++) {
        // start with random coordinates (-100..100 in each dim)
        for (uint i = 0; i < dimensions; i++) {
            initCoordinates[i] = uniform(-100, 100);
        }
        // compute minimum coordinates via Simplex-Downhill minimum
        // function value is returned, coords are written into initCoordinates
        // (call by reference)
        resval = CoordCalcFunction::simplex_min(&coordcalcf, initCoordinates);
        if (runs == 0 || (runs > 0 && resval < bestval) ) {
            bestval = resval;
            bestCoordinates = initCoordinates;
        }
    }

    for (uint i = 0; i < dimensions; i++) {
        computedCoordinatesStdVector[i] = bestCoordinates[i];
    }

    setOwnCoordinates(computedCoordinatesStdVector);
}

std::vector<TransportAddress> Nps::getLandmarks(uint8_t howmany)
{
    std::vector<TransportAddress> returnPool;

    if (howmany > globalNodeList->getLandmarkPeerSize()) {
        throw cRuntimeError("Not enough landmarks available in network!");
    }

    while (returnPool.size() < howmany) {
        TransportAddress* lm = globalNodeList->getRandomAliveNode();
        PeerInfo* lmInfo = globalNodeList->getPeerInfo(lm->getIp());
        if (lmInfo->getNpsLayer() >= 0 &&
            lmInfo->getNpsLayer() < maxLayer) {
            // already in returnPool?
            bool alreadyin = false;
            for (uint8_t i = 0; i < returnPool.size(); i++) {
                if (returnPool[i] == *lm)
                    alreadyin = true;
            }
            if (alreadyin == false) {
                returnPool.push_back(*lm);
            }
        }
    }
    return returnPool;
}

bool Nps::enoughLandmarks()
{
    return (globalNodeList->getLandmarkPeerSize() > dimensions);
}


Prox Nps::getCoordinateBasedProx(const AbstractNcsNodeInfo& abstractInfo) const
{
    return ownCoords->getDistance(abstractInfo);

    /*
    if (!dynamic_cast<const GnpNpsCoordsInfo*>(&abstractInfo)) {
            throw cRuntimeError("GNP/NPS coords needed!");
    }

    const GnpNpsCoordsInfo& info =
        *(static_cast<const GnpNpsCoordsInfo*>(&abstractInfo));

    double dist = 0.0;
    uint32_t size = info.getDimension();

    for (uint32_t i = 0; i < size; i++) {
        dist += pow(ownCoords->getCoords(i) - info.getCoords(i), 2);
    }
    dist = sqrt(dist);

    return Prox(dist, 0.0); //TODO accuracy
    */
}

void Nps::updateNodeMeasurement(const TransportAddress& node,
                                uint8_t pending,
                                uint8_t sent,
                                uint8_t passed)
{
    bool alreadySet = false;
    for(uint i = 0; i < nodeMeasurements.size(); i++) {
        if (nodeMeasurements[i].measuredNode == node && sent == 0) {
            nodeMeasurements[i].rttsPending += pending;
            nodeMeasurements[i].rttsSent += sent;
            nodeMeasurements[i].coordsPassed += passed;
            alreadySet = true;
            i = nodeMeasurements.size();
        } else if (nodeMeasurements[i].measuredNode == node) {
            nodeMeasurements[i].rttsPending = pending;
            nodeMeasurements[i].rttsSent = sent;
            nodeMeasurements[i].coordsPassed = passed;
            alreadySet = true;
            i = nodeMeasurements.size();
        }
    }
    if (!alreadySet) {
        RttMeasurement newNode;
        newNode.measuredNode = node;
        newNode.rttsPending = pending;
        newNode.rttsSent = sent;
        newNode.coordsPassed = passed;
        nodeMeasurements.push_back(newNode);
    }
}

void Nps::deleteNodeMeasurement(const TransportAddress& node)
{
    for(uint i = 0; i < nodeMeasurements.size(); i++) {
        if (nodeMeasurements[i].measuredNode == node) {
#ifdef EXTJOIN_DISCOVERY
            delete nodeMeasurements[i].message;
#endif
            nodeMeasurements.erase(nodeMeasurements.begin()+i);
            i--;
        }
    }
    if (nodeMeasurements.size() == 0) {
        //stopNodeMeasurement();
    }
}


AbstractNcsNodeInfo* Nps::createNcsInfo(const std::vector<double>& coords) const
{
    assert(coords.size() > 1);
    GnpNpsCoordsInfo* info = new GnpNpsCoordsInfo();

    uint8_t i;
    for (i = 0; i < coords.size() - 1; ++i) {
        info->setCoords(i, coords[i]);
    }
    info->setLayer(coords[i]);

    return info;
}


double CoordCalcFunction::simplex_min(CoordCalcFunction *functionObject,
                                      Vec_DP& init)
{
    double accf = 0.0000001;
    double accx = 0.0000001;
    uint32_t nmax = 30001;
    uint8_t dim = init.size();
    Simplex spx(dim);

    int ihi;    // Index of highest point at start of each iteration
    Vec_DP phi(dim);   // Highest point at start of each iteration.
    double vhi;    // Function value at highest point at start of each iteration.
    double vre;    // Function value at reflected point.
    double vlo, diff;  // for checking convergence.
    uint32_t count;

    // Initialize Simplex
    Vec_DP tmp(dim);
    spx.functionObject = functionObject;
    spx[0] = init;
    for (uint8_t i = 0; i < dim; i++) {
        tmp[i] = 1;
        spx[i+1] = init + tmp;
        tmp[i] = 0;
    }

    Vec_DP debugCoords(dim);

    for (count = 1; count <= nmax; count++) {
        /*
        if ((count % 10000) == 0) {
            std::cout << "running loop #" << count << " of " << nmax << endl;
            std::cout << "diff: " << diff << std::endl;
            std::cout << "vhi: " << vhi << std::endl;
            std::cout << "vlo: " << vlo << std::endl;
            debugCoords = spx[spx.low()];
            std::cout << "Coords: " << debugCoords << std::endl;
        }
        */
        ihi = spx.high(&vhi);
        phi = spx[ihi];
        spx.reflect();
        vre = functionObject->f(spx[ihi]);
        if (vre < functionObject->f(spx[spx.low()])) {
            spx[ihi] = phi;  // Undo reflection.
            spx.reflect_exp();
            vre = functionObject->f(spx[ihi]);
            if (vre > functionObject->f(spx[spx.low()])) {
                spx[ihi] = phi;
                spx.reflect();
            }
        } else if (vre >= vhi) { // Equal sign important!
            spx[ihi] = phi;  // Undo reflection.
            spx.contract();
            if (functionObject->f(spx[ihi]) > vhi) {
                spx[ihi] = phi; // Undo contraction.
                spx.reduce();
            } // else contraction ok.
        } // else reflection ok

        spx.high(&vhi);
        spx.low(&vlo);
        diff = vhi - vlo;
        if (diff < accf)
            if (spx.size() < accx)
                break;
    }
    init = spx[spx.low()];
    return vlo;
}

double CoordCalcFunction::f(const Vec_DP& initCoordinates) const
{
    double sum = 0;
    double rel_diff = 0;

    for (uint i = 0; i < landmarks.size(); i++) {
        // Distance = RTT in ms / 2
        double diff = SIMTIME_DBL(landmarks[i].rtt) / 2 * 1000 -
            endnodeDistance(initCoordinates, landmarks[i]);

        if (SIMTIME_DBL(landmarks[i].rtt) != 0) {
            rel_diff = diff / (SIMTIME_DBL(landmarks[i].rtt) / 2 * 1000);
        } else {
            opp_error("[CBR] RTT == 0. Node is landmark? This shouldn't happen.");
        }
        sum += rel_diff * rel_diff;
    }
    return sum;
}

double CoordCalcFunction::endnodeDistance(const Vec_DP& nodeCoordinates,
                                          LandmarkDataEntry landmark) const
{
    double sum_of_squares = 0.0;
    for (int i = 0; i < nodeCoordinates.size(); i++) {
        sum_of_squares += pow(landmark.coordinates[i] - nodeCoordinates[i], 2);
    }
    double result = sqrt(sum_of_squares);
    return result;
}

