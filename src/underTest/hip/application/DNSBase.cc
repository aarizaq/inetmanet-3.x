//*********************************************************************************
// File:           DNSBase.cc
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

//simple DNS server module

#include "DNSBase.h"
#include "UDPControlInfo_m.h"
#include <string.h>
#include "IPvXAddressResolver.h"
using namespace std;

Define_Module(DNSBase);

DNSBase::DNSBase()
{
    serverPort = 23;
}

DNSBase::~DNSBase()
{
    for (unsigned int i = 0; i < dnsVector.size(); i++)
        delete dnsVector[i];
}

void DNSBase::initialize()
{
    //loading data after everybody has its address
    scheduleAt(par("startTime"), new cMessage("DNSStart"));
    socket.setOutputGate(gate("udpOut"));
    socket.bind(serverPort);
}

// Loads data from specified XML file
void DNSBase::loadXml()
{
    //read xml
    const char *fileName = par("dnsDataFile");
    if (fileName == NULL || (!strcmp(fileName, "")) || !LoadDataFromXML(fileName))
        error("Error reading DNS configuration from file %s", fileName);
}

void DNSBase::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        delete msg;
        loadXml();

        ev << "dns data readed " << dnsVector.size() << endl;
        //DEBUG
        /* for (unsigned int i=0; i<dnsVector.size(); i++)
         {
         ev << dnsVector[i]->addr;
         ev << dnsVector[i]->HIT;
         ev << dnsVector[i]->domainName;
         }*/
    }
    else if (dynamic_cast<DNSRegRvsMsg *>(msg) != NULL)
    {
        registerRvs(check_and_cast<DNSRegRvsMsg *>(msg));
    }
    else
    {
        UDPDataIndication *controlInfo = check_and_cast<UDPDataIndication *>(msg->getControlInfo());
        DNSBaseMsg* dnsMsg = check_and_cast<DNSBaseMsg *>(msg);
        if (dnsMsg->opcode() != 0)
        {
            //not implemented
            delete dnsMsg;
            return;
        }
        short respType = 0;
        //CASE 1: HIT -> ip
        if (!dnsMsg->addrData().isUnspecified())
            respType = 1;
        //see if it starts with the HIT request prefix i.e. "hip-"
        else if (((string) dnsMsg->data()).length() > 3)
        {
            char prefix[5];
            strncpy(prefix, dnsMsg->data(), 4);
            prefix[4] = '\0';
            //CASE 2: name->HIT & ip
            if (strcmp(prefix, "hip-") == 0)
                respType = 2;
            //CASE 3: name->ip
            else
                respType = 3;
        }
        //CREATING response
        DNSBaseMsg* respMsg = new DNSBaseMsg("DNS Response"); //the response
        respMsg->setId(dnsMsg->getId()); //setting the id
        ev << "response type: " << respType << endl;
        switch (respType)
        {
            case 1: {
                for (unsigned int i = 0; i < dnsVector.size(); i++)
                {
                    if (dnsVector[i]->HIT == dnsMsg->addrData().get6())
                    {
                        //char HITchar[5];
                        //itoa(dnsVector[i]->HIT, HITchar, 10);
                        respMsg->setData(dnsVector[i]->HIT.str().c_str());
                        //if has RVS send RVS address, else send address
                        if (dnsVector[i]->rvs.isUnspecified())
                        {
                            ev << "dns: addr responee" << endl;
                            respMsg->setAddrData(dnsVector[i]->addr);
                        }
                        else
                        {
                            ev << "dns: rvs responee" << endl;
                            respMsg->setAddrData(dnsVector[i]->rvs);
                        }
                        break;
                    }
                }
                break;
            }
            case 2: {
                for (unsigned int i = 0; i < dnsVector.size(); i++)
                {
                    string domainName = dnsMsg->data();
                    domainName = domainName.substr(4);
                    if (dnsVector[i]->domainName == domainName)
                    {
                        //char HITchar[5];
                        //itoa(dnsVector[i]->HIT, HITchar, 10);
                        respMsg->setData(dnsVector[i]->HIT.str().c_str());
                        //if has RVS send RVS address, else send address
                        if (dnsVector[i]->rvs.isUnspecified())
                        {
                            ev << "dns: addr responee" << endl;
                            respMsg->setAddrData(dnsVector[i]->addr);
                        }
                        else
                        {
                            ev << "dns: rvs responee" << endl;
                            respMsg->setAddrData(dnsVector[i]->rvs);
                        }
                        break;
                    }
                }
                break;
            }
            case 3: {
                for (unsigned int i = 0; i < dnsVector.size(); i++)
                {
                    if (dnsVector[i]->domainName == dnsMsg->data())
                    {
                        respMsg->setAddrData(dnsVector[i]->addr);
                        break;
                    }
                }
                break;
            }
        }
        ev << "sending dns response " << endl;
        //DEBUG
        ev << respMsg->data() << " " << respMsg->addrData() << endl;

        // swap src and dest
        /*
         UDPControlInfo *respControlInfo = new UDPControlInfo();
         IPvXAddress srcAddr = controlInfo->srcAddr();
         int srcPort = controlInfo->srcPort();
         respControlInfo->setSrcAddr(controlInfo->destAddr());
         respControlInfo->setSrcPort(controlInfo->destPort());
         respControlInfo->setDestAddr(srcAddr);
         respControlInfo->setDestPort(srcPort);
         respMsg->setControlInfo(respControlInfo);
         */

        socket.sendTo(respMsg, controlInfo->getSrcAddr(), controlInfo->getSrcPort());

        //delete controlInfo;
        delete msg;
    }
}

//loads the xml and builds the vector conataining dns data
bool DNSBase::LoadDataFromXML(const char * filename)
{
    cXMLElement* dnsData = ev.getXMLDocument(filename);
    if (dnsData == NULL)
    {
        error("Cannot read DNS data from file: %s", filename);
    }
    //building the vector conataining dns data
    cXMLElementList xmlDnsEntries = dnsData->getChildren();
    for (cXMLElementList::iterator dnsIt = xmlDnsEntries.begin(); dnsIt != xmlDnsEntries.end(); dnsIt++)
    {
        std::string nodeName = (*dnsIt)->getTagName();
        if (nodeName == "DNSEntry")
        {
            DNSData *d = new DNSData;
            const char *address = (*dnsIt)->getChildrenByTagName("Address")[0]->getNodeValue();
            d->addr = IPvXAddressResolver().resolve(address);
            d->HIT.set((*dnsIt)->getChildrenByTagName("HIT")[0]->getNodeValue());
            d->domainName = (*dnsIt)->getChildrenByTagName("Name")[0]->getNodeValue();
            if ((*dnsIt)->getChildrenByTagName("RVS").size() > 0)
                d->rvs = IPvXAddressResolver().resolve((*dnsIt)->getChildrenByTagName("RVS")[0]->getNodeValue());
            //strcpy(d->domainName, (*dnsIt)->getChildrenByTagName ("Name")[0]->getNodeValue () );
            dnsVector.push_back(d);
        }
    }

    return true;
}

//registers node's RVS
//returns false if node is not found in dnsVector
bool DNSBase::registerRvs(DNSRegRvsMsg* regMsg)
{
    bool hostFound = false;
    for (unsigned int i = 0; i < dnsVector.size(); i++)
    {
        if (dnsVector[i]->HIT == IPv6Address(regMsg->data()))
        {
            hostFound = true;
            dnsVector[i]->rvs = regMsg->addrData();
        }
    }
    return hostFound;
}
