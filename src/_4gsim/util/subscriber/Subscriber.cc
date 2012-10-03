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

#include "Subscriber.h"
#include "LTEUtils.h"
#include "GTPUtils.h"

Subscriber::Subscriber() {
	// TODO Auto-generated constructor stub
	ratType = EUTRAN;
	s11Tunn = NULL;
	msisdn = NULL;
	channelNr = -1;
	enbId = 0;
	mmeId = 0;
	plmnId = NULL;
	cellId = NULL;
	tac = NULL;
	emm = NULL;
	mmeCode = NULL;
	mmeGrId = NULL;
	status = SUB_INACTIVE;
	gtpProc = NoProcedure;
}

Subscriber::~Subscriber() {
	// TODO Auto-generated destructor stub
	if (emm != NULL)
		delete emm;
	if (esm != NULL)
		delete esm;
}

std::string Subscriber::info() const {

    std::stringstream out;
    if (enbId != 0)
    	out << "\tenbId:" << enbId << "\n";
    if (mmeId != 0)
    	out << "\tmmeId:" << mmeId << "\n";
    out << "\tstatus:" << statusName() << "\n";
    if (channelNr != -1)
    	out << "\tchannelNr:" << channelNr << "\n";
    if (emm != NULL)
    	out << emm->info(1);
    if (plmnId != NULL)
    	out << "\tplmnId:" << LTEUtils().toASCIIString(plmnId, PLMNID_CODED_SIZE) << "\n";
    if (cellId != NULL)
    	out << "\tcellId:" << LTEUtils().toASCIIString(cellId, CELLID_CODED_SIZE, ASCII_TYPE) << "\n";
    if (tac != NULL)
    	out << "\ttac:" << LTEUtils().toASCIIString(tac, TAC_CODED_SIZE, ASCII_TYPE) << "\n";
    if (msisdn != NULL)
    	out << "\tmsisdn:" << LTEUtils().toASCIIString(msisdn, MSISDN_CODED_SIZE) << "\n";
    if (s11Tunn != NULL)
    	out << "\ts11tunn:{ " << s11Tunn->info(2) <<" }\n";
    if (esm != NULL)
    	out << esm->info(1);
    return out.str();
}

const char *Subscriber::statusName() const {
#define CASE(x) case x: s=#x; break
    const char *s = "unknown";
    switch (status) {
        CASE(SUB_ACTIVE);
        CASE(SUB_INACTIVE);
        CASE(SUB_PENDING);
    }
    return s;
#undef CASE
}

void Subscriber::initEntities(unsigned char appType) {
	ESMEntity *esm = new ESMEntity(appType);
	EMMEntity *emm = new EMMEntity(appType);
	emm->setPeer(esm);
	esm->setPeer(emm);
	emm->setOwner(this);
	esm->setOwner(this);
}

unsigned Subscriber::popSeqNr() {
	unsigned seqNr = seqNrs.back();
	seqNrs.pop_back();
	return seqNr;
}

