//
// Copyright (C) 2015 OpenSim Ltd.
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

#include "inet/physicallayer/ieee80211/mode/Ieee80211VHTMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211VHTCode.h"
#include "inet/physicallayer/modulation/QBPSKModulation.h"
#include <tuple>

namespace inet {
namespace physicallayer {


Ieee80211VHTCompliantModes Ieee80211VHTCompliantModes::singleton;

Ieee80211VHTMode::Ieee80211VHTMode(const char *name, const Ieee80211VHTPreambleMode* preambleMode, const Ieee80211VHTDataMode* dataMode, const BandMode carrierFrequencyMode) :
        Ieee80211ModeBase(name),
        preambleMode(preambleMode),
        dataMode(dataMode),
        carrierFrequencyMode(carrierFrequencyMode)
{
}

Ieee80211VHTModeBase::Ieee80211VHTModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        bandwidth(bandwidth),
        guardIntervalType(guardIntervalType),
        mcsIndex(modulationAndCodingScheme),
        numberOfSpatialStreams(numberOfSpatialStreams),
        netBitrate(bps(NaN)),
        grossBitrate(bps(NaN))
{
}

Ieee80211VHTPreambleMode::Ieee80211VHTPreambleMode(const Ieee80211VHTSignalMode* highThroughputSignalMode, const Ieee80211OFDMSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream) :
        highThroughputSignalMode(highThroughputSignalMode),
        legacySignalMode(legacySignalMode),
        preambleFormat(preambleFormat),
        numberOfHTLongTrainings(computeNumberOfHTLongTrainings(computeNumberOfSpaceTimeStreams(numberOfSpatialStream)))
{
}

Ieee80211VHTSignalMode::Ieee80211VHTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation *modulation, const Ieee80211VHTCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211VHTModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
        modulation(modulation),
        code(code)
{
}

Ieee80211VHTSignalMode::Ieee80211VHTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation* modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211VHTModeBase(modulationAndCodingScheme, 1, bandwidth, guardIntervalType),
        modulation(modulation),
        code(Ieee80211VHTCompliantCodes::getCompliantCode(convolutionalCode, modulation, nullptr, nullptr, nullptr, bandwidth, false))
{
}


Ieee80211VHTDataMode::Ieee80211VHTDataMode(const Ieee80211VHTMCS *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType) :
        Ieee80211VHTModeBase(modulationAndCodingScheme->getMcsIndex(), computeNumberOfSpatialStreams(modulationAndCodingScheme->getModulation(), modulationAndCodingScheme->getStreamExtension1Modulation(), modulationAndCodingScheme->getStreamExtension2Modulation(), modulationAndCodingScheme->getStreamExtension3Modulation()), bandwidth, guardIntervalType),
        modulationAndCodingScheme(modulationAndCodingScheme),
        numberOfBccEncoders(computeNumberOfBccEncoders())
{
}

Ieee80211VHTMCS::Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211VHTCode* code, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(stream4Modulation),
    code(code)
{
}

Ieee80211VHTMCS::Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(stream4Modulation),
    code(Ieee80211VHTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211VHTMCS::Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(stream3Modulation),
    stream4Modulation(nullptr),
    code(Ieee80211VHTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211VHTMCS::Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(stream2Modulation),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    code(Ieee80211VHTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

Ieee80211VHTMCS::Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth) :
    mcsIndex(mcsIndex),
    stream1Modulation(stream1Modulation),
    stream2Modulation(nullptr),
    stream3Modulation(nullptr),
    stream4Modulation(nullptr),
    code(Ieee80211VHTCompliantCodes::getCompliantCode(convolutionalCode, stream1Modulation, stream2Modulation, stream3Modulation, stream4Modulation, bandwidth)),
    bandwidth(bandwidth)
{
}

const simtime_t Ieee80211VHTPreambleMode::getFirstHTLongTrainingFieldDuration() const
{
    if (preambleFormat == HT_PREAMBLE_MIXED)
        return simtime_t(4E-6);
    else if (preambleFormat == HT_PREAMBLE_GREENFIELD)
        return simtime_t(8E-6);
    else
        throw cRuntimeError("Unknown preamble format");
}


unsigned int Ieee80211VHTPreambleMode::computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const
{
    // Table 20-12—Determining the number of space-time streams
    return numberOfSpatialStreams + highThroughputSignalMode->getSTBC();
}

unsigned int Ieee80211VHTPreambleMode::computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const
{
    // If the transmitter is providing training for exactly the space-time
    // streams (spatial mapper inputs) used for the transmission of the PSDU,
    // the number of training symbols, N_LTF, is equal to the number of space-time
    // streams, N STS, except that for three space-time streams, four training symbols
    // are required.
    return numberOfSpaceTimeStreams == 3 ? 4 : numberOfSpaceTimeStreams;
}

const simtime_t Ieee80211VHTPreambleMode::getDuration() const
{
    // 20.3.7 Mathematical description of signals
    simtime_t sumOfHTLTFs = getFirstHTLongTrainingFieldDuration() + getSecondAndSubsequentHTLongTrainingFielDuration() * (numberOfHTLongTrainings - 1);
    if (preambleFormat == HT_PREAMBLE_MIXED)
        // L-STF -> L-LTF -> L-SIG -> HT-SIG -> HT-STF -> HT-LTF1 -> HT-LTF2 -> ... -> HT_LTFn
        return getNonHTShortTrainingSequenceDuration() + getNonHTLongTrainingFieldDuration() + legacySignalMode->getDuration() + highThroughputSignalMode->getDuration() + getHTShortTrainingFieldDuration() + sumOfHTLTFs;
    else if (preambleFormat == HT_PREAMBLE_GREENFIELD)
        // HT-GF-STF -> HT-LTF1 -> HT-SIG -> HT-LTF2 -> ... -> HT-LTFn
        return getHTGreenfieldShortTrainingFieldDuration() + highThroughputSignalMode->getDuration() + sumOfHTLTFs;
    else
        throw cRuntimeError("Unknown preamble format");
}

bps Ieee80211VHTSignalMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSymbol = modulation->getSubcarrierModulation()->getCodeWordSize() * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211VHTSignalMode::computeNetBitrate() const
{
    return computeGrossBitrate() * code->getForwardErrorCorrection()->getCodeRate();
}


int Ieee80211VHTSignalMode::getBitLength() const
{
    return
        getMCSLength() +
        getCBWLength() +
        getHTLengthLength() +
        getSmoothingLength() +
        getNotSoundingLength() +
        getReservedLength() +
        getAggregationLength() +
        getSTBCLength() +
        getFECCodingLength() +
        getShortGILength() +
        getNumOfExtensionSpatialStreamsLength() +
        getCRCLength() +
        getTailBitsLength();
}

bps Ieee80211VHTDataMode::computeGrossBitrate() const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = computeNumberOfCodedBitsPerSubcarrierSum();
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    if (guardIntervalType == HT_GUARD_INTERVAL_LONG)
        return bps(numberOfCodedBitsPerSymbol / getSymbolInterval());
    else if (guardIntervalType == HT_GUARD_INTERVAL_SHORT)
        return bps(numberOfCodedBitsPerSymbol / getShortGISymbolInterval());
    else
        throw cRuntimeError("Unknown guard interval type");
}

bps Ieee80211VHTDataMode::computeNetBitrate() const
{
    return getGrossBitrate() * getCode()->getForwardErrorCorrection()->getCodeRate();
}

bps Ieee80211VHTModeBase::getNetBitrate() const
{
    if (std::isnan(netBitrate.get()))
        netBitrate = computeNetBitrate();
    return netBitrate;
}

bps Ieee80211VHTModeBase::getGrossBitrate() const
{
    if (std::isnan(grossBitrate.get()))
        grossBitrate = computeGrossBitrate();
    return grossBitrate;
}

int Ieee80211VHTModeBase::getNumberOfDataSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 52;
    else if (bandwidth == MHz(40))
        return mcsIndex ==  108;
    else if (bandwidth == MHz(80))
        return mcsIndex ==  234;
    else if (bandwidth == MHz(160))
        return mcsIndex ==  468;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

int Ieee80211VHTModeBase::getNumberOfPilotSubcarriers() const
{
    if (bandwidth == MHz(20))
        return 4;
    else if (bandwidth == MHz(40))
        // It is a spacial case, see the comment above.
        return 6;
    else if (bandwidth == MHz(80))
        // It is a spacial case, see the comment above.
        return 8;
    else if (bandwidth == MHz(160))
        // It is a spacial case, see the comment above.
        return 16;
    else
        throw cRuntimeError("Unsupported bandwidth");
}

int Ieee80211VHTDataMode::getBitLength(int dataBitLength) const
{
    return getServiceBitLength() + getTailBitLength() + dataBitLength; // TODO: padding?
}

unsigned int Ieee80211VHTDataMode::computeNumberOfSpatialStreams(const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) const
{
    return (stream1Modulation ? 1 : 0) + (stream2Modulation ? 1 : 0) +
           (stream3Modulation ? 1 : 0) + (stream4Modulation ? 1 : 0);
}

unsigned int Ieee80211VHTDataMode::computeNumberOfCodedBitsPerSubcarrierSum() const
{
    return
        (modulationAndCodingScheme->getModulation() ? modulationAndCodingScheme->getModulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension1Modulation() ? modulationAndCodingScheme->getStreamExtension1Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension2Modulation()? modulationAndCodingScheme->getStreamExtension2Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0) +
        (modulationAndCodingScheme->getStreamExtension3Modulation() ? modulationAndCodingScheme->getStreamExtension3Modulation()->getSubcarrierModulation()->getCodeWordSize() : 0);
}

unsigned int Ieee80211VHTDataMode::computeNumberOfBccEncoders() const
{
    // When the BCC FEC encoder is used, a single encoder is used, except that two encoders
    // are used when the selected MCS has a PHY rate greater than 300 Mb/s (see 20.6).
    if (getGrossBitrate() < Mbps(540))
        return 1;
    else if (getGrossBitrate() < Mbps(1170))
        return 2;
    else
        return 3;
}

const simtime_t Ieee80211VHTDataMode::getDuration(int dataBitLength) const
{
    unsigned int numberOfCodedBitsPerSubcarrierSum = computeNumberOfCodedBitsPerSubcarrierSum();
    unsigned int numberOfCodedBitsPerSymbol = numberOfCodedBitsPerSubcarrierSum * getNumberOfDataSubcarriers();
    const IForwardErrorCorrection *forwardErrorCorrection = getCode() ? getCode()->getForwardErrorCorrection() : nullptr;
    unsigned int dataBitsPerSymbol = forwardErrorCorrection ? forwardErrorCorrection->getDecodedLength(numberOfCodedBitsPerSymbol) : numberOfCodedBitsPerSymbol;
    int numberOfSymbols = lrint(ceil((double)getBitLength(dataBitLength) / dataBitsPerSymbol)); // TODO: getBitLength(dataBitLength) should be divisible by dataBitsPerSymbol
    return numberOfSymbols * getSymbolInterval();
}

const simtime_t Ieee80211VHTMode::getSlotTime() const
{
    if (carrierFrequencyMode  == BAND_5GHZ)
        return 9E-6;
    else
        throw cRuntimeError("Unsupported carrier frequency");
}

inline const simtime_t Ieee80211VHTMode::getSifsTime() const
{
    if (carrierFrequencyMode == BAND_5GHZ)
        return 16E-6;
    else
        throw cRuntimeError("Sifs time is not defined for this carrier frequency"); // TODO
}

const simtime_t Ieee80211VHTMode::getShortSlotTime() const
{
    return 9E-6;
}

Ieee80211VHTCompliantModes::Ieee80211VHTCompliantModes()
{
}

Ieee80211VHTCompliantModes::~Ieee80211VHTCompliantModes()
{
    for (auto & entry : modeCache)
        delete entry.second;
}

const Ieee80211VHTMode* Ieee80211VHTCompliantModes::getCompliantMode(const Ieee80211VHTMCS *mcsMode, Ieee80211VHTMode::BandMode carrierFrequencyMode, Ieee80211VHTPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211VHTModeBase::GuardIntervalType guardIntervalType)
{
    const char *name =""; //TODO
    auto htModeId = std::make_tuple(mcsMode->getBandwidth(), mcsMode->getMcsIndex(), guardIntervalType);
    auto mode = singleton.modeCache.find(htModeId);
    if (mode == std::end(singleton.modeCache))
    {
        const Ieee80211OFDMSignalMode *legacySignal = nullptr;
        const Ieee80211VHTSignalMode *htSignal = nullptr;
        if (preambleFormat == Ieee80211VHTPreambleMode::HT_PREAMBLE_GREENFIELD)
            htSignal = new Ieee80211VHTSignalMode(mcsMode->getMcsIndex(), &Ieee80211OFDMCompliantModulations::bpskModulation, Ieee80211VHTCompliantCodes::getCompliantCode(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantModulations::bpskModulation, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
        else if (preambleFormat == Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED)
        {
            legacySignal = &Ieee80211OFDMCompliantModes::ofdmHeaderMode6MbpsRate13;
            htSignal = new Ieee80211VHTSignalMode(mcsMode->getMcsIndex(), &Ieee80211OFDMCompliantModulations::qbpskModulation, Ieee80211VHTCompliantCodes::getCompliantCode(&Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, &Ieee80211OFDMCompliantModulations::qbpskModulation, nullptr, nullptr, nullptr, mcsMode->getBandwidth(), false), mcsMode->getBandwidth(), guardIntervalType);
        }
        else
            throw cRuntimeError("Unknown preamble format");
        const Ieee80211VHTDataMode *dataMode = new Ieee80211VHTDataMode(mcsMode, mcsMode->getBandwidth(), guardIntervalType);
        const Ieee80211VHTPreambleMode *preambleMode = new Ieee80211VHTPreambleMode(htSignal, legacySignal, preambleFormat, dataMode->getNumberOfSpatialStreams());
        const Ieee80211VHTMode *htMode = new Ieee80211VHTMode(name, preambleMode, dataMode, carrierFrequencyMode);
        singleton.modeCache.insert(std::pair<std::tuple<Hz, unsigned int, Ieee80211VHTModeBase::GuardIntervalType>, const Ieee80211VHTMode *>(htModeId, htMode));
        return htMode;
    }
    return mode->second;
}

Ieee80211VHTMCS::~Ieee80211VHTMCS()
{
    delete code;
}

Ieee80211VHTSignalMode::~Ieee80211VHTSignalMode()
{
    delete code;
}


const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs0BW20MHz([](){ return new Ieee80211VHTMCS(0, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs1BW20MHz([](){ return new Ieee80211VHTMCS(1, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs2BW20MHz([](){ return new Ieee80211VHTMCS(2, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs3BW20MHz([](){ return new Ieee80211VHTMCS(3, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs4BW20MHz([](){ return new Ieee80211VHTMCS(4, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs5BW20MHz([](){ return new Ieee80211VHTMCS(5, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs6BW20MHz([](){ return new Ieee80211VHTMCS(6, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs7BW20MHz([](){ return new Ieee80211VHTMCS(7, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs8BW20MHz([](){ return new Ieee80211VHTMCS(8, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs9BW20MHz([](){ return nullptr;});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs10BW20MHz([](){ return new Ieee80211VHTMCS(10, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs11BW20MHz([](){ return new Ieee80211VHTMCS(11, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs12BW20MHz([](){ return new Ieee80211VHTMCS(12, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs13BW20MHz([](){ return new Ieee80211VHTMCS(13, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs14BW20MHz([](){ return new Ieee80211VHTMCS(14, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs15BW20MHz([](){ return new Ieee80211VHTMCS(15, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs16BW20MHz([](){ return new Ieee80211VHTMCS(16, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs17BW20MHz([](){ return new Ieee80211VHTMCS(17, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs18BW20MHz([](){ return new Ieee80211VHTMCS(18, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs19BW20MHz([](){ return nullptr;});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs20BW20MHz([](){ return new Ieee80211VHTMCS(20, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs21BW20MHz([](){ return new Ieee80211VHTMCS(21, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs22BW20MHz([](){ return new Ieee80211VHTMCS(22, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs23BW20MHz([](){ return new Ieee80211VHTMCS(23, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs24BW20MHz([](){ return new Ieee80211VHTMCS(24, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs25BW20MHz([](){ return new Ieee80211VHTMCS(25, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs26BW20MHz([](){ return new Ieee80211VHTMCS(26, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs27BW20MHz([](){ return new Ieee80211VHTMCS(27, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs28BW20MHz([](){ return new Ieee80211VHTMCS(28, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs29BW20MHz([](){ return nullptr;});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs30BW20MHz([](){ return new Ieee80211VHTMCS(30, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs31BW20MHz([](){ return new Ieee80211VHTMCS(31, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs32BW20MHz([](){ return new Ieee80211VHTMCS(32, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs33BW20MHz([](){ return new Ieee80211VHTMCS(33, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs34BW20MHz([](){ return new Ieee80211VHTMCS(34, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs35BW20MHz([](){ return new Ieee80211VHTMCS(35, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs36BW20MHz([](){ return new Ieee80211VHTMCS(36, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs37BW20MHz([](){ return new Ieee80211VHTMCS(37, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs38BW20MHz([](){ return new Ieee80211VHTMCS(38, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs39BW20MHz([](){ return nullptr;});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs40BW20MHz([](){ return new Ieee80211VHTMCS(40, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs41BW20MHz([](){ return new Ieee80211VHTMCS(41, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs42BW20MHz([](){ return new Ieee80211VHTMCS(42, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs43BW20MHz([](){ return new Ieee80211VHTMCS(43, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs44BW20MHz([](){ return new Ieee80211VHTMCS(44, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs45BW20MHz([](){ return new Ieee80211VHTMCS(45, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs46BW20MHz([](){ return new Ieee80211VHTMCS(46, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs47BW20MHz([](){ return new Ieee80211VHTMCS(47, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs48BW20MHz([](){ return new Ieee80211VHTMCS(48, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs49BW20MHz([](){ return nullptr;});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs50BW20MHz([](){ return new Ieee80211VHTMCS(50, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs51BW20MHz([](){ return new Ieee80211VHTMCS(51, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs52BW20MHz([](){ return new Ieee80211VHTMCS(52, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs53BW20MHz([](){ return new Ieee80211VHTMCS(53, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs54BW20MHz([](){ return new Ieee80211VHTMCS(54, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs55BW20MHz([](){ return new Ieee80211VHTMCS(55, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs56BW20MHz([](){ return new Ieee80211VHTMCS(56, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs57BW20MHz([](){ return new Ieee80211VHTMCS(57, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs58BW20MHz([](){ return new Ieee80211VHTMCS(58, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs59BW20MHz([](){ return nullptr;});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs60BW20MHz([](){ return new Ieee80211VHTMCS(60, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs61BW20MHz([](){ return new Ieee80211VHTMCS(61, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs62BW20MHz([](){ return new Ieee80211VHTMCS(62, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs63BW20MHz([](){ return new Ieee80211VHTMCS(63, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs64BW20MHz([](){ return new Ieee80211VHTMCS(64, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs65BW20MHz([](){ return new Ieee80211VHTMCS(65, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs66BW20MHz([](){ return new Ieee80211VHTMCS(66, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs67BW20MHz([](){ return new Ieee80211VHTMCS(67, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs68BW20MHz([](){ return new Ieee80211VHTMCS(68, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs69BW20MHz([](){ return nullptr;})

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs70BW20MHz([](){ return new Ieee80211VHTMCS(70, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs71BW20MHz([](){ return new Ieee80211VHTMCS(71, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs72BW20MHz([](){ return new Ieee80211VHTMCS(72, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs73BW20MHz([](){ return new Ieee80211VHTMCS(73, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs74BW20MHz([](){ return new Ieee80211VHTMCS(74, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs75BW20MHz([](){ return new Ieee80211VHTMCS(75, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs76BW20MHz([](){ return new Ieee80211VHTMCS(76, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs77BW20MHz([](){ return new Ieee80211VHTMCS(77, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs78BW20MHz([](){ return new Ieee80211VHTMCS(78, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(20));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs79BW20MHz([](){ return nullptr;})


const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs0BW40MHz([](){ return new Ieee80211VHTMCS(0, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs1BW40MHz([](){ return new Ieee80211VHTMCS(1, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs2BW40MHz([](){ return new Ieee80211VHTMCS(2, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs3BW40MHz([](){ return new Ieee80211VHTMCS(3, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs4BW40MHz([](){ return new Ieee80211VHTMCS(4, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs5BW40MHz([](){ return new Ieee80211VHTMCS(5, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs6BW40MHz([](){ return new Ieee80211VHTMCS(6, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs7BW40MHz([](){ return new Ieee80211VHTMCS(7, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs8BW40MHz([](){ return new Ieee80211VHTMCS(8, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs9BW40MHz([](){ return new Ieee80211VHTMCS(9, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs10BW40MHz([](){ return new Ieee80211VHTMCS(10, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs11BW40MHz([](){ return new Ieee80211VHTMCS(11, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs12BW40MHz([](){ return new Ieee80211VHTMCS(12, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs13BW40MHz([](){ return new Ieee80211VHTMCS(13, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs14BW40MHz([](){ return new Ieee80211VHTMCS(14, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs15BW40MHz([](){ return new Ieee80211VHTMCS(15, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs16BW40MHz([](){ return new Ieee80211VHTMCS(16, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs17BW40MHz([](){ return new Ieee80211VHTMCS(17, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs18BW40MHz([](){ return new Ieee80211VHTMCS(18, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs19BW40MHz([](){ return new Ieee80211VHTMCS(19, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs20BW40MHz([](){ return new Ieee80211VHTMCS(20, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs21BW40MHz([](){ return new Ieee80211VHTMCS(21, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs22BW40MHz([](){ return new Ieee80211VHTMCS(22, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs23BW40MHz([](){ return new Ieee80211VHTMCS(23, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs24BW40MHz([](){ return new Ieee80211VHTMCS(24, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs25BW40MHz([](){ return new Ieee80211VHTMCS(25, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs26BW40MHz([](){ return new Ieee80211VHTMCS(26, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs27BW40MHz([](){ return new Ieee80211VHTMCS(27, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs28BW40MHz([](){ return new Ieee80211VHTMCS(28, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs29BW40MHz([](){ return new Ieee80211VHTMCS(29, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs30BW40MHz([](){ return new Ieee80211VHTMCS(30, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs31BW40MHz([](){ return new Ieee80211VHTMCS(31, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs32BW40MHz([](){ return new Ieee80211VHTMCS(32, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs33BW40MHz([](){ return new Ieee80211VHTMCS(33, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs34BW40MHz([](){ return new Ieee80211VHTMCS(34, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs35BW40MHz([](){ return new Ieee80211VHTMCS(35, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs36BW40MHz([](){ return new Ieee80211VHTMCS(36, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs37BW40MHz([](){ return new Ieee80211VHTMCS(37, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs38BW40MHz([](){ return new Ieee80211VHTMCS(38, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs39BW40MHz([](){ return new Ieee80211VHTMCS(39, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs40BW40MHz([](){ return new Ieee80211VHTMCS(40, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs41BW40MHz([](){ return new Ieee80211VHTMCS(41, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs42BW40MHz([](){ return new Ieee80211VHTMCS(42, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs43BW40MHz([](){ return new Ieee80211VHTMCS(43, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs44BW40MHz([](){ return new Ieee80211VHTMCS(44, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs45BW40MHz([](){ return new Ieee80211VHTMCS(45, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs46BW40MHz([](){ return new Ieee80211VHTMCS(46, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs47BW40MHz([](){ return new Ieee80211VHTMCS(47, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs48BW40MHz([](){ return new Ieee80211VHTMCS(48, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs49BW40MHz([](){ return new Ieee80211VHTMCS(49, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs50BW40MHz([](){ return new Ieee80211VHTMCS(50, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs51BW40MHz([](){ return new Ieee80211VHTMCS(51, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs52BW40MHz([](){ return new Ieee80211VHTMCS(52, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs53BW40MHz([](){ return new Ieee80211VHTMCS(53, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs54BW40MHz([](){ return new Ieee80211VHTMCS(54, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs55BW40MHz([](){ return new Ieee80211VHTMCS(55, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs56BW40MHz([](){ return new Ieee80211VHTMCS(56, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs57BW40MHz([](){ return new Ieee80211VHTMCS(57, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs58BW40MHz([](){ return new Ieee80211VHTMCS(58, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs59BW40MHz([](){ return new Ieee80211VHTMCS(59, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs60BW40MHz([](){ return new Ieee80211VHTMCS(60, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs61BW40MHz([](){ return new Ieee80211VHTMCS(61, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs62BW40MHz([](){ return new Ieee80211VHTMCS(62, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs63BW40MHz([](){ return new Ieee80211VHTMCS(63, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs64BW40MHz([](){ return new Ieee80211VHTMCS(64, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs65BW40MHz([](){ return new Ieee80211VHTMCS(65, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs66BW40MHz([](){ return new Ieee80211VHTMCS(66, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs67BW40MHz([](){ return new Ieee80211VHTMCS(67, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs68BW40MHz([](){ return new Ieee80211VHTMCS(68, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs69BW40MHz([](){ return new Ieee80211VHTMCS(69, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs70BW40MHz([](){ return new Ieee80211VHTMCS(70, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs71BW40MHz([](){ return new Ieee80211VHTMCS(71, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs72BW40MHz([](){ return new Ieee80211VHTMCS(72, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs73BW40MHz([](){ return new Ieee80211VHTMCS(73, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs74BW40MHz([](){ return new Ieee80211VHTMCS(74, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs75BW40MHz([](){ return new Ieee80211VHTMCS(75, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs76BW40MHz([](){ return new Ieee80211VHTMCS(76, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs77BW40MHz([](){ return new Ieee80211VHTMCS(77, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs78BW40MHz([](){ return new Ieee80211VHTMCS(78, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(40));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs79BW40MHz([](){ return new Ieee80211VHTMCS(89, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(40));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs0BW80MHz([](){ return new Ieee80211VHTMCS(0, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs1BW80MHz([](){ return new Ieee80211VHTMCS(1, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs2BW80MHz([](){ return new Ieee80211VHTMCS(2, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs3BW80MHz([](){ return new Ieee80211VHTMCS(3, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs4BW80MHz([](){ return new Ieee80211VHTMCS(4, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs5BW80MHz([](){ return new Ieee80211VHTMCS(5, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs6BW80MHz([](){ return new Ieee80211VHTMCS(6, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs7BW80MHz([](){ return new Ieee80211VHTMCS(7, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs8BW80MHz([](){ return new Ieee80211VHTMCS(8, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs9BW80MHz([](){ return new Ieee80211VHTMCS(9, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs10BW80MHz([](){ return new Ieee80211VHTMCS(10, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs11BW80MHz([](){ return new Ieee80211VHTMCS(11, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs12BW80MHz([](){ return new Ieee80211VHTMCS(12, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs13BW80MHz([](){ return new Ieee80211VHTMCS(13, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs14BW80MHz([](){ return new Ieee80211VHTMCS(14, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs15BW80MHz([](){ return new Ieee80211VHTMCS(15, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs16BW80MHz([](){ return new Ieee80211VHTMCS(16, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs17BW80MHz([](){ return new Ieee80211VHTMCS(17, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs18BW80MHz([](){ return new Ieee80211VHTMCS(18, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs19BW80MHz([](){ return new Ieee80211VHTMCS(19, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs20BW80MHz([](){ return new Ieee80211VHTMCS(20, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs21BW80MHz([](){ return new Ieee80211VHTMCS(21, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs22BW80MHz([](){ return new Ieee80211VHTMCS(22, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs23BW80MHz([](){ return new Ieee80211VHTMCS(23, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs24BW80MHz([](){ return new Ieee80211VHTMCS(24, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs25BW80MHz([](){ return new Ieee80211VHTMCS(25, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs26BW80MHz([](){ return nullptr;});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs27BW80MHz([](){ return new Ieee80211VHTMCS(27, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs28BW80MHz([](){ return new Ieee80211VHTMCS(28, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs29BW80MHz([](){ return new Ieee80211VHTMCS(29, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs30BW80MHz([](){ return new Ieee80211VHTMCS(30, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs31BW80MHz([](){ return new Ieee80211VHTMCS(31, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs32BW80MHz([](){ return new Ieee80211VHTMCS(32, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs33BW80MHz([](){ return new Ieee80211VHTMCS(33, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs34BW80MHz([](){ return new Ieee80211VHTMCS(34, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs35BW80MHz([](){ return new Ieee80211VHTMCS(35, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs36BW80MHz([](){ return new Ieee80211VHTMCS(36, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs37BW80MHz([](){ return new Ieee80211VHTMCS(37, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs38BW80MHz([](){ return new Ieee80211VHTMCS(38, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs39BW80MHz([](){ return new Ieee80211VHTMCS(39, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs40BW80MHz([](){ return new Ieee80211VHTMCS(40, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs41BW80MHz([](){ return new Ieee80211VHTMCS(41, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs42BW80MHz([](){ return new Ieee80211VHTMCS(42, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs43BW80MHz([](){ return new Ieee80211VHTMCS(43, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs44BW80MHz([](){ return new Ieee80211VHTMCS(44, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs45BW80MHz([](){ return new Ieee80211VHTMCS(45, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs46BW80MHz([](){ return new Ieee80211VHTMCS(46, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs47BW80MHz([](){ return new Ieee80211VHTMCS(47, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs48BW80MHz([](){ return new Ieee80211VHTMCS(48, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs49BW80MHz([](){ return new Ieee80211VHTMCS(49, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs50BW80MHz([](){ return new Ieee80211VHTMCS(50, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs51BW80MHz([](){ return new Ieee80211VHTMCS(51, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs52BW80MHz([](){ return new Ieee80211VHTMCS(52, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs53BW80MHz([](){ return new Ieee80211VHTMCS(53, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs54BW80MHz([](){ return new Ieee80211VHTMCS(54, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs55BW80MHz([](){ return new Ieee80211VHTMCS(55, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs56BW80MHz([](){ return new Ieee80211VHTMCS(56, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs57BW80MHz([](){ return new Ieee80211VHTMCS(57, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs58BW80MHz([](){ return new Ieee80211VHTMCS(58, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs59BW80MHz([](){ return new Ieee80211VHTMCS(59, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs60BW80MHz([](){ return new Ieee80211VHTMCS(60, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs61BW80MHz([](){ return new Ieee80211VHTMCS(61, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs62BW80MHz([](){ return new Ieee80211VHTMCS(62, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs63BW80MHz([](){ return new Ieee80211VHTMCS(63, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs64BW80MHz([](){ return new Ieee80211VHTMCS(64, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs65BW80MHz([](){ return new Ieee80211VHTMCS(65, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs66BW80MHz([](){ return nullptr;});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs67BW80MHz([](){ return new Ieee80211VHTMCS(67, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs68BW80MHz([](){ return new Ieee80211VHTMCS(68, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs69BW80MHz([](){ return new Ieee80211VHTMCS(69, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs70BW80MHz([](){ return new Ieee80211VHTMCS(70, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs71BW80MHz([](){ return new Ieee80211VHTMCS(71, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs72BW80MHz([](){ return new Ieee80211VHTMCS(72, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs73BW80MHz([](){ return new Ieee80211VHTMCS(73, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs74BW80MHz([](){ return new Ieee80211VHTMCS(74, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs75BW80MHz([](){ return new Ieee80211VHTMCS(75, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs76BW80MHz([](){ return new Ieee80211VHTMCS(76, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs77BW80MHz([](){ return new Ieee80211VHTMCS(77, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs78BW80MHz([](){ return new Ieee80211VHTMCS(78, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(80));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs79BW80MHz([](){ return new Ieee80211VHTMCS(89, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(80));});


const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs0BW160MHz([](){ return new Ieee80211VHTMCS(0, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs1BW160MHz([](){ return new Ieee80211VHTMCS(1, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs2BW160MHz([](){ return new Ieee80211VHTMCS(2, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs3BW160MHz([](){ return new Ieee80211VHTMCS(3, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs4BW160MHz([](){ return new Ieee80211VHTMCS(4, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs5BW160MHz([](){ return new Ieee80211VHTMCS(5, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs6BW160MHz([](){ return new Ieee80211VHTMCS(6, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs7BW160MHz([](){ return new Ieee80211VHTMCS(7, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs8BW160MHz([](){ return new Ieee80211VHTMCS(8, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs9BW160MHz([](){ return new Ieee80211VHTMCS(9, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs10BW160MHz([](){ return new Ieee80211VHTMCS(10, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs11BW160MHz([](){ return new Ieee80211VHTMCS(11, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs12BW160MHz([](){ return new Ieee80211VHTMCS(12, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs13BW160MHz([](){ return new Ieee80211VHTMCS(13, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs14BW160MHz([](){ return new Ieee80211VHTMCS(14, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs15BW160MHz([](){ return new Ieee80211VHTMCS(15, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs16BW160MHz([](){ return new Ieee80211VHTMCS(16, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs17BW160MHz([](){ return new Ieee80211VHTMCS(17, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs18BW160MHz([](){ return new Ieee80211VHTMCS(18, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs19BW160MHz([](){ return new Ieee80211VHTMCS(19, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs20BW160MHz([](){ return new Ieee80211VHTMCS(20, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs21BW160MHz([](){ return new Ieee80211VHTMCS(21, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs22BW160MHz([](){ return new Ieee80211VHTMCS(22, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs23BW160MHz([](){ return new Ieee80211VHTMCS(23, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs24BW160MHz([](){ return new Ieee80211VHTMCS(24, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs25BW160MHz([](){ return new Ieee80211VHTMCS(25, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs26BW160MHz([](){ return new Ieee80211VHTMCS(26, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs27BW160MHz([](){ return new Ieee80211VHTMCS(27, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs28BW160MHz([](){ return new Ieee80211VHTMCS(28, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs29BW160MHz([](){ return nullptr;});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs30BW160MHz([](){ return new Ieee80211VHTMCS(30, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs31BW160MHz([](){ return new Ieee80211VHTMCS(31, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs32BW160MHz([](){ return new Ieee80211VHTMCS(32, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs33BW160MHz([](){ return new Ieee80211VHTMCS(33, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs34BW160MHz([](){ return new Ieee80211VHTMCS(34, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs35BW160MHz([](){ return new Ieee80211VHTMCS(35, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs36BW160MHz([](){ return new Ieee80211VHTMCS(36, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs37BW160MHz([](){ return new Ieee80211VHTMCS(37, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs38BW160MHz([](){ return new Ieee80211VHTMCS(38, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs39BW160MHz([](){ return new Ieee80211VHTMCS(39, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs40BW160MHz([](){ return new Ieee80211VHTMCS(40, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs41BW160MHz([](){ return new Ieee80211VHTMCS(41, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs42BW160MHz([](){ return new Ieee80211VHTMCS(42, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs43BW160MHz([](){ return new Ieee80211VHTMCS(43, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs44BW160MHz([](){ return new Ieee80211VHTMCS(44, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs45BW160MHz([](){ return new Ieee80211VHTMCS(45, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs46BW160MHz([](){ return new Ieee80211VHTMCS(46, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs47BW160MHz([](){ return new Ieee80211VHTMCS(47, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs48BW160MHz([](){ return new Ieee80211VHTMCS(48, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs49BW160MHz([](){ return new Ieee80211VHTMCS(49, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs50BW160MHz([](){ return new Ieee80211VHTMCS(50, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs51BW160MHz([](){ return new Ieee80211VHTMCS(51, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs52BW160MHz([](){ return new Ieee80211VHTMCS(52, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs53BW160MHz([](){ return new Ieee80211VHTMCS(53, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs54BW160MHz([](){ return new Ieee80211VHTMCS(54, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs55BW160MHz([](){ return new Ieee80211VHTMCS(55, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs56BW160MHz([](){ return new Ieee80211VHTMCS(56, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs57BW160MHz([](){ return new Ieee80211VHTMCS(57, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs58BW160MHz([](){ return new Ieee80211VHTMCS(58, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs59BW160MHz([](){ return new Ieee80211VHTMCS(59, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs60BW160MHz([](){ return new Ieee80211VHTMCS(60, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs61BW160MHz([](){ return new Ieee80211VHTMCS(61, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs62BW160MHz([](){ return new Ieee80211VHTMCS(62, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs63BW160MHz([](){ return new Ieee80211VHTMCS(63, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs64BW160MHz([](){ return new Ieee80211VHTMCS(64, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs65BW160MHz([](){ return new Ieee80211VHTMCS(65, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs66BW160MHz([](){ return new Ieee80211VHTMCS(66, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs67BW160MHz([](){ return new Ieee80211VHTMCS(67, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs68BW160MHz([](){ return new Ieee80211VHTMCS(68, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs69BW160MHz([](){ return new Ieee80211VHTMCS(69, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});

const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs70BW160MHz([](){ return new Ieee80211VHTMCS(70, &Ieee80211OFDMCompliantModulations::bpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs71BW160MHz([](){ return new Ieee80211VHTMCS(71, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs72BW160MHz([](){ return new Ieee80211VHTMCS(72, &Ieee80211OFDMCompliantModulations::qpskModulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs73BW160MHz([](){ return new Ieee80211VHTMCS(73, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode1_2, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs74BW160MHz([](){ return new Ieee80211VHTMCS(74, &Ieee80211OFDMCompliantModulations::qam16Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs75BW160MHz([](){ return new Ieee80211VHTMCS(75, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode2_3, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs76BW160MHz([](){ return new Ieee80211VHTMCS(76, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs77BW160MHz([](){ return new Ieee80211VHTMCS(77, &Ieee80211OFDMCompliantModulations::qam64Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs78BW160MHz([](){ return new Ieee80211VHTMCS(78, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode3_4, MHz(160));});
const DI<Ieee80211VHTMCS> Ieee80211VHTMCSTable::vhtMcs79BW160MHz([](){ return new Ieee80211VHTMCS(89, &Ieee80211OFDMCompliantModulations::qam256Modulation, &Ieee80211OFDMCompliantCodes::ofdmConvolutionalCode5_6, MHz(160));});

} /* namespace physicallayer */
} /* namespace inet */
