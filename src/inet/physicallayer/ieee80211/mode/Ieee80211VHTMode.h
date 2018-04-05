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


class INET_API Ieee80211VHTSignalMode : public IIeee80211HeaderMode, public Ieee80211HTModeBase, public Ieee80211HTTimingRelatedParametersBase
{
    protected:
        const Ieee80211OFDMModulation *modulation;
        const Ieee80211VHTCode *code;

    protected:
        virtual bps computeGrossBitrate() const override;
        virtual bps computeNetBitrate() const override;

    public:
        Ieee80211VHTSignalMode(unsigned int modulationAndCodingScheme, const Ieee80211OFDMModulation *modulation, const Ieee80211HTCode *code, const Hz bandwidth, GuardIntervalType guardIntervalType);
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
        virtual const IModulation *getModulation() const override { return modulation; }
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
        virtual const Ieee80211HTSignalMode *getSignalMode() const { return highThroughputSignalMode; }
        virtual const Ieee80211OFDMSignalMode *getLegacySignalMode() const { return legacySignalMode; }
        virtual const Ieee80211HTSignalMode* getHighThroughputSignalMode() const { return highThroughputSignalMode; }
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
        const Ieee80211OFDMModulation *stream1Modulation;
        const Ieee80211OFDMModulation *stream2Modulation = nullptr;
        const Ieee80211OFDMModulation *stream3Modulation = nullptr;
        const Ieee80211OFDMModulation *stream4Modulation = nullptr;
        const Ieee80211HTCode *code;
        const Hz bandwidth;

    public:
        Ieee80211VHTMCS(unsigned int mcsIndex, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211ConvolutionalCode *convolutionalCode, Hz bandwidth);
        virtual ~Ieee80211VHTMCS();

