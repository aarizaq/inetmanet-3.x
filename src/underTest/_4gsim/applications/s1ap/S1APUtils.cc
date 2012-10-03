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

#include "S1APUtils.h"
#include "LTEUtils.h"

SupportedTaItem::SupportedTaItem() {
	tac = NULL;
}

SupportedTaItem::~SupportedTaItem() {
	if (tac) {
		delete tac;
		tac = 0;
	}
	std::vector<char*>::iterator i = bplmns.begin();
	for (;i != bplmns.end(); ++i)
		delete *i;
	bplmns.erase(bplmns.begin(), bplmns.end());
}

SupportedTaItem& SupportedTaItem::operator=(const SupportedTaItem& other) {
	tac = (char*)calloc(sizeof(char), TAC_CODED_SIZE);
	memcpy(tac, other.tac, TAC_CODED_SIZE);
	for (unsigned i = 0; i < other.bplmns.size(); i++) {
		char *bplmn = (char*)calloc(sizeof(char), PLMNID_CODED_SIZE);
		memcpy(bplmn, other.bplmns[i], PLMNID_CODED_SIZE);
		bplmns.push_back(bplmn);
	}
	return *this;
}

ServedGummeiItem::~ServedGummeiItem() {
	std::vector<char*>::iterator i = servGrIds.begin();
	for (;i != servGrIds.end(); ++i)
		delete *i;
	servGrIds.erase(servGrIds.begin(), servGrIds.end());
	i = servMmecs.begin();
	for (;i != servMmecs.end(); ++i)
		delete *i;
	servMmecs.erase(servMmecs.begin(), servMmecs.end());
	i = servPlmns.begin();
	for (;i != servPlmns.end(); ++i)
		delete *i;
	servPlmns.erase(servPlmns.begin(), servPlmns.end());
}

ServedGummeiItem& ServedGummeiItem::operator=(const ServedGummeiItem& other) {
	for (unsigned i = 0; i < other.servGrIds.size(); i++) {
		char *servGrId = (char*)calloc(sizeof(char), GROUPID_CODED_SIZE);
		memcpy(servGrId, other.servGrIds[i], GROUPID_CODED_SIZE);
		servGrIds.push_back(servGrId);
	}
	for (unsigned i = 0; i < other.servMmecs.size(); i++) {
		char *servMmec = (char*)calloc(sizeof(char), MMECODE_CODED_SIZE);
		memcpy(servMmec, other.servMmecs[i], MMECODE_CODED_SIZE);
		servMmecs.push_back(servMmec);
	}
	for (unsigned i = 0; i < other.servPlmns.size(); i++) {
		char *servPlmn = (char*)calloc(sizeof(char), PLMNID_CODED_SIZE);
		memcpy(servPlmn, other.servPlmns[i], PLMNID_CODED_SIZE);
		servPlmns.push_back(servPlmn);
	}
	return *this;
}

S1APUtils::S1APUtils() {
	// TODO Auto-generated constructor stub

}

S1APUtils::~S1APUtils() {
	// TODO Auto-generated destructor stub
}
