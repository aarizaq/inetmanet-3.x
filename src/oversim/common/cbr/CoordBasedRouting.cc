//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file CoordBasedRouting.cc
 * @author Fabian Hartmann
 */


#include <fstream>
#include <string>
#include <cassert>

#include <omnetpp.h>

#include <GlobalNodeListAccess.h>
#include <PeerInfo.h>

#include "CoordBasedRouting.h"


Define_Module(CoordBasedRouting);

const std::string CoordBasedRouting::NOPREFIX = "NOPREFIX";

void CoordBasedRouting::initialize()
{
    areaCoordinateSource = par("areaCoordinateSource");
    cbrStartAtDigit = par("CBRstartAtDigit");
    cbrStopAtDigit = par("CBRstopAtDigit");
    globalNodeList = GlobalNodeListAccess().get();

    // XML file found?
    std::ifstream check_for_xml_file(areaCoordinateSource);
    if (!check_for_xml_file) {
        check_for_xml_file.close();
        throw cRuntimeError("CBR area file not found!");
        return;
    }
    else {
        EV << "[CoordBasedRouting::initialize()]\n    CBR area file '"
           << areaCoordinateSource << "' loaded." << endl;
        check_for_xml_file.close();
    }

    // XML file found, let's parse it
    parseSource(areaCoordinateSource);
}

void CoordBasedRouting::finish()
{
    for (uint32_t i = 0; i < CBRAreaPool.size(); i++) {
        delete CBRAreaPool[i];
    }
    CBRAreaPool.clear();
}


void CoordBasedRouting::parseSource(const char* areaCoordinateSource)
{
    cXMLElement* rootElement = ev.getXMLDocument(areaCoordinateSource);

    xmlDimensions = atoi(rootElement->getAttribute("dimensions"));

    for (cXMLElement *area = rootElement->getFirstChildWithTag("area"); area;
         area = area->getNextSiblingWithTag("area") ) {
        CBRArea* tmpArea = new CBRArea(xmlDimensions);
        for (cXMLElement *areavals = area->getFirstChild(); areavals;
             areavals = areavals->getNextSibling() ) {
            std::string tagname = std::string(areavals->getTagName());
            if (tagname == "min") {
                uint8_t currentdim = atoi(areavals->getAttribute("dimension"));
                double value = atof(areavals->getNodeValue());
                tmpArea->min[currentdim] = value;
            }

            else if (tagname == "max") {
                uint8_t currentdim = atoi(areavals->getAttribute("dimension"));
                double value = atof(areavals->getNodeValue());
                tmpArea->max[currentdim] = value;
            }

            else if (tagname == "prefix") {
                tmpArea->prefix = areavals->getNodeValue();
            }
        }
        CBRAreaPool.push_back(tmpArea);
    }

    EV << "[CoordBasedRouting::parseSource()]" << endl;
    EV << "    " << CBRAreaPool.size() << " prefix areas detected." << endl;
}

OverlayKey CoordBasedRouting::getNodeId(const std::vector<double>& coords,
                                        uint8_t bpd, uint8_t length) const
{
    std::string prefix = getPrefix(coords);

    // if no prefix is returned, something is seriously wrong with the Area Source XML
    if (prefix == NOPREFIX) {
        opp_error("[CoordBasedRouting::getNodeId()]: "
                  "No prefix for given coords found. "
                  "Check your area source file!");
    }
    std::string idString;

    // ID string:
    //                          |- endPos
    // 00000000000000011010101010000000000000
    // |_startLength_||_prefix_||_endLength_|
    // |__  .. beforeEnd ..  __|
    // |___        .... length ....      ___|
    //
    // startLength and endLength bits are set to 0 at first, then
    // randomized
    // Prefix will be cut off if stop digit is exceeded

    uint8_t startLength = (bpd * cbrStartAtDigit < length) ?
                          (bpd * cbrStartAtDigit) : length;
    uint8_t beforeEnd = (startLength + prefix.length() < length) ?
                        (startLength + prefix.length()) : length;
    uint8_t endPos = (bpd * cbrStopAtDigit < beforeEnd) ?
                     (bpd * cbrStopAtDigit) : beforeEnd;
    uint8_t endLength = length - endPos;

    // Fill startLength bits with zeros
    for (uint8_t i = 0; i < startLength; i++)
        idString += "0";

    // Now add prefix and cut it off if stop digit and/or key length is exceeded
    idString += prefix;
    if (endPos < idString.length())
        idString.erase(endPos);
    if (length < idString.length())
        idString.erase(length);

    // fill endLength bits with zeros, thus key length is reached
    for (uint8_t i = 0; i < endLength; i++)
        idString += "0";

    OverlayKey nodeId(idString, 2);

    // randomize non-prefix (zero filled) parts
    if (startLength > 0)
        nodeId = nodeId.randomPrefix(length - startLength);
    if (endLength > 0)
        nodeId = nodeId.randomSuffix(endLength);

    EV << "[CoordBasedRouting::getNodeId()]\n"
       <<"    calculated id: " << nodeId << endl;
    return nodeId;
}

