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

#include "GatewayTable.h"
#include "LTEUtils.h"
#include <algorithm>

Define_Module(GatewayTable);

std::ostream& operator<<(std::ostream& os, Gateway& g) {
    os << g.info();
    return os;
};

GatewayTable::GatewayTable() {
	// TODO Auto-generated constructor stub

}

GatewayTable::~GatewayTable() {
	// TODO Auto-generated destructor stub
    erase(0, gws.size());
}

void GatewayTable::initialize(int stage) {
	const char *fileName = par("configFile");
	if (fileName == NULL || (!strcmp(fileName, "")))
		error("GatewayTable: Error reading configuration from file %s", fileName);

	cXMLElement* config = ev.getXMLDocument(fileName);
	if (config == NULL)
		error("GatewayTable: Cannot read configuration from file: %s", fileName);

	loadGatewaysFromXML(*config);
	WATCH_PTRVECTOR(gws);
}

Gateway *GatewayTable::findGateway(char *tac) {
	for (unsigned i = 0; i < gws.size(); i++) {
		Gateway *gw = gws.at(i);
		if (!strncmp(gw->getTAC(), tac, TAC_CODED_SIZE))
			return gws.at(i);
	}
	return NULL;
}

void GatewayTable::erase(unsigned start, unsigned end) {
    Gateways::iterator first = gws.begin() + start;
    Gateways::iterator last = gws.begin() + end;
    Gateways::iterator i = first;
    for (;i != last; ++i) {
        delete *i;
    }
    gws.erase(first, last);
}

void GatewayTable::loadGatewaysFromXML(const cXMLElement& config) {
	cXMLElement* gwssNode = config.getElementByPath("Gateways");
	if (gwssNode != NULL) {
	    cXMLElementList gwsList = gwssNode->getChildren();
	    for (cXMLElementList::iterator gwIt = gwsList.begin(); gwIt != gwsList.end(); gwIt++) {

	    	std::string elementName = (*gwIt)->getTagName();
	        if ((elementName == "Gateway")) {
	        	const char *mcc = (*gwIt)->getAttribute("mcc");
	        	if (!mcc)
	        		error("GatewayTable: Gateway has no mcc attribute");

				const char *mnc = (*gwIt)->getAttribute("mnc");
				if (!mnc)
					error("GatewayTable: Gateway has no mnc attribute");

				const char *tac = (*gwIt)->getAttribute("tac");
				if (!tac)
					error("GatewayTable: Gateway has no tac attribute");

				if (!(*gwIt)->getAttribute("pathId"))
					error("GatewayTable: Gateway has no pathId attribute");
				unsigned intfId = atoi((*gwIt)->getAttribute("pathId"));

	        	Gateway *gw = new Gateway(LTEUtils().toPLMNId(mcc, mnc), LTEUtils().toByteString(tac, TAC_UNCODED_SIZE), intfId);
	        	push_back(gw);
	        }
	    }
	}
}