        const Ieee80211VHTCode* getCode() const { return code; }
        unsigned int getMcsIndex() const { return mcsIndex; }
        virtual const Ieee80211OFDMModulation* getModulation() const { return stream1Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension1Modulation() const { return stream2Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension2Modulation() const { return stream3Modulation; }
        virtual const Ieee80211OFDMModulation* getStreamExtension3Modulation() const { return stream4Modulation; }
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
        unsigned int computeNumberOfSpatialStreams(const Ieee80211OFDMModulation* stream1Modulation, const Ieee80211OFDMModulation* stream2Modulation, const Ieee80211OFDMModulation* stream3Modulation, const Ieee80211OFDMModulation* stream4Modulation) const;
        unsigned int computeNumberOfCodedBitsPerSubcarrierSum() const;
        unsigned int computeNumberOfBccEncoders() const;

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
        static const DI<Ieee80211VHTMCS> vhtMcs0BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW20MHz; // No valid

        // Table 20-31—MCS parameters for optional 20 MHz, N_SS = 2
        static const DI<Ieee80211VHTMCS> vhtMcs10BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs11BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs12BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs13BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs14BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs15BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs16BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs17BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs18BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs19BW20MHz; // No valid

        // Table 20-32—MCS parameters for optional 20 MHz, N_SS = 3
        static const DI<Ieee80211VHTMCS> vhtMcs20BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs21BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs22BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs23BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs24BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs25BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs26BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs27BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs28BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs29BW20MHz;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 4
        static const DI<Ieee80211VHTMCS> vhtMcs30BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs31BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs32BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs33BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs34BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs35BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs36BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs37BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs38BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs39BW20MHz; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 5
        static const DI<Ieee80211VHTMCS> vhtMcs40BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs41BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs42BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs43BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs44BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs45BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs46BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs47BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs48BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs49BW20MHz; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 6
        static const DI<Ieee80211VHTMCS> vhtMcs50BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs51BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs52BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs53BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs54BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs55BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs56BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs57BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs58BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs59BW20MHz;

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 7
        static const DI<Ieee80211VHTMCS> vhtMcs60BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs61BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs62BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs63BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs64BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs65BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs66BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs67BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs68BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs69BW20MHz; // No valid

        // Table 20-33—MCS parameters for optional 20 MHz, N_SS = 8
        static const DI<Ieee80211VHTMCS> vhtMcs70BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs71BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs72BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs73BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs74BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs75BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs76BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs77BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs78BW20MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs79BW20MHz; // No valid

        // Table 20-30—MCS parameters for mandatory 40 MHz, N_SS = 1, N_ES = 1
        static const DI<Ieee80211VHTMCS> vhtMcs0BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs1BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs2BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs3BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs4BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs5BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs6BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs7BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs8BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs9BW40MHz; // no valid

        // Table 20-31—MCS parameters for optional 40 MHz, N_SS = 2
        static const DI<Ieee80211VHTMCS> vhtMcs10BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs11BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs12BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs13BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs14BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs15BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs16BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs17BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs18BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs19BW40MHz;

        // Table 20-32—MCS parameters for optional 40 MHz, N_SS = 3
        static const DI<Ieee80211VHTMCS> vhtMcs20BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs21BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs22BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs23BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs24BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs25BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs26BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs27BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs28BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs29BW40MHz;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 4
        static const DI<Ieee80211VHTMCS> vhtMcs30BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs31BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs32BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs33BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs34BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs35BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs36BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs37BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs38BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs39BW40MHz;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 5
        static const DI<Ieee80211VHTMCS> vhtMcs40BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs41BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs42BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs43BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs44BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs45BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs46BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs47BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs48BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs49BW40MHz;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 6
        static const DI<Ieee80211VHTMCS> vhtMcs50BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs51BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs52BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs53BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs54BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs55BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs56BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs57BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs58BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs59BW40MHz;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 7
        static const DI<Ieee80211VHTMCS> vhtMcs60BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs61BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs62BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs63BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs64BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs65BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs66BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs67BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs68BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs69BW40MHz;

        // Table 20-33—MCS parameters for optional 40 MHz, N_SS = 8
        static const DI<Ieee80211VHTMCS> vhtMcs70BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs71BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs72BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs73BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs74BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs75BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs76BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs77BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs78BW40MHz;
        static const DI<Ieee80211VHTMCS> vhtMcs79BW40MHz;

        // Table 20-30—MCS parameters for mandatory 80 MHz, N_SS = 1, N_ES = 1
          static const DI<Ieee80211VHTMCS> vhtMcs0BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs1BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs2BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs3BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs4BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs5BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs6BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs7BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs8BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs9BW80MHz;

          // Table 20-31—MCS parameters for optional 80 MHz, N_SS = 2
          static const DI<Ieee80211VHTMCS> vhtMcs10BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs11BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs12BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs13BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs14BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs15BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs16BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs17BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs18BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs19BW80MHz;

          // Table 20-32—MCS parameters for optional 80 MHz, N_SS = 3
          static const DI<Ieee80211VHTMCS> vhtMcs20BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs21BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs22BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs23BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs24BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs25BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs26BW80MHz; // no valid
          static const DI<Ieee80211VHTMCS> vhtMcs27BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs28BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs29BW80MHz;

          // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 4
          static const DI<Ieee80211VHTMCS> vhtMcs30BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs31BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs32BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs33BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs34BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs35BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs36BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs37BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs38BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs39BW80MHz;

          // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 5
          static const DI<Ieee80211VHTMCS> vhtMcs40BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs41BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs42BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs43BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs44BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs45BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs46BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs47BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs48BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs49BW80MHz;

          // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 6
          static const DI<Ieee80211VHTMCS> vhtMcs50BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs51BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs52BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs53BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs54BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs55BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs56BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs57BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs58BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs59BW80MHz;

          // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 7
          static const DI<Ieee80211VHTMCS> vhtMcs60BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs61BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs62BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs63BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs64BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs65BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs66BW80MHz; // no valid
          static const DI<Ieee80211VHTMCS> vhtMcs67BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs68BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs69BW80MHz;

          // Table 20-33—MCS parameters for optional 80 MHz, N_SS = 8
          static const DI<Ieee80211VHTMCS> vhtMcs70BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs71BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs72BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs73BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs74BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs75BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs76BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs77BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs78BW80MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs79BW80MHz;

          // Table 20-30—MCS parameters for mandatory 160 MHz, N_SS = 1, N_ES = 1
          static const DI<Ieee80211VHTMCS> vhtMcs0BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs1BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs2BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs3BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs4BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs5BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs6BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs7BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs8BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs9BW160MHz;

          // Table 20-31—MCS parameters for optional 160 MHz, N_SS = 2
          static const DI<Ieee80211VHTMCS> vhtMcs10BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs11BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs12BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs13BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs14BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs15BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs16BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs17BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs18BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs19BW160MHz;

          // Table 20-32—MCS parameters for optional 160 MHz, N_SS = 3
          static const DI<Ieee80211VHTMCS> vhtMcs20BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs21BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs22BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs23BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs24BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs25BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs26BW160MHz; // no valid
          static const DI<Ieee80211VHTMCS> vhtMcs27BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs28BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs29BW160MHz;

          // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 4
          static const DI<Ieee80211VHTMCS> vhtMcs30BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs31BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs32BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs33BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs34BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs35BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs36BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs37BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs38BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs39BW160MHz;

          // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 5
          static const DI<Ieee80211VHTMCS> vhtMcs40BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs41BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs42BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs43BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs44BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs45BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs46BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs47BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs48BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs49BW160MHz;

          // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 6
          static const DI<Ieee80211VHTMCS> vhtMcs50BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs51BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs52BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs53BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs54BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs55BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs56BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs57BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs58BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs59BW160MHz;

          // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 7
          static const DI<Ieee80211VHTMCS> vhtMcs60BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs61BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs62BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs63BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs64BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs65BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs66BW160MHz; // no valid
          static const DI<Ieee80211VHTMCS> vhtMcs67BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs68BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs69BW160MHz;

          // Table 20-33—MCS parameters for optional 160 MHz, N_SS = 8
          static const DI<Ieee80211VHTMCS> vhtMcs70BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs71BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs72BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs73BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs74BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs75BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs76BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs77BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs78BW160MHz;
          static const DI<Ieee80211VHTMCS> vhtMcs79BW160MHz;
};

class INET_API Ieee80211VHTCompliantModes
{
    protected:
        static Ieee80211VHTCompliantModes singleton;

        std::map<std::tuple<Hz, unsigned int, Ieee80211VHTModeBase::GuardIntervalType>, const Ieee80211HTMode *> modeCache;

    public:
        Ieee80211VHTCompliantModes();
        virtual ~Ieee80211VHTCompliantModes();

        static const Ieee80211VHTMode *getCompliantMode(const Ieee80211VHTMCS *mcsMode, Ieee80211HTMode::BandMode carrierFrequencyMode, Ieee80211HTPreambleMode::HighTroughputPreambleFormat preambleFormat, Ieee80211HTModeBase::GuardIntervalType guardIntervalType);

};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211VHTMODE_H
