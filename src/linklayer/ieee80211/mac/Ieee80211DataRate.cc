// Copyright (C) 2012 OpenSim Ltd
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
// @author: Zoltan Bojthe
//

#include <algorithm>

#include "Ieee80211DataRate.h"

#include "WifiMode.h"


/* Bit rates for 802.11b/g/a/p.
 * Must be ordered by mode, bitrate.
 */
const Ieee80211DescriptorData Ieee80211Descriptor::data[] =
{
    {'a',  6000000, WifiModulationType::GetOfdmRate6Mbps(),false},
    {'a',  9000000, WifiModulationType::GetOfdmRate9Mbps(),false},
    {'a', 12000000, WifiModulationType::GetOfdmRate12Mbps(),false},
    {'a', 18000000, WifiModulationType::GetOfdmRate18Mbps(),false},
    {'a', 24000000, WifiModulationType::GetOfdmRate24Mbps(),false},
    {'a', 36000000, WifiModulationType::GetOfdmRate36Mbps(),false},
    {'a', 48000000, WifiModulationType::GetOfdmRate48Mbps(),false},
    {'a', 54000000, WifiModulationType::GetOfdmRate54Mbps(),false},

    {'b',  1000000, WifiModulationType::GetDsssRate1Mbps(),false},
    {'b',  2000000, WifiModulationType::GetDsssRate2Mbps(),false},
    {'b',  5500000, WifiModulationType::GetDsssRate5_5Mbps(),false},
    {'b', 11000000, WifiModulationType::GetDsssRate11Mbps(),false},

    {'g',  1000000, WifiModulationType::GetDsssRate1Mbps(),false},
    {'g',  2000000, WifiModulationType::GetDsssRate2Mbps(),false},
    {'g',  5500000, WifiModulationType::GetDsssRate5_5Mbps(),false},
    {'g',  6000000, WifiModulationType::GetErpOfdmRate6Mbps(),false},
    {'g',  9000000, WifiModulationType::GetErpOfdmRate9Mbps(),false},
    {'g', 11000000, WifiModulationType::GetDsssRate11Mbps(),false},
    {'g', 12000000, WifiModulationType::GetErpOfdmRate12Mbps(),false},
    {'g', 18000000, WifiModulationType::GetErpOfdmRate18Mbps(),false},
    {'g', 24000000, WifiModulationType::GetErpOfdmRate24Mbps(),false},
    {'g', 36000000, WifiModulationType::GetErpOfdmRate36Mbps(),false},
    {'g', 48000000, WifiModulationType::GetErpOfdmRate48Mbps(),false},
    {'g', 54000000, WifiModulationType::GetErpOfdmRate54Mbps(),false},
 
    {'n',  1000000, WifiModulationType::GetDsssRate1Mbps(),false},
    {'n',  2000000, WifiModulationType::GetDsssRate2Mbps(),false},
    {'n',  5500000, WifiModulationType::GetDsssRate5_5Mbps(),false},
    {'n',  6000000, WifiModulationType::GetErpOfdmRate6Mbps(),false},
    {'n',  6500000, WifiModulationType::GetOfdmRate6_5MbpsBW20MHz(),false},
    {'n',  7200000, WifiModulationType::GetOfdmRate7_2MbpsBW20MHz(),true},
    {'n', 11000000, WifiModulationType::GetDsssRate11Mbps(),false},
    {'n', 12000000, WifiModulationType::GetErpOfdmRate12Mbps(),false},
    {'n', 13000000, WifiModulationType::GetOfdmRate13MbpsBW20MHz(),false},
    {'n', 13500000, WifiModulationType::GetOfdmRate13_5MbpsBW40MHz(),false},
    {'n', 14400000, WifiModulationType::GetOfdmRate14_4MbpsBW20MHz(),true},
    {'n', 15000000, WifiModulationType::GetOfdmRate15MbpsBW40MHz(),true},
    {'n', 19500000, WifiModulationType::GetOfdmRate19_5MbpsBW20MHz(),false},
    {'n', 21000000, WifiModulationType::GetOfdmRate21_7MbpsBW20MHz(),true},
    {'n', 24000000, WifiModulationType::GetErpOfdmRate24Mbps(),false},
    {'n', 26000000, WifiModulationType::GetOfdmRate26MbpsBW20MHz(),false},
    {'n', 27000000, WifiModulationType::GetOfdmRate27MbpsBW40MHz(),false},
    {'n', 28900000, WifiModulationType::GetOfdmRate28_9MbpsBW20MHz(),true},
    {'n', 30000000, WifiModulationType::GetOfdmRate30MbpsBW40MHz(),true},
    {'n', 39000000, WifiModulationType::GetOfdmRate39MbpsBW20MHz(),false},
    {'n', 40500000, WifiModulationType::GetOfdmRate40_5MbpsBW40MHz(),false},
    {'n', 43300000, WifiModulationType::GetOfdmRate43_3MbpsBW20MHz(),true},
    {'n', 45000000, WifiModulationType::GetOfdmRate45MbpsBW40MHz(),true},
    {'n', 52000000, WifiModulationType::GetOfdmRate52MbpsBW20MHz(),false},
    {'n', 54000000, WifiModulationType::GetOfdmRate54MbpsBW40MHz(),false},
    {'n', 57800000, WifiModulationType::GetOfdmRate57_8MbpsBW20MHz(),true},
    {'n', 58500000, WifiModulationType::GetOfdmRate58_5MbpsBW20MHz(),false},
    {'n', 60000000, WifiModulationType::GetOfdmRate60MbpsBW40MHz(),true},
    {'n', 65000000, WifiModulationType::GetOfdmRate65MbpsBW20MHz(),false},
    {'n', 65000000, WifiModulationType::GetOfdmRate65MbpsBW20MHzShGi(),true}, // 1n short guard repeat bit rate
    {'n', 72000000, WifiModulationType::GetOfdmRate72_2MbpsBW20MHz(),true},
    {'n', 81000000, WifiModulationType::GetOfdmRate81MbpsBW40MHz(),false},
    {'n', 90000000, WifiModulationType::GetOfdmRate90MbpsBW40MHz(),true},
    {'n', 108000000, WifiModulationType::GetOfdmRate108MbpsBW40MHz(),false},
    {'n', 120000000, WifiModulationType::GetOfdmRate120MbpsBW40MHz(),true},
    {'n', 121500000, WifiModulationType::GetOfdmRate121_5MbpsBW40MHz(),false},
    {'n', 135000000, WifiModulationType::GetOfdmRate135MbpsBW40MHz(),false},
    {'n', 135000000, WifiModulationType::GetOfdmRate65MbpsBW20MHzShGi(),true}, // 1n short guard repeat bit rate
    {'n', 150000000, WifiModulationType::GetOfdmRate150MbpsBW40MHz(),true},




    {'p',  3000000, WifiModulationType::GetOfdmRate3MbpsBW10MHz(),false},
    {'p',  4500000, WifiModulationType::GetOfdmRate4_5MbpsBW10MHz(),false},
    {'p',  6000000, WifiModulationType::GetOfdmRate6MbpsBW10MHz(),false},
    {'p',  9000000, WifiModulationType::GetOfdmRate9MbpsBW10MHz(),false},
    {'p', 12000000, WifiModulationType::GetOfdmRate12MbpsBW10MHz(),false},
    {'p', 18000000, WifiModulationType::GetOfdmRate18MbpsBW10MHz(),false},
    {'p', 24000000, WifiModulationType::GetOfdmRate24MbpsBW10MHz(),false},
    {'p', 27000000, WifiModulationType::GetOfdmRate27MbpsBW10MHz(),false},


};

