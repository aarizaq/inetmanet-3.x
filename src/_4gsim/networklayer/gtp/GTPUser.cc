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

#include "GTPUser.h"
#include "LTEUtils.h"
#include "TunnelEndpointTableAccess.h"
#include "GTPPathTableAccess.h"
#include "BearerContext.h"

Define_Module(GTPUser);

GTPUser::GTPUser() {
	// TODO Auto-generated constructor stub
	localCounter = 1;
	tunnIds = 5 + uniform(5, 10);
	plane = GTP_USER;
}

GTPUser::~GTPUser() {
	// TODO Auto-generated destructor stub
}

void GTPUser::initialize(int stage) {
	GTP::initialize(stage);

	nb->subscribe(this, NF_SUB_NEEDS_TUNN);
}

void GTPUser::receiveChangeNotification(int category, const cPolymorphic *details) {

	Enter_Method_Silent();
	if (category == NF_SUB_NEEDS_TUNN) {
		EV << "GTPUser: Received NF_SUB_NEEDS_TUNN notification. Processing notification.\n";
		BearerContext *bearer = check_and_cast<BearerContext*>(details);
		TunnelEndpoint *userTe = bearer->getENBTunnEnd();
		GTPPath *path = pT->findPath(userTe->getRemoteAddr(), S1_U_eNodeB_GTP_U);
		if (path == NULL) {
			nb->fireChangeNotification(NF_SUB_TUNN_NACK, bearer);
			return;
		}
		TunnelEndpoint *newTe = new TunnelEndpoint(path);
		newTe->setRemoteId(userTe->getRemoteId());
		teT->push_back(newTe);
		bearer->setENBTunnEnd(newTe);
		delete userTe;
		nb->fireChangeNotification(NF_SUB_TUNN_ACK, bearer);
	}
}

