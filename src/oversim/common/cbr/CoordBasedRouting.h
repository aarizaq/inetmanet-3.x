//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file CoordBasedRouting.h
 * @author Fabian Hartmann
 */

#ifndef __COORDBASEDROUTING_H_
#define __COORDBASEDROUTING_H_

#include <omnetpp.h>
#include <OverlayKey.h>
#include <TransportAddress.h>

class GlobalNodeList;


/**
 * Auxiliary class for CoordBasedRouting:
 * Holds the min and max border values for each dimension and
 * the area's NodeID prefix
 */
class CBRArea
{
 public:
    CBRArea(uint8_t dim);
    ~CBRArea() {};

    std::vector<double> min;
    std::vector<double> max;
    std::string prefix;
};

class CoordBasedRouting : public cSimpleModule
{
 protected:
    /**
     * CBR is a global module, stuff in initialize() is run once
     * Parsing the area source XML is done here
     */
    virtual void initialize();
    void finish();

 private:
    /**
     * parses the area source XML and puts the resulting areas
     * into CBRAreaPool
     */
    void parseSource(const char* areaCoordinateSource);

    /**
     * auxiliary protected function which returns the NodeID prefix
     * of the given coords' area
     */
    std::string getPrefix(const std::vector<double>& coords) const;

    /**
     * returns if given coords' dimensions value (from underlay or overlay
     * calculations) is matching the dimensions value in the area source XML.
     * This is mandatory for correct mapping results!
     */
    bool checkDimensions(uint8_t dims) const;

 public:
    /**
     * returns a NodeID with given length and prefix according
     * to coords' area.
     * Takes the parameters CBRstartAtDigit and CBRstopAtDigit into account.
     * Non-prefix bits are currently randomized.
     */
    OverlayKey getNodeId(const std::vector<double>& coords,
                         uint8_t bpd, uint8_t length) const;

    /**
     * returns the number of dimensions set in the XML file.
     * Can be strictly regarded as reference whenever it comes to dimensionality
     */
    uint8_t getXmlDimensions() const { return xmlDimensions; }

    double getEuclidianDistanceByKeyAndCoords(const OverlayKey& destKey,
                                              const std::vector<double>& nodeCoords,
                                              uint8_t bpd) const;

 private:
    static const std::string NOPREFIX;

    const char* areaCoordinateSource;
    uint8_t cbrStartAtDigit;
    uint8_t cbrStopAtDigit;
    uint8_t xmlDimensions;

    std::vector<CBRArea*> CBRAreaPool;
    GlobalNodeList* globalNodeList;
};
#endif
