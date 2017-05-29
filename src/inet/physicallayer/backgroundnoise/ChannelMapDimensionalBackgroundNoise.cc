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

#include "inet/physicallayer/backgroundnoise/ChannelMapDimensionalBackgroundNoise.h"
#include "inet/physicallayer/analogmodel/packetlevel/DimensionalNoise.h"
#include "inet/physicallayer/common/packetlevel/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(ChannelMapDimensionalBackgroundNoise);

ChannelMapDimensionalBackgroundNoise::ChannelMapDimensionalBackgroundNoise()
{
}

void ChannelMapDimensionalBackgroundNoise::initialize(int stage)
{
    IsotropicDimensionalBackgroundNoise::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        parseXMLConfigFile();
    }
}

std::ostream& ChannelMapDimensionalBackgroundNoise::printToStream(std::ostream& stream, int level) const
{
    stream << "IsotropicDimensionalBackgroundNoise";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << ", power = " << power;
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", interpolationMode = " << interpolationMode
               << ", dimensions = " << dimensions ;
    return stream;
}

const INoise *ChannelMapDimensionalBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    const simtime_t startTime = listening->getStartTime();
    const simtime_t endTime = listening->getEndTime();
    Hz carrierFrequency = bandListening->getCarrierFrequency();
    Hz bandwidth = bandListening->getBandwidth();

    if (freqMap.empty())
        return IsotropicDimensionalBackgroundNoise::computeNoise(listening);

    // search
    for (auto elem : freqMap) {
        if (elem.freqBegin < carrierFrequency && carrierFrequency < elem.freqEnd) {
            Mapping *powerMapping = MappingUtils::createMapping(Argument::MappedZero, Dimension::frequency, interpolationMode);
            Argument position(Dimension::frequency);
            position.setArgValue(Dimension::frequency, 0);
            powerMapping->setValue(position, 0);
            position.setArgValue(Dimension::frequency, (carrierFrequency - bandwidth / 2).get());
            powerMapping->setValue(position, elem.power.get());
            position.setArgValue(Dimension::frequency, (carrierFrequency + bandwidth / 2).get());
            powerMapping->setValue(position, 0);
            return new DimensionalNoise(startTime, endTime, carrierFrequency, bandwidth, powerMapping);
        }
    }
    return IsotropicDimensionalBackgroundNoise::computeNoise(listening);
}


void ChannelMapDimensionalBackgroundNoise::parseXMLConfigFile()
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

