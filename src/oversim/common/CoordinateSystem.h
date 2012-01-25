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
 * @file CoordinateSystem.h
 * @author Bernhard Heep
 */

#ifndef COORDINATESYSTEM_H_
#define COORDINATESYSTEM_H_

#include <stdint.h>
#include <vector>

#include <omnetpp.h>

class Prox;
class NeighborCache;
class BaseCallMessage;


class AbstractNcsNodeInfo
{
public:
    virtual ~AbstractNcsNodeInfo() {};
    virtual bool isValid() = 0;
    virtual Prox getDistance(const AbstractNcsNodeInfo& node) const = 0;
    virtual bool update(const AbstractNcsNodeInfo& info) = 0;

    virtual operator std::vector<double>() const = 0;
};

class EuclideanNcsNodeInfo : public AbstractNcsNodeInfo
{
public:
    EuclideanNcsNodeInfo() { coordinates.resize(dim); };
    virtual ~EuclideanNcsNodeInfo() { };

    uint8_t getDimension() const { return coordinates.size(); };
    static void setDimension(uint8_t dimension) { dim = dimension; };

    double getCoords(uint8_t i) const {
        if (i >= coordinates.size()) {
            throw cRuntimeError("too high value for dim!");
        }
        return coordinates[i];
    };

    const std::vector<double>& getCoords() const { return coordinates; };

    void setCoords(uint8_t i, double value) {
        if (i >= coordinates.size()) {
            throw cRuntimeError("coordinates too small");
        }
        coordinates[i] = value;
    };

    Prox getDistance(const AbstractNcsNodeInfo& abstractInfo) const;

protected:
    std::vector<double> coordinates;
    static uint8_t dim;
};

class GnpNpsCoordsInfo : public EuclideanNcsNodeInfo
{
public:
    GnpNpsCoordsInfo() { npsLayer = -1; };

    bool isValid() { return npsLayer != -1; };

    int8_t getLayer() const { return npsLayer; };
    void setLayer(int8_t layer) { npsLayer = layer; };

    bool update(const AbstractNcsNodeInfo& abstractInfo);

    operator std::vector<double>() const;

protected:
    int8_t npsLayer;
};

std::ostream& operator<<(std::ostream& os, const GnpNpsCoordsInfo& info);


class SimpleCoordsInfo : public EuclideanNcsNodeInfo
{
  public:
    bool isValid() { return true; };

    Prox getDistance(const AbstractNcsNodeInfo& abstractInfo) const;
    bool update(const AbstractNcsNodeInfo& abstractInfo);

    simtime_t getAccessDelay() const { return accessDelay; };
    void setAccessDelay(simtime_t delay) { accessDelay = delay; };

    operator std::vector<double>() const;

protected:
    simtime_t accessDelay;
};

std::ostream& operator<<(std::ostream& os, const SimpleCoordsInfo& info);


class VivaldiCoordsInfo : public EuclideanNcsNodeInfo
{
public:
    VivaldiCoordsInfo(bool useHeightVector = false) {
        coordErr = 1.0;
        heightVector = (useHeightVector ? 0 : -1.0);
    };

    bool isValid() { return coordErr >= 0.0 && coordErr < 1.0; };

    double getError() const { return coordErr; };
    void setError(double err) {
        coordErr = ((err > 1.0) ? 1.0 : ((err < 0.0) ? 0.0 : coordErr = err));
    };

    double getHeightVector() const { return heightVector; };
    void setHeightVector(double height) {
        heightVector = ((height > 0.0) ? height : 0.0);
    };

    Prox getDistance(const AbstractNcsNodeInfo& node) const;
    bool update(const AbstractNcsNodeInfo& info);
    operator std::vector<double>() const ;

protected:
    double coordErr;
    double heightVector;
};

std::ostream& operator<<(std::ostream& os, const VivaldiCoordsInfo& info);

class AbstractNcs {
  public:
    virtual ~AbstractNcs() { };

    virtual void init(NeighborCache* neighorCache) = 0;

    virtual AbstractNcsNodeInfo* getUnvalidNcsInfo() const = 0;

    virtual Prox getCoordinateBasedProx(const AbstractNcsNodeInfo& node) const = 0;
    virtual void processCoordinates(const simtime_t& rtt,
                                    const AbstractNcsNodeInfo& nodeInfo) { };

    virtual const AbstractNcsNodeInfo& getOwnNcsInfo() const = 0;
    virtual AbstractNcsNodeInfo* createNcsInfo(const std::vector<double>& coords) const = 0;

    virtual void handleTimerEvent(cMessage* msg) { };
    virtual bool handleRpcCall(BaseCallMessage* msg) { return false; };
};

#endif /* COORDINATESYSTEM_H_ */
