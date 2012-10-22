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

#ifndef ILOCATOR_H_
#define ILOCATOR_H_

#include "ModuleAccess.h"
#include <vector>
#include "MACAddress.h"
#include "IPv4Address.h"

class INET_API ILocator
{
    public:
        virtual ~ILocator() {}
        virtual const MACAddress  getLocatorMacToMac(const MACAddress &) = 0;
        virtual const IPv4Address getLocatorMacToIp(const MACAddress &) = 0;
        virtual const IPv4Address getLocatorIpToIp(const IPv4Address &) = 0;
        virtual const MACAddress  getLocatorIpToMac(const IPv4Address &) = 0;
        virtual void getApList(const MACAddress &,std::vector<MACAddress>&) = 0;
        virtual void getApListIp(const IPv4Address &,std::vector<IPv4Address>&) = 0;
        virtual bool isAp(const MACAddress &) = 0;
        virtual bool isApIp(const IPv4Address &) = 0;
        virtual bool isThisAp() = 0;
        virtual bool isThisApIp() = 0;

        virtual void setIpAddress(const IPv4Address &add) = 0;
        virtual void setMacAddress(const MACAddress &add) = 0;
        virtual IPv4Address getIpAddress() = 0;
        virtual MACAddress getMacAddress() = 0;

};

class INET_API LocatorModuleAccess : public ModuleAccess<ILocator>
{
  public:
    LocatorModuleAccess() : ModuleAccess<ILocator>("locator") {}
};
#endif /* LOCATORMODULE_H_ */
