//*********************************************************************************
// File:           RvsHIP.h
//
// Authors:        Laszlo Tamas Zeke, Levente Mihalyi, Laszlo Bokor
//
// Copyright: (C) 2008-2009 BME-HT (Department of Telecommunications,
// Budapest University of Technology and Economics), Budapest, Hungary
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
//**********************************************************************************
// Part of: HIPSim++ Host Identity Protocol Simulation Framework developed by BME-HT
//**********************************************************************************

#ifndef __RVSHIP_H__
#define __RVSHIP_H__

#include <HIP.h>
#include "IPv6Address.h"

class INET_API RvsHIP : public HIP
{
private:
	//bool exists;

//work variables
	IPv6Address dstHITwork;
	IPv6Address ownHIT;
public:
	RvsHIP();
	virtual ~RvsHIP();
protected:
	virtual void handleMsgFromNetwork(cMessage *msg);
	virtual void specInitialize();
	virtual void alterHipPacketAndSend(HIPHeaderMessage* hipHead, IPv6Address &rvsIP, IPv6Address &destIP,  IPv6Address &fromIP);
	virtual void handleMsgFromTransport(cMessage *msg);
	virtual void handleAddressChange();

};

#endif
