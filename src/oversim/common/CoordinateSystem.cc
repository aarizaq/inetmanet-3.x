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
 * @file CoordinateSystem.cc
 * @author Bernhard Heep
 */

#include <ProxNodeHandle.h>

#include <CoordinateSystem.h>


uint8_t EuclideanNcsNodeInfo::dim;

Prox EuclideanNcsNodeInfo::getDistance(const AbstractNcsNodeInfo& abstractInfo) const
{
    if (!dynamic_cast<const EuclideanNcsNodeInfo*>(&abstractInfo)) {
        return Prox::PROX_UNKNOWN;
    }
    const EuclideanNcsNodeInfo& info =
        *(static_cast<const EuclideanNcsNodeInfo*>(&abstractInfo));

    double dist = 0.0;

    for (uint8_t i = 0; i < info.getDimension(); ++i) {
        dist += pow(getCoords(i) - info.getCoords(i), 2);
    }
    dist = sqrt(dist);

    return Prox(dist, 0.7); //TODO
}

bool GnpNpsCoordsInfo::update(const AbstractNcsNodeInfo& abstractInfo)
{
    if (!dynamic_cast<const GnpNpsCoordsInfo*>(&abstractInfo)) return false;

    const GnpNpsCoordsInfo& temp =
        static_cast<const GnpNpsCoordsInfo&>(abstractInfo);

    coordinates = temp.coordinates;
    npsLayer = temp.npsLayer;

    return true;
}

GnpNpsCoordsInfo::operator std::vector<double>() const
{
    std::vector<double> temp;
    for (uint8_t i = 0; i < coordinates.size(); ++i) {
        temp.push_back(coordinates[i]);
    }
    temp.push_back(npsLayer);

    return temp;
}

std::ostream& operator<<(std::ostream& os, const GnpNpsCoordsInfo& info)
{
    if (!info.getCoords().size()) throw cRuntimeError("dim = 0");

    os << "< ";
    uint8_t i;
    for (i = 0; i < info.getCoords().size() - 1; ++i) {
        os << info.getCoords(i) << ", ";
    }
    os << info.getCoords(i) << " >";
    if (info.getLayer() != -1)
        os << ", NPS-Layer = " << (int)info.getLayer();

    return os;
}

Prox VivaldiCoordsInfo::getDistance(const AbstractNcsNodeInfo& abstractInfo) const
{
    if (!dynamic_cast<const VivaldiCoordsInfo*>(&abstractInfo)) {
            return Prox::PROX_UNKNOWN;
    }
    const VivaldiCoordsInfo& info =
        *(static_cast<const VivaldiCoordsInfo*>(&abstractInfo));

    double dist = 0.0, accuracy = 0.0;

    for (uint8_t i = 0; i < info.getDimension(); ++i) {
        dist += pow(getCoords(i) - info.getCoords(i), 2);
    }
    dist = sqrt(dist);

    accuracy = 1 - ((info.getError() + getError()) / 2);
    if (info.getError() >= 1.0 || getError() >= 1.0) accuracy = 0.0;
    if (accuracy < 0) accuracy = 0.0;
    if (accuracy > 1) accuracy = 1;

    if (getHeightVector() != -1.0 && info.getHeightVector() != -1.0) {
        return Prox(dist + getHeightVector() + info.getHeightVector(),
                    info.getError());
    }
    return Prox(dist, accuracy);
}

bool VivaldiCoordsInfo::update(const AbstractNcsNodeInfo& info)
{
    if (!dynamic_cast<const VivaldiCoordsInfo*>(&info)) return false;

    const VivaldiCoordsInfo& temp = static_cast<const VivaldiCoordsInfo&>(info);
    if (coordErr > temp.coordErr) {
        coordErr = temp.coordErr;
        coordinates = temp.coordinates;
        heightVector = temp.heightVector;

        return true;
    }
    return false;
}

VivaldiCoordsInfo::operator std::vector<double>() const
{
    std::vector<double> temp;
    for (uint8_t i = 0; i < coordinates.size(); ++i) {
        temp.push_back(coordinates[i]);
    }
    temp.push_back(coordErr);
    if (heightVector >= 0) temp.push_back(heightVector);

    return temp;
}

std::ostream& operator<<(std::ostream& os, const VivaldiCoordsInfo& info)
{
    if (!info.getCoords().size()) throw cRuntimeError("dim = 0");

    os << "< ";
    uint8_t i;
    for (i = 0; i < info.getCoords().size() - 1; ++i) {
        os << info.getCoords(i) << ", ";
    }
    os << info.getCoords(i) << " >";
    os << ", Err = " << info.getError();
    if (info.getHeightVector() != -1.0)
        os << ", HeightVec = " << info.getHeightVector();

    return os;
}


Prox SimpleCoordsInfo::getDistance(const AbstractNcsNodeInfo& abstractInfo) const
{
    const SimpleCoordsInfo& temp =
        dynamic_cast<const SimpleCoordsInfo&>(abstractInfo);

    return Prox(2 * (accessDelay +
                temp.getAccessDelay() +
                EuclideanNcsNodeInfo::getDistance(abstractInfo)));
}


bool SimpleCoordsInfo::update(const AbstractNcsNodeInfo& abstractInfo)
{
    if (!dynamic_cast<const SimpleCoordsInfo*>(&abstractInfo)) return false;

    const SimpleCoordsInfo& temp =
        static_cast<const SimpleCoordsInfo&>(abstractInfo);

    coordinates = temp.coordinates;

    return true;
}


SimpleCoordsInfo::operator std::vector<double>() const
{
    std::vector<double> temp;
    for (uint8_t i = 0; i < coordinates.size(); ++i) {
        temp.push_back(coordinates[i]);
    }
    temp.push_back(SIMTIME_DBL(accessDelay));

    return temp;
}
