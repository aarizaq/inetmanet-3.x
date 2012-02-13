//
// Copyright (C) Alfonso Ariza 2011 Universidad de Malaga
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

#include "AddressModule.h"
#include "IPvXAddressResolver.h"
#include "NotificationBoard.h"
#include <string.h>

simsignal_t AddressModule::changeAddressSignal;
AddressModule::AddressModule()
{
    // TODO Auto-generated constructor stub
    emitSignal = false;
    isInitialized = false;
    myAddress.set(IPv4Address::UNSPECIFIED_ADDRESS);
}

AddressModule::~AddressModule()
{
    // TODO Auto-generated destructor stub
    if (emitSignal)
    {
        cSimpleModule * owner = check_and_cast<cSimpleModule*> (getOwner());
        owner->emit(changeAddressSignal,this);
    }
}

void AddressModule::initModule(bool mode)
{

    cSimpleModule * owner = check_and_cast<cSimpleModule*>(getOwner());
    emitSignal = mode;
    destAddresses.clear();

    if (owner->hasPar("destAddresses"))
    {
        const char *token;
        cStringTokenizer tokenizer(owner->par("destAddresses"));
        if (!IPvXAddressResolver().tryResolve(owner->getParentModule()->getFullPath().c_str(), myAddress))
            return;

        while ((token = tokenizer.nextToken()) != NULL)
        {
            if (strstr(token, "Broadcast") != NULL)
            {
                if (!myAddress.isIPv6())
                    destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
                else
                    destAddresses.push_back(IPv6Address::ALL_NODES_1);
            }
            else
            {
                IPvXAddress addr = IPvXAddressResolver().resolve(token);
                if (addr != myAddress)
                    destAddresses.push_back(addr);
            }
        }
    }
    if (emitSignal)
    {
        changeAddressSignal = owner->registerSignal("changeAddressSignal");
        simulation.getSystemModule()->subscribe(changeAddressSignal, this);
        if (simTime() > 0)
            owner->emit(changeAddressSignal, this);

    }
    isInitialized = true;
}

IPvXAddress AddressModule::getAddress(int val)
{
    if (val == -1)
    {
        if (chosedAddresses.isUnspecified() && !destAddresses.empty())
            chosedAddresses = choseNewAddress();
        return chosedAddresses;
    }
    if (val >= 0 && val < (int)destAddresses.size())
        return destAddresses[val];
    throw cRuntimeError(this, "Invalid index: %i", val);
}

IPvXAddress AddressModule::choseNewAddress()
{
    if (destAddresses.empty())
        chosedAddresses.set(IPv4Address::UNSPECIFIED_ADDRESS);
    else if (destAddresses.size() == 1)
        chosedAddresses =  destAddresses[0];
    else
    {
        int k = intrand((long)destAddresses.size());
        chosedAddresses = destAddresses[k];
    }
    return chosedAddresses;
}

void AddressModule::receiveSignal(cComponent *src, simsignal_t id, cObject *obj)
{
    if (id != changeAddressSignal)
        return;

    if (obj == this)
        return;

    // rebuild address destination table
    cSimpleModule * owner = check_and_cast<cSimpleModule*> (getOwner());
    const char *destAddrs = owner->par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    IPvXAddress myAddr = IPvXAddressResolver().resolve(owner->getParentModule()->getFullPath().c_str());
    while ((token = tokenizer.nextToken()) != NULL)
    {
        if (strstr(token, "Broadcast") != NULL)
            destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
        else
        {
            IPvXAddress addr = IPvXAddressResolver().resolve(token);
            if (addr != myAddr)
                destAddresses.push_back(addr);
        }
    }
    if (destAddresses.empty())
    {
        IPvXAddress aux;
        chosedAddresses = aux;
        return;
    }
    // search if the address continue
    for (unsigned int i = 0; i < destAddresses.size(); i++)
    {
        if (chosedAddresses == destAddresses[i])
            return;
    }
    // choose other address
    chosedAddresses = choseNewAddress();
    isInitialized = true;
}

void AddressModule::rebuildAddressList()
{
    cSimpleModule * owner = check_and_cast<cSimpleModule*> (getOwner());
    IPvXAddress myAddr;
    IPvXAddressResolver().tryResolve(owner->getParentModule()->getFullPath().c_str(), myAddr);
    if (myAddr == myAddress)
        return;

    if (isInitialized)
        return;
    const char *destAddrs = owner->par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken()) != NULL)
    {
        if (strstr(token, "Broadcast") != NULL)
            destAddresses.push_back(IPv4Address::ALLONES_ADDRESS);
        else
        {
            IPvXAddress addr = IPvXAddressResolver().resolve(token);
            if (addr != myAddr)
                destAddresses.push_back(addr);
        }
    }
    if (destAddresses.empty())
    {
        IPvXAddress aux;
        chosedAddresses = aux;
        return;
    }
    // search if the address continue
    for (unsigned int i = 0; i < destAddresses.size(); i++)
    {
        if (chosedAddresses == destAddresses[i])
            return;
    }
    // choose other address
    chosedAddresses = choseNewAddress();
    if (emitSignal)
        owner->emit(changeAddressSignal, this);
}