const int Ieee80211Descriptor::descriptorSize = sizeof(Ieee80211Descriptor::data)/sizeof(Ieee80211Descriptor::data[0]);

#if 0
// linear search

int Ieee80211Descriptor::findIdx(char mode, double bitrate)
{
    for (int i=0; i<descriptorSize; i++)
    {
        if (data[i].mode == mode)
        {
            if (data[i].bitrate == bitrate)
                return i;
            if (data[i].bitrate > bitrate)
                break;
        }
        if (data[i].mode > mode)
            break;
    }
    return -1;
}

int Ieee80211Descriptor::getMinIdx(char mode)
{
    for (int i=0; i<descriptorSize; i++)
    {
        if (data[i].mode == mode)
            return i;
        if (data[i].mode > mode)
            break;
    }
    throw cRuntimeError("mode '%c' not valid", mode);
}

int Ieee80211Descriptor::getMaxIdx(char mode)
{
    int idx = -1;
    for (int i=0; i<descriptorSize; i++)
    {
        if (data[i].mode == mode)
            idx = i;
        if (data[i].mode > mode)
            break;
    }
    if (idx == -1)
        throw cRuntimeError("mode '%c' not valid", mode);
    return idx;
}

#else

namespace
{
    bool ieee80211DescriptorCompareModeBitrate(const Ieee80211DescriptorData& a, const Ieee80211DescriptorData& b)
    {
        // return a < b;
        return (a.mode < b.mode) || ((a.mode == b.mode) && (a.bitrate < b.bitrate)) || ((a.mode == b.mode) && (a.bitrate == b.bitrate) && (a.guardInterval < b.guardInterval));
    }

