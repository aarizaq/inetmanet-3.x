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

#ifndef GATEWAYTABLE_H_
#define GATEWAYTABLE_H_

#include <omnetpp.h>
#include "Gateway.h"

/*
 * Class for gateway table. This table will hold all the gateways in a particular
 * network node. Gateways are used for selection functions in those network nodes.
 * One example is S-GW selection in MME entity during the attach procedure.
 */
class GatewayTable : public cSimpleModule {
private:
	typedef std::vector<Gateway*> Gateways;
	Gateways gws;

    /*
     * Methods for parsing the XML configuration file.
     * ex.
     *  <Gateways>
     *      <Gateway mcc="558" mnc="71" tac="7712" pathId="0"/>
     *  </Gateways>
     */
	void loadGatewaysFromXML(const cXMLElement& config);
public:
	GatewayTable();
	virtual ~GatewayTable();

    /*
     * Method for initializing the gateway table. It will read the configuration
     * from the XML file and fill the table with the generated gateways.
     */
	void initialize(int stage);

    /*
     * Method for finding a gateway for a given TAC. The method returns the gateway,
     * if it is found, or NULL otherwise.
     */
	Gateway *findGateway(char *tac);

    /*
     * Method for deleting a gateway. The method calls first the destructor for the
     * gateway and removes it afterwards.
     */
    void erase(unsigned start, unsigned end);

    /*
     * Wrapper methods.
     */
    unsigned int size() {return gws.size();}
    void push_back(Gateway *gw) { gws.push_back(gw); }
    Gateway *at(unsigned i) { return gws.at(i); }

};

#endif /* GATEWAYTABLE_H_ */
