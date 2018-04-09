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

#ifndef __INET_IEEE80211VHTMODE_H
#define __INET_IEEE80211VHTMODE_H

#define DI DelayedInitializer

#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211VHTCode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTMode.h"
#include "inet/common/DelayedInitializer.h"

namespace inet {
namespace physicallayer {


class INET_API Ieee80211VHTModeBase
{
    public:
        enum GuardIntervalType
        {
            HT_GUARD_INTERVAL_SHORT, // 400 ns
            HT_GUARD_INTERVAL_LONG // 800 ns
        };

    protected:
        const Hz bandwidth;
        const GuardIntervalType guardIntervalType;
        const unsigned int mcsIndex; // MCS
        const unsigned int numberOfSpatialStreams; // N_SS

        mutable bps netBitrate; // cached
        mutable bps grossBitrate; // cached

    protected:
        virtual bps computeGrossBitrate() const = 0;
        virtual bps computeNetBitrate() const = 0;

    public:
        Ieee80211VHTModeBase(unsigned int modulationAndCodingScheme, unsigned int numberOfSpatialStreams, const Hz bandwidth, GuardIntervalType guardIntervalType);

        virtual int getNumberOfDataSubcarriers() const;
        virtual int getNumberOfPilotSubcarriers() const;
        virtual int getNumberOfTotalSubcarriers() const { return getNumberOfDataSubcarriers() + getNumberOfPilotSubcarriers(); }
        virtual GuardIntervalType getGuardIntervalType() const { return guardIntervalType; }
        virtual int getNumberOfSpatialStreams() const { return numberOfSpatialStreams; }
        virtual unsigned int getMcsIndex() const { return mcsIndex; }
        virtual Hz getBandwidth() const { return bandwidth; }
        virtual bps getNetBitrate() const;
        virtual bps getGrossBitrate() const;
};


class INET_API Ieee80211VHTSignalMode : public IIeee80211HeaderMode, public Ieee80211VHTModeBase, public Ieee80211HTTimingRelatedParametersBase
{
    protected:
        const Ieee80211OFDMModulation *modulation;
        const Ieee80211VHTCode *code;

    protected:
        virtual bps computeGrossBitrate() const override;
        virtual bps computeNetBitrate() const override;

    public:
        Ieee80211VHTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation *modulation, const Ieee80211VHTCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType);
        Ieee80211VHTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation *modulation, const Ieee80211ConvolutionalCode *convolutionalCode, const Hz bandwidth, GuardIntervalType guardIntervalType);
        virtual ~Ieee80211VHTSignalMode();

        /* Table 20-11—HT-SIG fields, 1699p */

        // HT-SIG_1 (24 bits)
        virtual inline int getMCSLength() const { return 9; }
        virtual inline int getCBWLength() const { return 1; }
        virtual inline int getHTLengthLength() const { return 16; }

        // HT-SIG_2 (24 bits)
        virtual inline int getSmoothingLength() const { return 1; }
        virtual inline int getNotSoundingLength() const { return 1; }
        virtual inline int getReservedLength() const { return 1; }
        virtual inline int getAggregationLength() const { return 1; }
        virtual inline int getSTBCLength() const { return 2; }
        virtual inline int getFECCodingLength() const { return 1; }
        virtual inline int getShortGILength() const { return 1; }
        virtual inline int getNumOfExtensionSpatialStreamsLength() const { return 2; }
        virtual inline int getCRCLength() const { return 8; }
        virtual inline int getTailBitsLength() const { return 6; }
        virtual unsigned int getSTBC() const { return 0; } // Limitation: We assume that STBC is not used

        virtual const inline simtime_t getHTSIGDuration() const { return 2 * getSymbolInterval(); } // HT-SIG

        virtual unsigned int getModulationAndCodingScheme() const { return mcsIndex; }
        virtual const simtime_t getDuration() const override { return getHTSIGDuration(); }
        virtual int getBitLength() const override;
        virtual bps getNetBitrate() const override { return Ieee80211VHTModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211VHTModeBase::getGrossBitrate(); }
        virtual const Ieee80211OFDMModulation *getModulation() const override { return modulation; }
        virtual const Ieee80211VHTCode * getCode() const {return code;}
};

