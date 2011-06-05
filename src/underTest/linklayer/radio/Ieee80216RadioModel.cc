//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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

#include "Ieee80216RadioModel.h"
#include "Ieee80211Consts.h"
#include "FWMath.h"

Register_Class(Ieee80216RadioModel);

void Ieee80216RadioModel::initializeFrom(cModule* radioModule)
{
    snirThreshold = dB2fraction(radioModule->par("snirThreshold"));
}

double Ieee80216RadioModel::calculateDuration(AirFrame* airframe)
{
    // The physical layer header is sent with 1Mbit/s and the rest with the frame's bitrate
    return airframe->getByteLength() / airframe->getBitrate(); // + PHY_HEADER_LENGTH/BITRATE_HEADER;
}

bool Ieee80216RadioModel::isReceivedCorrectly(AirFrame* airframe, const SnrList& receivedList)
{
    // calculate snirMin
    double snirMin = receivedList.begin()->snr;
    for (SnrList::const_iterator iter = receivedList.begin(); iter != receivedList.end(); iter++)
        if (iter->snr < snirMin)
            snirMin = iter->snr;
#if OMNETPP_VERSION>0x0400
    cPacket *frame = airframe->getEncapsulatedPacket();
#else
    cPacket *frame = airframe->getEncapsulatedMsg();
#endif
    EV << "packet (" << frame->getClassName() << ")" << frame->getName() << " (" << frame->info() <<
        ") snrMin=" << snirMin << endl;

    if (snirMin <= snirThreshold)
    {
        // if snir is too low for the packet to be recognized
        EV << "COLLISION! Packet got lost\n";
        return false;
    }
    else if (isPacketOK(snirMin, frame->getByteLength(), airframe->getBitrate()))
    {
        EV << "packet was received correctly, it is now handed to upper layer...\n";
        return true;
    }
    else
    {
        EV << "Packet has BIT ERRORS! It is lost!\n";
        return false;
    }
}

bool Ieee80216RadioModel::isPacketOK(double snirMin, int lengthMPDU, double bitrate)
{
    double berHeader, berMPDU;

    berHeader = 0.5 * exp(-snirMin * BANDWIDTH / BITRATE_HEADER);

    // if QPSK modulation with 1/2 FEC
    if (bitrate == 4E+6)
        berMPDU = 0.5 * exp(-snirMin * BANDWIDTH / bitrate);
    // if QPSK modulation with 3/4 FEC
    else if (bitrate == 6E+6)
        berMPDU = 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snirMin * BANDWIDTH / bitrate);
    // if 16-QAM modulation with 1/2 FEC
    else if (bitrate == 8E+6)
        berMPDU = 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snirMin * BANDWIDTH / bitrate);
    // if 16-QAM modulation with 3/4 FEC
    else if (bitrate == 12E+6)
        berMPDU = 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snirMin * BANDWIDTH / bitrate);
    // if 64-QAM modulation with 1/2 FEC
    else if (bitrate == 12E+6)
        berMPDU = 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snirMin * BANDWIDTH / bitrate);
    // if 64-QAM modulation with 2/3 FEC
    else if (bitrate == 16E+6)
        berMPDU = 0.5 * (1 - 1 / sqrt(pow(2.0, 4))) * erfc(snirMin * BANDWIDTH / bitrate);
    //  64-QAM modulation with 3/4 FEC
    else
        berMPDU = 0.25 * (1 - 1 / sqrt(pow(2.0, 8))) * erfc(snirMin * BANDWIDTH / bitrate);

    // probability of no bit error in the PLCP header
    double headerNoError = pow(1.0 - berHeader, HEADER_WITHOUT_PREAMBLE);

    // probability of no bit error in the MPDU
    double MpduNoError = pow(1.0 - berMPDU, lengthMPDU);
    EV << "berHeader: " << berHeader << " berMPDU: " << berMPDU << endl;
    double rand = dblrand();

    if (rand > headerNoError)
        return false;           // error in header
    else if (dblrand() > MpduNoError)
        return false;           // error in MPDU
    else
        return true;            // no error
}

double Ieee80216RadioModel::dB2fraction(double dB)
{
    return pow(10.0, (dB / 10));
}