std::string CoordBasedRouting::getPrefix(const std::vector<double>& coords) const
{
    bool areaFound = false;
    uint32_t iter = 0;

    // Return no prefix if coords dimensions don't match area file dimensions
    if (!checkDimensions(coords.size()))
        return NOPREFIX;

    while (!areaFound && iter < CBRAreaPool.size()) {
        CBRArea* thisArea = CBRAreaPool[iter];

        // assume we're in the correct area unless any dimension tells us otherwise
        areaFound = true;
        for (uint8_t thisdim = 0; thisdim < coords.size(); thisdim++) {
            if (coords[thisdim] < thisArea->min[thisdim] ||
                coords[thisdim] > thisArea->max[thisdim]) {
                areaFound = false;
                break;
            }
        }

        // no borders are broken in any dimension -> we're in the correct area,
        // return corresponding prefix
        if (areaFound) {
            EV << "[CoordBasedRouting::getPrefix()]\n"
               <<"    calculated prefix: " << thisArea->prefix << endl;
            return thisArea->prefix;
        }
        iter++;
    }

    // no corresponding prefix found, XML file broken?
    EV << "[CoordBasedRouting::getPrefix()]\n"
       << "    No corresponding prefix found, check your area source file!"
       << endl;

    return NOPREFIX;
}

double CoordBasedRouting::getEuclidianDistanceByKeyAndCoords(const OverlayKey& destKey,
                                                             const std::vector<double>& coords,
                                                             uint8_t bpd) const
{
    assert(!destKey.isUnspecified());
    uint32_t iter = 0;
    while (iter < CBRAreaPool.size()) {
        CBRArea* thisArea = CBRAreaPool[iter];

        // Take CBR Start/Stop Digit into account
        uint8_t startbit = bpd * cbrStartAtDigit;
        uint8_t length = (bpd * cbrStopAtDigit - bpd * cbrStartAtDigit <
                (uint8_t)thisArea->prefix.length() - bpd * cbrStartAtDigit)
                ? (bpd * cbrStopAtDigit - bpd * cbrStartAtDigit)
                : (thisArea->prefix.length() - bpd * cbrStartAtDigit);
        if (destKey.toString(2).substr(startbit, length) ==
            thisArea->prefix.substr(startbit, length)) {
            // Get euclidian distance of area center to given coords
            std::vector<double> areaCenterCoords;
            areaCenterCoords.resize(getXmlDimensions());
            double sumofsquares = 0;
            for (uint8_t dim = 0; dim < getXmlDimensions(); dim++) {
                areaCenterCoords[dim] =
                    (thisArea->min[dim] + thisArea->max[dim]) / 2;
                sumofsquares += pow((coords[dim] - areaCenterCoords[dim]), 2);
            }
            return sqrt(sumofsquares);
        }
        iter++;
    }
    // while loop finished -> no area with needed prefix found
    // (this shouldn't happen!)
    throw cRuntimeError("[CoordBasedRouting::"
                        "getEuclidianDistanceByKeyAndCoords]: "
                        "No prefix for search key found!");
    return -1;
}


bool CoordBasedRouting::checkDimensions(uint8_t dims) const
{
    if (dims == xmlDimensions) {
        return true;
    } else {
        EV << "[CoordBasedRouting::checkDimensions()]" << endl;
        EV << "    ERROR: Given coordinate dimensions do not match dimensions "
              "in the used area source file. Mapping results will be wrong."
           << endl;
        return false;
    }
}


/**
 * CBRArea Constructor, reserves space for min & max vectors
 */
CBRArea::CBRArea(uint8_t dim)
{
    min.reserve(dim);
    max.reserve(dim);
}

