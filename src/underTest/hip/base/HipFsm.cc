//*********************************************************************************
// File:           HipFsm.cc
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


#include "HipFsm.h"

Define_Module(HipFsm);
HipFsm::HipFsm(){}
HipFsm::~HipFsm(){}


void HipFsm::specInitialize()
{
	hip = check_and_cast<HIP*>(this->getParentModule());
	hitToIpMap = &hip->hitToIpMap;
}
// Getting associated IP by HIT
void HipFsm::findIPaddress(IPv6Address &HIT)
{
		hitToIpMapIt = hitToIpMap->find(HIT);
		dstIPwork = &hitToIpMapIt->second->addr.front();
}

// Put a HIT-IP association to the hitToIpMap list
void HipFsm::setIPaddress(IPv6Address &HIT, IPv6Address& IP)
{
	hitToIpMapIt = hitToIpMap->find(HIT);
	hitToIpMapIt->second->addr.pop_front();
	hitToIpMapIt->second->addr.push_front(IP);
}

// Add a new IP address to the specified HIT
void HipFsm::addIPaddress(IPv6Address &HIT, IPv6Address& IP)
{
	hitToIpMapIt = hitToIpMap->find(HIT);
	hitToIpMapIt->second->addr.push_back(IP);
}

// Removes all IP addresses for specified HIT
void HipFsm::deleteAllIPaddress(IPv6Address &HIT)
{
	hitToIpMapIt = hitToIpMap->find(HIT);
	while(hitToIpMapIt->second->addr.size() > 0)
		hitToIpMapIt->second->addr.pop_back();
}
