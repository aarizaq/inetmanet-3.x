//
// Copyright (C) 2012 Calin Cerchez
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "LTERadioModel.h"
#include "Modulation.h"
#include "FWMath.h"

Register_Class(LTERadioModel);

LTERadioModel::LTERadioModel()
{
    modulation = NULL;
}

LTERadioModel::~LTERadioModel()
{
    delete modulation;
}

void LTERadioModel::initializeFrom(cModule *radioModule)
{
    snirThreshold = dB2fraction((double)radioModule->par("snirThreshold"));
    headerLengthBits = radioModule->par("headerLengthBits");
    bandwidth = radioModule->par("bandwidth");

    const char *modulationName = radioModule->par("modulation");
    if (strcmp(modulationName, "null")==0)
        modulation = new NullModulation();
    else if (strcmp(modulationName, "BPSK")==0)
        modulation = new BPSKModulation();
    else if (strcmp(modulationName, "16-QAM")==0)
        modulation = new QAM16Modulation();
    else if (strcmp(modulationName, "256-QAM")==0)
        modulation = new QAM256Modulation();
    else
        opp_error("unrecognized modulation '%s'", modulationName);
}


double LTERadioModel::calculateDuration(AirFrame *airframe)
{
    return (airframe->getBitLength()+headerLengthBits) / airframe->getBitrate();
}


bool LTERadioModel::isReceivedCorrectly(AirFrame *airframe, const SnrList& receivedList)
{
    // calculate snirMin
    double snirMin = receivedList.begin()->snr;
    for (SnrList::const_iterator iter = receivedList.begin(); iter != receivedList.end(); iter++)
        if (iter->snr < snirMin)
            snirMin = iter->snr;

    if (snirMin <= snirThreshold)
    {
        // if snir is too low for the packet to be recognized
        EV << "COLLISION! Packet got lost\n";
        return false;
    }
    else if (isPacketOK(snirMin, airframe->getBitLength()+headerLengthBits, airframe->getBitrate()))
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


bool LTERadioModel::isPacketOK(double snirMin, int length, double bitrate)
{
    double ber = modulation->calculateBER(snirMin, bandwidth, bitrate);

    if (ber==0.0)
        return true;

    double probNoError = pow(1.0 - ber, length); // probability of no bit error

    if (dblrand() > probNoError)
        return false; // error in MPDU
    else
        return true; // no error
}

double LTERadioModel::dB2fraction(double dB)
{
    return pow(10.0, (dB / 10));
}