/*
 * The HT preambles are defined in HT-mixed format and in HT-greenfield format to carry the required
 * information to operate in a system with multiple transmit and multiple receive antennas. (20.3.9 HT preamble)
 */
class INET_API Ieee80211VHTPreambleMode : public IIeee80211PreambleMode, public Ieee80211HTTimingRelatedParametersBase
{
    public:
        enum HighTroughputPreambleFormat
        {
            HT_PREAMBLE_MIXED,      // can be received by non-HT STAs compliant with Clause 18 or Clause 19
            HT_PREAMBLE_GREENFIELD  // all of the non-HT fields are omitted
        };

    protected:
        const Ieee80211VHTSignalMode *highThroughputSignalMode; // In HT-terminology the HT-SIG (signal field) and L-SIG are part of the preamble
        const Ieee80211OFDMSignalMode *legacySignalMode; // L-SIG
        const HighTroughputPreambleFormat preambleFormat;
        const unsigned int numberOfHTLongTrainings; // N_LTF, 20.3.9.4.6 HT-LTF definition

    protected:
        virtual unsigned int computeNumberOfSpaceTimeStreams(unsigned int numberOfSpatialStreams) const;
        virtual unsigned int computeNumberOfHTLongTrainings(unsigned int numberOfSpaceTimeStreams) const;

    public:
        Ieee80211VHTPreambleMode(const Ieee80211VHTSignalMode* highThroughputSignalMode, const Ieee80211OFDMSignalMode *legacySignalMode, HighTroughputPreambleFormat preambleFormat, unsigned int numberOfSpatialStream);
        virtual ~Ieee80211VHTPreambleMode() { delete highThroughputSignalMode; }

        HighTroughputPreambleFormat getPreambleFormat() const { return preambleFormat; }
        virtual const Ieee80211VHTSignalMode *getSignalMode() const { return highThroughputSignalMode; }
        virtual const Ieee80211OFDMSignalMode *getLegacySignalMode() const { return legacySignalMode; }
        virtual const Ieee80211VHTSignalMode* getHighThroughputSignalMode() const { return highThroughputSignalMode; }
        virtual inline unsigned int getNumberOfHTLongTrainings() const { return numberOfHTLongTrainings; }

        virtual const inline simtime_t getDoubleGIDuration() const { return 2 * getGIDuration(); } // GI2
        virtual const inline simtime_t getLSIGDuration() const { return getSymbolInterval(); } // L-SIG
        virtual const inline simtime_t getNonHTShortTrainingSequenceDuration() const { return 10 * getDFTPeriod() / 4;  } // L-STF
        virtual const inline simtime_t getHTGreenfieldShortTrainingFieldDuration() const { return 10 * getDFTPeriod() / 4; } // HT-GF-STF
        virtual const inline simtime_t getNonHTLongTrainingFieldDuration() const { return 2 * getDFTPeriod() + getDoubleGIDuration(); } // L-LTF
        virtual const inline simtime_t getHTShortTrainingFieldDuration() const { return 4E-6; } // HT-STF
        virtual const simtime_t getFirstHTLongTrainingFieldDuration() const;
        virtual const inline simtime_t getSecondAndSubsequentHTLongTrainingFielDuration() const { return 4E-6; } // HT-LTFs, s = 2,3,..,n
        virtual const inline unsigned int getNumberOfHtLongTrainings() const { return numberOfHTLongTrainings; }

        virtual const simtime_t getDuration() const override;

};


class INET_API Ieee80211VHTMCS
{
    protected:
        const unsigned int mcsIndex;
        const Ieee80211OFDMModulation *stream1Modulation = nullptr;
        const Ieee80211OFDMModulation *stream2Modulation = nullptr;
        const Ieee80211OFDMModulation *stream3Modulation = nullptr;
        const Ieee80211OFDMModulation *stream4Modulation = nullptr;
        const Ieee80211OFDMModulation *stream5Modulation = nullptr;
        const Ieee80211OFDMModulation *stream6Modulation = nullptr;
        const Ieee80211OFDMModulation *stream7Modulation = nullptr;
        const Ieee80211OFDMModulation *stream8Modulation = nullptr;
        const Ieee80211VHTCode *code;
        const Hz bandwidth;

