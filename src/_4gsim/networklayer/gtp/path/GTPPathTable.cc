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

#include "GTPPathTable.h"
#include "GTP.h"

Define_Module(GTPPathTable);

std::ostream& operator<<(std::ostream& os, GTPPath& p) {
    os << p.info(0);
    return os;
};

GTPPathTable::GTPPathTable() {
	// TODO Auto-generated constructor stub

}

GTPPathTable::~GTPPathTable() {
	// TODO Auto-generated destructor stub
	erase(0, paths.size());
}

void GTPPathTable::initialize(int stage) {
	WATCH_PTRVECTOR(paths);
}

GTPPath *GTPPathTable::findPath(IPvXAddress addr, unsigned char type) {	// addr can be remote or local
	for (unsigned i = 0; i < paths.size(); i++) {
		GTPPath *path = paths[i];
		if (((path->getRemoteAddr() == addr) && ((path->getType() / 2) == (type) / 2)) ||
				((path->getLocalAddr() == addr) && ((path->getType()) == type)))
			return path;
	}
	return NULL;
}

GTPPath *GTPPathTable::findPath(IPvXAddress ctrlAddr) {
	for (unsigned i = 0; i < paths.size(); i++) {
		GTPPath *path = paths[i];
		if (path->getCtrlAddr() == ctrlAddr)
			return path;
	}
	return NULL;
}

GTPPath *GTPPathTable::findPath(char type) {
	for (unsigned i = 0; i < paths.size(); i++) {
		GTPPath *path = paths[i];
		if (path->getType() == type)
			return path;
	}
	return NULL;
}

GTPPath *GTPPathTable::findPath(cMessage *msg) {
    UDPDataIndication *ctrl = check_and_cast<UDPDataIndication*>(msg->getControlInfo());
	for (unsigned i = 0; i < paths.size(); i++) {
		GTPPath *path = paths[i];
		if ((path->getRemoteAddr() == ctrl->getSrcAddr()) &&
				(path->getLocalAddr() == ctrl->getDestAddr())) {
			if ((path->getPlane() == GTP_CONTROL && ctrl->getDestPort() == GTP_CONTROL_PORT) ||
					(path->getPlane() == GTP_USER && ctrl->getDestPort() == GTP_USER_PORT))
				return path;
		}

	}
	return NULL;
}

void GTPPathTable::erase(unsigned start, unsigned end) {
	GTPPaths::iterator first = paths.begin() + start;
	GTPPaths::iterator last = paths.begin() + end;
	GTPPaths::iterator i = first;
	for (;i != last; ++i)
		delete *i;
	paths.erase(first, last);
}
