//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "ChannelMapBackgroundNoise.h"

#include "inet/physicallayer/analogmodel/packetlevel/ScalarNoise.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(ChannelMapBackgroundNoise);

ChannelMapBackgroundNoise::ChannelMapBackgroundNoise()
{
}

void ChannelMapBackgroundNoise::initialize(int stage)
{
    IsotropicScalarBackgroundNoise::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        parseXMLConfigFile();
    }
}

const INoise *ChannelMapBackgroundNoise::computeNoise(const IListening *listening) const
{

    if (freqMap.empty())
        return IsotropicScalarBackgroundNoise::computeNoise(listening);

    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz carrierFrequency = bandListening->getCarrierFrequency();

    // search
    for (auto elem : freqMap) {
        if (elem.freqBegin < carrierFrequency && carrierFrequency < elem.freqEnd) {
            const simtime_t startTime = listening->getStartTime();
            const simtime_t endTime = listening->getEndTime();
            std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
            powerChanges->insert(std::pair<simtime_t, W>(startTime, elem.power));
            powerChanges->insert(std::pair<simtime_t, W>(endTime, -elem.power));
            return new ScalarNoise(startTime, endTime, bandListening->getCarrierFrequency(), bandListening->getBandwidth(), powerChanges);
        }
    }
    return IsotropicScalarBackgroundNoise::computeNoise(listening);
}


void ChannelMapBackgroundNoise::parseXMLConfigFile()
{
    // configure interfaces from XML config file
    cXMLElement *config = par("noiseMap");
    for (cXMLElement *child = config->getFirstChild(); child; child = child->getNextSibling()) {
        //std::cout << "configuring interfaces from XML file." << endl;
        //std::cout << "selected element is: " << child->getTagName() << endl;
        // we ensure that the selected element is local.
        if (opp_strcmp(child->getTagName(), "frequency") != 0)
            continue;
        if (child->getAttribute("carrier") == nullptr) throw cRuntimeError("Attribute carrier not present");
        double cr = std::atof(child->getAttribute("carrier"));
        if (child->getAttribute("bandwidth") == nullptr) throw cRuntimeError("Attribute bandwidth not present");
        double bw = std::atof(child->getAttribute("bandwidth"));

        Values val;
        val.freqBegin = Hz(cr-bw);
        val.freqEnd = Hz(cr+bw);

        if (child->getAttribute("power") == nullptr) throw cRuntimeError("Attribute bandwidth not present");
        val.power = mW(math::dBm2mW(std::atof(child->getAttribute("power"))));

        if (freqMap.empty() || freqMap.back().freqEnd < val.freqEnd)
            freqMap.push_back(val);
        else {
            for (auto it = freqMap.begin();it != freqMap.end(); ++it) {
                // search position
                if (it->freqEnd > val.freqEnd) {
                    freqMap.insert(it, val);
                    break;
                }
            }
        }
    }
}

} // namespace physicallayer

} // namespace inet

