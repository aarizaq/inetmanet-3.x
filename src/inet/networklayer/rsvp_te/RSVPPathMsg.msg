//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


import inet.networklayer.rsvp_te.RSVPPacket;

cplusplus {{
#include "inet/networklayer/rsvp_te/IntServ.h"
#include "inet/networklayer/rsvp_te/RSVPPacket.h"
}}

namespace inet;

//
// FIXME missing documentation
//
packet RSVPPathMsg extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    LabelRequestObj_t label_request;
    SenderDescriptor_t sender_descriptor;
    EroVector ERO;
    int color;

    rsvpKind = PATH_MESSAGE;
}

//
// FIXME missing documentation
//
packet RSVPPathTear extends RSVPPacket
{
    @customize(true);
    RsvpHopObj_t hop;
    SenderTemplateObj_t senderTemplate;
    bool force;
    rsvpKind = PTEAR_MESSAGE;
}


//
// FIXME missing documentation
//
packet RSVPPathError extends RSVPPacket
{
    @customize(true);
    IPv4Address errorNode;
    int errorCode;
    SenderDescriptor_t sender_descriptor;

    rsvpKind = PERROR_MESSAGE;
}
