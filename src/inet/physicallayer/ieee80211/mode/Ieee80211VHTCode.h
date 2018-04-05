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

#ifndef __INET_IEEE80211VHTSIGNALCODE_H
#define __INET_IEEE80211VHTSIGNALCODE_H

#include "inet/physicallayer/contract/bitlevel/ICode.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211VHTInterleaving.h"
#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/common/bitlevel/AdditiveScrambling.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMModulation.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211VHTCode : public ICode
{
    protected:
        const IForwardErrorCorrection *forwardErrorCorrection;
        const Ieee80211VHTInterleaving *interleaving;
        const AdditiveScrambling *scrambling;

    public:
        Ieee80211VHTCode(const IForwardErrorCorrection *forwardErrorCorrection, const Ieee80211VHTInterleaving *interleaving, const AdditiveScrambling *scrambling);

        const IForwardErrorCorrection *getForwardErrorCorrection() const { return forwardErrorCorrection; }
        const AdditiveScrambling *getScrambling() const { return scrambling; }
        const Ieee80211HTInterleaving *getInterleaving() const { return interleaving; }

        virtual ~Ieee80211VHTCode();

};

class INET_API Ieee80211VHTCompliantCodes
{
    public:
        // Convolutional codes (TODO: LDPC codes).
        static const Ieee80211VHTCode *getCompliantCode(const Ieee80211ConvolutionalCode *convolutionalCode, const Ieee80211OFDMModulation *stream1Modulation, const Ieee80211OFDMModulation *stream2Modulation, const Ieee80211OFDMModulation *stream3Modulation, const Ieee80211OFDMModulation *stream4Modulation, Hz bandwidth, bool withScrambling = true);

};

} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211HTSIGNALCODE_H
