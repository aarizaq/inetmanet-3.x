//*********************************************************************************
// File:           DNSBase.h
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


//simple DNS module


#ifndef __DNSBASE_H__
#define __DNSBASE_H__

#include <vector>
#include <omnetpp.h>
#include "DNSBaseMsg_m.h"
#include "DNSRegRvsMsg_m.h"
#include "IPvXAddress.h"
#include <string.h>
#include "UDPSocket.h"

using namespace std;



class DNSBase : public cSimpleModule
{
  public:
    /**
     * Stores information on a video stream
     */
    struct DNSData
    {
        IPvXAddress addr;   //ipv6 address
        IPv6Address HIT;           //HIT (see HIP module)
        string domainName;  //domain name
        IPvXAddress rvs; //HIP RVS
    };

  protected:
    UDPSocket socket;
    typedef std::vector<DNSData *> DNSVector;
    DNSVector dnsVector;

    // module parameters
    int serverPort; //default is 23

  public:
    DNSBase();
    virtual ~DNSBase();

  protected:
    ///@name Overidden cSimpleModule functions
    //@{
    virtual void initialize();
    virtual void handleMessage(cMessage* msg);
    //@}
  private:
    bool    LoadDataFromXML (const char * filename);
    void    loadXml();
    bool    registerRvs(DNSRegRvsMsg* regMsg);
};

#endif
