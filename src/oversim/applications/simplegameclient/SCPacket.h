//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

/**
 * @file SCPacket.h
 * @author Helge Backhaus
 */

#ifndef __SCPACKET_H_
#define __SCPACKET_H_

#define SC_MOVE_INDICATION     0
#define SC_ADD_NEIGHBOR        1
#define SC_REMOVE_NEIGHBOR     2
#define SC_RESIZE_AOI          3
#define SC_PARAM_REQUEST       4
#define SC_PARAM_RESPONSE      5
#define SC_EV_CHAT             6
#define SC_EV_SNOWBALL         7
#define SC_EV_FROZEN           8

class SCBasePacket // SC_PARAM_REQUEST
{
    public:
        unsigned char packetType;
};

class SCAddPacket : public SCBasePacket // SC_ADD_NEIGHBOR
{
    public:
        double posX;
        double posY;
        unsigned int ip;
};

class SCRemovePacket : public SCBasePacket // SC_REMOVE_NEIGHBOR
{
    public:
        unsigned int ip;
};

class SCMovePacket : public SCBasePacket // SC_MOVE_INDICATION
{
    public:
        double posX;
        double posY;
};

class SCAOIPacket : public SCBasePacket // SC_RESIZE_AOI
{
    public:
        double AOI;
};

class SCParamPacket : public SCBasePacket // SC_PARAM_RESPONSE
{
    public:
        double speed;
        double dimension;
        double AOI;
        double delay;
        double startX;
        double startY;
        unsigned int ip;
        unsigned int seed;
};


class SCChatPacket : public SCBasePacket // SC_EV_CHAT
{
    public:
        unsigned int ip;
        char msg[];
};

class SCSnowPacket : public SCBasePacket // SC_EV_SNOWBALL
{
    public:
        unsigned int ip;
        double startX;
        double startY;
        double endX;
        double endY;
        int time_sec;
        int time_usec;
};

class SCFrozenPacket : public SCBasePacket // SC_EV_FROZEN
{
    public:
        unsigned int ip;
        unsigned int source;
        int time_sec;
        int time_usec;
};

#endif
