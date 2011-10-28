//*********************************************************************************
// File:           HipFsm.h
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

#ifndef __HIPFSM_H__
#define __HIPFSM_H__

#include <omnetpp.h>
#include "HipFsmBase.h"
#include "HIP.h"
#include "IPv6Address.h"
#include "HipMessages_m.h"



class HipFsm : public HipFsmBase
{
private:
	HIP *hip;

	//HIP::HitToIpMap *hitToIpMap;
	HIP::HitToIpMap *hitToIpMap;
	//HIP:HitToIpMap::iterator hitToIpMapIt;
	HIP::HitToIpMap::iterator hitToIpMapIt;

public:
	//constructor/destructor
	HipFsm();
	virtual ~HipFsm();
protected:
	virtual void specInitialize();
	virtual void findIPaddress(IPv6Address& HIT);
	virtual void setIPaddress(IPv6Address& HIT, IPv6Address& IP);
	virtual void addIPaddress(IPv6Address &HIT, IPv6Address& IP);
	virtual void deleteAllIPaddress(IPv6Address &HIT);
};

#endif