    public:
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211VHTCode *code, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211OFDMModulation *stream4Modulation, const Ieee80211OFDMModulation *stream5Modulation, const Ieee80211OFDMModulation *stream6Modulation, const Ieee80211OFDMModulation *stream7Modulation, const Ieee80211OFDMModulation *stream8Modulation);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Ieee80211OFDMModulation* stream5Modulation, const Ieee80211OFDMModulation* stream6Modulation, const Ieee80211OFDMModulation* stream7Modulation, const Ieee80211OFDMModulation* stream8Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Ieee80211OFDMModulation* stream5Modulation, const Ieee80211OFDMModulation* stream6Modulation, const Ieee80211OFDMModulation* stream7Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Ieee80211OFDMModulation* stream5Modulation, const Ieee80211OFDMModulation* stream6Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Ieee80211OFDMModulation* stream5Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation, const Ieee80211ConvolutionalCode* convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth, int nss);

        virtual ~Ieee80211VHTMCS();

        const Ieee80211VHTCode* getCode() const { return code; }
        unsigned int getMcsIndex() const { return mcsIndex; }
        virtual const Ieee80211OFDMModulation* getModulation() const { return stream1Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension1Modulation() const { return stream2Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension2Modulation() const { return stream3Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension3Modulation() const { return stream4Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension4Modulation() const { return stream5Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension5Modulation() const { return stream6Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension6Modulation() const { return stream7Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension7Modulation() const { return stream8Modulation; }
        virtual Hz getBandwidth() const { return bandwidth; }
};

class INET_API Ieee80211VHTDataMode : public IIeee80211DataMode, public Ieee80211VHTModeBase, public Ieee80211HTTimingRelatedParametersBase
{
    protected:
        const Ieee80211VHTMCS *modulationAndCodingScheme;
        const unsigned int numberOfBccEncoders;

    protected:
        bps computeGrossBitrate() const override;
        bps computeNetBitrate() const override;
        unsigned int computeNumberOfSpatialStreams(const Ieee80211VHTMCS*) const;
        unsigned int computeNumberOfCodedBitsPerSubcarrierSum() const;
        unsigned int computeNumberOfBccEncoders() const;
        unsigned int getNumberOfBccEncoders20MHz() const;
        unsigned int getNumberOfBccEncoders40MHz() const;
        unsigned int getNumberOfBccEncoders80MHz() const;
        unsigned int getNumberOfBccEncoders160MHz() const;

    public:
        Ieee80211VHTDataMode(const Ieee80211VHTMCS *modulationAndCodingScheme, const Hz bandwidth, GuardIntervalType guardIntervalType);

        inline int getServiceBitLength() const { return 16; }
        inline int getTailBitLength() const { return 6 * numberOfBccEncoders; }

        virtual int getNumberOfSpatialStreams() const override { return Ieee80211VHTModeBase::getNumberOfSpatialStreams(); }
        virtual int getBitLength(int dataBitLength) const override;
        virtual const simtime_t getDuration(int dataBitLength) const override;
        virtual bps getNetBitrate() const override { return Ieee80211VHTModeBase::getNetBitrate(); }
        virtual bps getGrossBitrate() const override { return Ieee80211VHTModeBase::getGrossBitrate(); }
        virtual const Ieee80211VHTMCS *getModulationAndCodingScheme() const { return modulationAndCodingScheme; }
        virtual const Ieee80211VHTCode* getCode() const { return modulationAndCodingScheme->getCode(); }
        virtual const Ieee80211OFDMModulation* getModulation() const override { return modulationAndCodingScheme->getModulation(); }
};

class INET_API Ieee80211VHTMode : public Ieee80211ModeBase
{
    public:
        enum BandMode
        {
            BAND_2_4GHZ,
            BAND_5GHZ
        };

    protected:
        const Ieee80211VHTPreambleMode *preambleMode;
        const Ieee80211VHTDataMode *dataMode;
        const BandMode carrierFrequencyMode;

    protected:
        virtual inline int getLegacyCwMin() const override { return 15; }
        virtual inline int getLegacyCwMax() const override { return 1023; }

    public:
        Ieee80211VHTMode(const char *name, const Ieee80211VHTPreambleMode *preambleMode, const Ieee80211VHTDataMode *dataMode, const BandMode carrierFrequencyMode);
        virtual ~Ieee80211VHTMode() { delete preambleMode; delete dataMode; }

        virtual const Ieee80211VHTDataMode* getDataMode() const override { return dataMode; }
        virtual const Ieee80211VHTPreambleMode* getPreambleMode() const override { return preambleMode; }
        virtual const Ieee80211VHTSignalMode *getHeaderMode() const override { return preambleMode->getSignalMode(); }
        virtual const Ieee80211OFDMSignalMode *getLegacySignalMode() const { return preambleMode->getLegacySignalMode(); }

        // Table 20-25—MIMO PHY characteristics
        virtual const simtime_t getSlotTime() const override;
        virtual const simtime_t getShortSlotTime() const;
        virtual inline const simtime_t getSifsTime() const override;
        virtual inline const simtime_t getRifsTime() const override { return 2E-6; }
        virtual inline const simtime_t getCcaTime() const override { return 4E-6; } // < 4
        virtual inline const simtime_t getPhyRxStartDelay() const override { return 33E-6; }
        virtual inline const simtime_t getRxTxTurnaroundTime() const override { return 2E-6; } // < 2
        virtual inline const simtime_t getPreambleLength() const override { return 16E-6; }
        virtual inline const simtime_t getPlcpHeaderLength() const override { return 4E-6; }
        virtual inline int getMpduMaxLength() const override { return 65535; } // in octets
        virtual BandMode getCarrierFrequencyMode() const { return carrierFrequencyMode; }

        virtual const simtime_t getDuration(int dataBitLength) const override { return preambleMode->getDuration() + dataMode->getDuration(dataBitLength); }
};

// A specification of the high-throughput (HT) physical layer (PHY)
// parameters that consists of modulation order (e.g., BPSK, QPSK, 16-QAM,
// 64-QAM) and forward error correction (FEC) coding rate (e.g., 1/2, 2/3,
// 3/4, 5/6).
class INET_API Ieee80211VHTMCSTable
{
    public:
        // Table 20-30—MCS parameters for mandatory 20 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss1; // No valid

        // Table 20-31—MCS parameters for optional 20 MHz, N_SS = 2
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss2; // No valid

        // Table 20-32—MCS parameters for optional 20 MHz, N_SS = 3
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss3;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 4
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss4; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 5
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss5; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 6
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss6;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 7
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss7; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 8
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHzNss8; // No valid

        // Table 20-30—MCS parameters for mandatory 40 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss1;

        // Table 20-31—MCS parameters for optional 40 MHz, N_SS = 2
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss2;

        // Table 20-32—MCS parameters for optional 40 MHz, N_SS = 3
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss3;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 4
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss4;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 5
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss5;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 6
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss6;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 7
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss7;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 8
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHzNss8;
        // Table 20-30—MCS parameters for mandatory 80 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss1;

        // Table 20-31—MCS parameters for optional 80 MHz, N_SS = 2
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss2;

        // Table 20-32—MCS parameters for optional 80 MHz, N_SS = 3
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss3;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 4
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss4;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 5
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss5;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 6
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss6;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 7
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss7;

        // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 8
        static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHzNss8;

        // Table 20-30—MCS parameters for mandatory 160 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss1;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss1;

        // Table 20-31—MCS parameters for optional 160 MHz, N_SS = 2
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss2;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss2;

        // Table 20-32—MCS parameters for optional 160 MHz, N_SS = 3
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss3;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss3;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 4
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss4;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss4;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 5
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss5;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss5;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 6
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss6;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss6;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 7
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss7;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss7;

        // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 8
        static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHzNss8;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHzNss8;


};

class INET_API Ieee80211VHTCompliantModes
{
    protected:
        static Ieee80211VHTCompliantModes singleton;

        std::map<std::tuple<Hz, unsigned int, Ieee80211VHTModeBase::GuardIntervalType>, const Ieee80211VHTMode *> modeCache;

    public:
        Ieee80211VHTCompliantModes();
        virtual ~Ieee80211VHTCompliantModes();

        static const Ieee80211VHTMode *getCompliantMode(const Ieee80211VHTMCS *mcsMode, Ieee80211VHTMode::BandMode carrierFrequencyMode, Ieee80211VHTPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211VHTModeBase::GuardIntervalType guardIntervalType);

};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211VHTMODE_H
