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
 * @file Nps.h
 * @author Christian Fickinger, Bernhard Heep
 */

#ifndef NPS_H_
#define NPS_H_

#include <yang.h>
#include <simplex.h>

#include <RpcListener.h>
#include <Nps_m.h>
#include <CoordinateSystem.h>

class CoordBasedRouting;

/**
 *  Auxiliary struct that holds all data from a landmark
 *  that a node needs for its own coord computation
 */
struct LandmarkDataEntry
{
    simtime_t rtt;
    int8_t layer;
    std::vector<double> coordinates;
    const IPvXAddress* ip;
};

struct RttMeasurement
{
    TransportAddress measuredNode;
    char rttsPending;
    char rttsSent;
    char coordsPassed;
#ifdef EXTJOIN_DISCOVERY
    RttToNodeCall* message;
#endif
};

/**
 * Auxiliary class for coord calculation
 */
class CoordCalcFunction
{
  private:
    std::vector<LandmarkDataEntry> landmarks;

    double endnodeDistance(const Vec_DP& nodeCoordinates,
                           LandmarkDataEntry landmark) const;

  public:
    CoordCalcFunction(const std::vector<LandmarkDataEntry> myLandmarks) : landmarks(myLandmarks) { };
    double f(const Vec_DP& initCoordinates) const;
    static double simplex_min(CoordCalcFunction *functionObject, Vec_DP& init);
};

class Nps : public RpcListener, public AbstractNcs
{
    friend class Landmark;

  private:
    NeighborCache* neighborCache;
    BaseOverlay* overlay;
    GlobalNodeList* globalNodeList;
    CoordBasedRouting* coordBasedRouting;

    uint8_t maxLayer;
    uint8_t dimensions;
    simtime_t landmarkTimeout;

    GnpNpsCoordsInfo* ownCoords; //TODO

    int16_t pendingRequests;
    uint8_t coordCalcRuns;

    std::vector<TransportAddress> landmarkSet;

    uint16_t receivedCalls;

    cMessage* landmarkTimer;

  public:
    Nps() {};
    void init(NeighborCache* neighborCache);

    void handleTimerEvent(cMessage* msg);
    virtual bool handleRpcCall(BaseCallMessage* msg);

    Prox getCoordinateBasedProx(const AbstractNcsNodeInfo& info) const;
    AbstractNcsNodeInfo* getUnvalidNcsInfo() const {return new GnpNpsCoordsInfo; };
    AbstractNcsNodeInfo* createNcsInfo(const std::vector<double>& coords) const;
    virtual const AbstractNcsNodeInfo& getOwnNcsInfo() const { return *ownCoords; };

    const std::vector<double>& getOwnCoordinates() const { return ownCoords->getCoords(); };
    double getOwnCoordinates(uint8_t i) const { return ownCoords->getCoords(i); };
    uint8_t getOwnLayer() const { return ownCoords->getLayer(); };

  protected:
    /**
     *  computes this node's NPS layer (max of reference points' layers + 1)
     */
    void computeOwnLayer (const std::vector<LandmarkDataEntry>& landmarks);

    /**
     *  methods for computing own coordinates with the received data
     */
    void computeOwnCoordinates (const std::vector<LandmarkDataEntry>& landmarks);

    /**
     *  announces node's NPS layer to Bootstrap Oracle and Neighbor Cache
     */
    void setOwnLayer(int8_t layer);
    void setOwnCoordinates(const std::vector<double>& coords) {
        for (uint8_t i = 0; i < coords.size(); ++i) {
            ownCoords->setCoords(i, coords[i]);
        }
    };

    /**
     *  Get a random set of landmarks / NPS reference node and
     *  send coordinate requests to them
     */
    void sendCoordRequests();
    void sendCoordRequests(const std::vector<TransportAddress>& landmarks);
    void sendCoordsReqCall(const TransportAddress& dest,
                              simtime_t timeout);

    void handleRpcResponse(BaseResponseMessage* msg,
                           cPolymorphic* context,
                           int rpcId, simtime_t rtt);

    void handleRpcTimeout(BaseCallMessage* msg,
                          const TransportAddress& dest,
                          cPolymorphic* context, int rpcId,
                          const OverlayKey& destKey);

    void coordsReqRpcResponse(CoordsReqResponse* response,
                              cPolymorphic* context,
                              int rpcId, simtime_t rtt);

    void coordsReqRpc(CoordsReqCall* msg);

    std::vector<LandmarkDataEntry> getLandmarkData() const;

    /**
     * Askes GlobalNodeList for appropriate landmark nodes
     *
     * @param howmany number of needed landmark nodes
     * @returns vector of landmark addresses
     */
    std::vector<TransportAddress> getLandmarks(uint8_t howmany);
    const std::vector<TransportAddress>& getLandmarkSet() const { return landmarkSet; };
    uint16_t getLandmarkSetSize() const { return landmarkSet.size(); };
    //bool setLandmarkSet(uint8_t dim, uint8_t maxLayer);
    bool setLandmarkSet(uint8_t howManyLM, uint8_t maxLayer,
                        std::vector<TransportAddress>* landmarkSet);
    bool enoughLandmarks();

    std::vector<RttMeasurement> nodeMeasurements;

    void updateNodeMeasurement(const TransportAddress& node,
                               uint8_t pending = 0,
                               uint8_t sent = 0,
                               uint8_t passed = 0);

    void deleteNodeMeasurement(const TransportAddress& node);

    uint16_t getReceivedCalls() const { return receivedCalls; };
};

#endif /* NPS_H_ */