    bool ieee80211DescriptorCompareMode(const Ieee80211DescriptorData& a, const Ieee80211DescriptorData& b)
    {
        // return a < b;
        return (a.mode < b.mode);
    }
}

int Ieee80211Descriptor::findIdx(char mode, double bitrate, bool guard)
{
    Ieee80211DescriptorData d;
    d.mode = mode;
    d.bitrate = bitrate;
    d.guardInterval = guard;
    const Ieee80211DescriptorData *found = std::lower_bound(data, &data[descriptorSize], d, ieee80211DescriptorCompareModeBitrate);
    if (mode != 'n' || (mode == 'n' && bitrate != 135000000 && bitrate != 65000000))
    {
        if (found->mode == mode && found->bitrate == bitrate)
            return (int)(found - data);
        return -1;
    }
    else
    {
        if (found->mode == mode && found->bitrate == bitrate && found->guardInterval == guard)
            return (int)(found - data);
        return -1;
    }
}

int Ieee80211Descriptor::getMinIdx(char mode)
{
    Ieee80211DescriptorData d;
    d.mode = mode;

    const Ieee80211DescriptorData *found = std::lower_bound(data, &data[descriptorSize], d, ieee80211DescriptorCompareMode);
    if (found->mode == mode)
        return (int)(found - data);
    throw cRuntimeError("mode '%c' not valid", mode);
}

int Ieee80211Descriptor::getMaxIdx(char mode)
{
    Ieee80211DescriptorData d;
    d.mode = mode;

    const Ieee80211DescriptorData *found = std::upper_bound(data, &data[descriptorSize], d, ieee80211DescriptorCompareMode);
    if (found > data)
        --found;
    if (found->mode == mode)
        return (int)(found - data);
    throw cRuntimeError("mode '%c' not valid", mode);
}

#endif


int Ieee80211Descriptor::getIdx(char mode, double bitrate, bool guard)
{
    int idx = findIdx(mode, bitrate, guard);
    if (idx == -1)
        throw cRuntimeError("mode '%c':%g bps not valid", mode, bitrate);
    return idx;
}

bool Ieee80211Descriptor::incIdx(int& idx)
{
    ASSERT(idx >= 0 && idx < descriptorSize);

    if (data[idx].mode != data[idx+1].mode)
        return false;
    ++idx;
    return true;
}

bool Ieee80211Descriptor::decIdx(int& idx)
{
    ASSERT(idx >= 0 && idx < descriptorSize);

    if (idx==0 || data[idx].mode != data[idx-1].mode)
        return false;
    --idx;
    return true;
}

const Ieee80211DescriptorData& Ieee80211Descriptor::getDescriptor(int idx)
{
    ASSERT(idx >= 0 && idx < descriptorSize);

    return data[idx];
}


