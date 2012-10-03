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

#include <platdep/sockets.h>
#include "S1APIe.h"
#include "S1APPdu.h"

NasPdu::NasPdu(char *val, int64_t len) : OctetStringBase() {
	setValue(val);
	setLength(len);
}

NasPdu::NasPdu(S1APControlInfo *ctrl) : OctetStringBase() {
	char *val = (char*)calloc(ctrl->getValueArraySize(), sizeof(char));
	for (unsigned i = 0; i < ctrl->getValueArraySize(); i++)
		val[i] = ctrl->getValue(i);
	setLength(ctrl->getValueArraySize());
	setValue(val);
}

const void *AllocationAndRetentionPriority::itemsInfo[4] = {
	&PriorityLevel::theInfo,
	&PreEmptionCapability::theInfo,
	&PreEmptionVulnerability::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool AllocationAndRetentionPriority::itemsPres[4] = {
	1,
	1,
	1,
	0
};

const AllocationAndRetentionPriority::Info AllocationAndRetentionPriority::theInfo = {
	AllocationAndRetentionPriority::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	4, 1, 0
};

AllocationAndRetentionPriority::AllocationAndRetentionPriority(unsigned char prioLvl, unsigned char preCapab, unsigned char preVul, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setPriorityLevel(prioLvl);
	setPreEmptionCapability(preCapab);
	setPreEmptionVulnerability(preVul);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void AllocationAndRetentionPriority::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[3])->push_back(exts->at(i));
}

Bplmns *toBplmns(std::vector<char*> bplmnsVec) {
	Bplmns *bplmns = new Bplmns();
	for (unsigned i = 0; i < bplmnsVec.size(); i++)
		bplmns->push_back(PlmnIdentity(bplmnsVec.at(i)));
	return bplmns;
}

std::vector<char*> fromBplmns(Bplmns *bplmns) {
	std::vector<char*> ret;
	for (unsigned i = 0; i < bplmns->size(); i++)
		ret.push_back(static_cast<const PlmnIdentity>(bplmns->at(i)).getValue());
	return ret;
}

const void *Cause::choicesInfo[5] = {
	&CauseRadioNetwork::theInfo,
	&CauseTransport::theInfo,
	&CauseNas::theInfo,
	&CauseProtocol::theInfo,
	&CauseMisc::theInfo,
};

const Cause::Info Cause::theInfo = {
	Choice::create,
	CHOICE,
	0,
	true,
	choicesInfo,
	4
};

Cause::Cause(unsigned char choice, unsigned char cause) : Choice(&theInfo) {
	createChoice(choice);
	setValue(cause);
}

void Cause::setValue(unsigned char cause) {
	if (choice == causeRadioNetwork)
		static_cast<CauseRadioNetwork*>(value)->setValue(cause);
	else if (choice == causeTransport)
		static_cast<CauseTransport*>(value)->setValue(cause);
	else if (choice == causeNas)
		static_cast<CauseNas*>(value)->setValue(cause);
	else if (choice == causeProtocol)
		static_cast<CauseProtocol*>(value)->setValue(cause);
	else if (choice == causeMisc)
		static_cast<CauseMisc*>(value)->setValue(cause);
}

const void *EnbId::choicesInfo[2] = {
	&MacroEnbId::theInfo,
	&HomeEnbId::theInfo
};

const EnbId::Info EnbId::theInfo = {
	Choice::create,
	CHOICE,
	0,
	true,
	choicesInfo,
	1
};

void EnbId::setId(char *id) {
	if (choice == homeEnbId)
		static_cast<HomeEnbId*>(value)->setValue(id);
	else if (choice == macroEnbId)
		static_cast<MacroEnbId*>(value)->setValue(id);
}

char *EnbId::getId() {
	if (choice == homeEnbId)
		return static_cast<HomeEnbId*>(value)->getValue();
	else if (choice == macroEnbId)
		return static_cast<MacroEnbId*>(value)->getValue();
	return NULL;
}

const void *EutranCgi::itemsInfo[3] = {
	&PlmnIdentity::theInfo,
	&CellIdentity::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool EutranCgi::itemsPres[3] = {
	1,
	1,
	0
};

const EutranCgi::Info EutranCgi::theInfo = {
	EutranCgi::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	3, 1, 0
};

EutranCgi::EutranCgi(char *plmnId, char *cellId, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setPlmnIdentity(plmnId);
	setCellIdentity(cellId);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void EutranCgi::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[2])->push_back(exts->at(i));
}

const void *GbrQosInformation::itemsInfo[5] = {
	&BitRate::theInfo,
	&BitRate::theInfo,
	&BitRate::theInfo,
	&BitRate::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool GbrQosInformation::itemsPres[5] = {
	1,
	1,
	1,
	1,
	0
};

const GbrQosInformation::Info GbrQosInformation::theInfo = {
	GbrQosInformation::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	5, 1, 0
};

GbrQosInformation::GbrQosInformation(int64_t maxBrDl, int64_t maxBrUl, int64_t guarBrDl, int64_t guarBrUl, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setMaximumBitrateDL(maxBrDl);
	setMaximumBitrateUL(maxBrUl);
	setGuaranteedBitrateDL(guarBrDl);
	setGuaranteedBitrateUL(guarBrUl);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void GbrQosInformation::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[4])->push_back(exts->at(i));
}

const void *ERabLevelQosParameters::itemsInfo[4] = {
	&Qci::theInfo,
	&AllocationAndRetentionPriority::theInfo,
	&GbrQosInformation::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool ERabLevelQosParameters::itemsPres[4] = {
	1,
	1,
	0,
	0
};

GbrQosInformation ERabLevelQosParameters::gbrDef = GbrQosInformation();

const ERabLevelQosParameters::Info ERabLevelQosParameters::theInfo = {
	ERabLevelQosParameters::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	4, 2, 0
};

ERabLevelQosParameters::ERabLevelQosParameters(unsigned char qci, AllocationAndRetentionPriority& alloc, GbrQosInformation& gbr, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setQci(qci);
	setAllocationAndRetentionPriority(alloc);
	if (gbr.compare(gbrDef)) {
		setOptFlag(0, true);
		setGbrQosInformation(gbr);
	}
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(1, true);
	setProtocolExtContainer(exts);
}

void ERabLevelQosParameters::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[3])->push_back(exts->at(i));
}

const void *ERabToBeSetupItemCtxtSuReq::itemsInfo[6] = {
	&ERabId::theInfo,
	&ERabLevelQosParameters::theInfo,
	&TransportLayerAddress::theInfo,
	&GtpTeid::theInfo,
	&NasPdu::theInfo,
	&ProtocolExtContainer::theInfo
};

bool ERabToBeSetupItemCtxtSuReq::itemsPres[6] = {
	1,
	1,
	1,
	1,
	0,
	0
};

const ERabToBeSetupItemCtxtSuReq::Info ERabToBeSetupItemCtxtSuReq::theInfo = {
	ERabToBeSetupItemCtxtSuReq::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	6, 2, 0
};

ERabToBeSetupItemCtxtSuReq::ERabToBeSetupItemCtxtSuReq(unsigned char erabId, ERabLevelQosParameters& qos, IPvXAddress addr, unsigned teid, NasPdu *nasPdu, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setERabId(erabId);
	setERabLevelQosParameters(qos);
	setTransportLayerAddress(addr);
	setGtpTeid(teid);
	setNasPdu(nasPdu);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(1, true);
	setProtocolExtContainer(exts);
}

IPvXAddress ERabToBeSetupItemCtxtSuReq::getTransportLayerAddress() {
	if (static_cast<TransportLayerAddress*>(items[2])->getLength() == 32) {	// IPv4
		char *val = static_cast<TransportLayerAddress*>(items[2])->getValue();
		return IPvXAddress(IPv4Address(ntohl(*((unsigned*)(val)))));
	} else {
		return IPvXAddress();
	}
}

unsigned ERabToBeSetupItemCtxtSuReq::getGtpTeid() {
	char *val = static_cast<GtpTeid*>(items[3])->getValue();
	return ntohl(*((unsigned*)(val)));
}

void ERabToBeSetupItemCtxtSuReq::setTransportLayerAddress(IPvXAddress addr) {
	if (addr.isIPv6()) {

	} else {
		char *val = (char*)calloc(4, sizeof(char));
		*((unsigned*)val) = ntohl(addr.get4().getInt());
		static_cast<TransportLayerAddress*>(items[2])->setValue(val);
		static_cast<TransportLayerAddress*>(items[2])->setLength(32);
	}
}

void ERabToBeSetupItemCtxtSuReq::setGtpTeid(unsigned teid) {
	char *val = (char*)calloc(4, sizeof(char));
	*((unsigned*)val) = ntohl(teid);
	static_cast<GtpTeid*>(items[3])->setValue(val);
}

void ERabToBeSetupItemCtxtSuReq::setNasPdu(NasPdu *nasPdu) {
	if (nasPdu == NULL)
		nasPdu = new NasPdu();
	else
		setOptFlag(0, true);
	static_cast<NasPdu*>(items[4])->setValue(nasPdu->getValue());
	static_cast<NasPdu*>(items[4])->setLength(nasPdu->getLength());
}

void ERabToBeSetupItemCtxtSuReq::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[5])->push_back(exts->at(i));
}

ERabToBeSetupListCtxtSuReq* toERabToBeSetupListCtxtSuReq(std::vector<BearerContext*> bearers) {
	ERabToBeSetupListCtxtSuReq *eRabToBeSetupListCtxtSuReq = new ERabToBeSetupListCtxtSuReq();
	for (unsigned i = 0; i < bearers.size(); i++) {
		BearerContext *bearer = bearers.at(i);
		/* TODO qos */
		AllocationAndRetentionPriority alloc = AllocationAndRetentionPriority(no_priority, shall_not_trigger_pre_emption, not_pre_emptable);
		GbrQosInformation gbr = GbrQosInformation(100, 100, 100, 100);
		ERabLevelQosParameters qos = ERabLevelQosParameters(bearer->getQci(), alloc, gbr);
		TunnelEndpoint *te = bearer->getENBTunnEnd();
		ERabToBeSetupItemCtxtSuReq *eRabToBeSetupItemCtxtSuReq = new ERabToBeSetupItemCtxtSuReq(bearer->getId(), qos, te->getRemoteAddr(), te->getRemoteId(), bearer->getNasPdu());
		eRabToBeSetupListCtxtSuReq->push_back(ProtocolIeSingleContainer(id_E_RABToBeSetupItemCtxtSUReq, reject, eRabToBeSetupItemCtxtSuReq));
	}
	return eRabToBeSetupListCtxtSuReq;
}

std::vector<BearerContext*> fromERabToBeSetupListCtxtSuReq(ERabToBeSetupListCtxtSuReq *eRabToBeSetupListCtxtSuReq) {
	std::vector<BearerContext*> ret;
	for (unsigned i = 0; i < eRabToBeSetupListCtxtSuReq->size(); i++) {
		/* TODO qos */
		ProtocolIeSingleContainer container = static_cast<ProtocolIeSingleContainer>(eRabToBeSetupListCtxtSuReq->at(i));
		ERabToBeSetupItemCtxtSuReq eRabToBeSetupItemCtxtSuReq = ERabToBeSetupItemCtxtSuReq();
		eRabToBeSetupItemCtxtSuReq.decode(container.getValue()->getValue());

		BearerContext *bearer = new BearerContext();
		GTPPath *path = new GTPPath();
		path->setRemoteAddr(eRabToBeSetupItemCtxtSuReq.getTransportLayerAddress());
		TunnelEndpoint *te = new TunnelEndpoint(path);
		te->setRemoteId(eRabToBeSetupItemCtxtSuReq.getGtpTeid());
		bearer->setId(eRabToBeSetupItemCtxtSuReq.getERabId());
		bearer->setENBTunnEnd(te);
		if (eRabToBeSetupItemCtxtSuReq.getOptFlag(0))
			bearer->setNasPdu(eRabToBeSetupItemCtxtSuReq.getNasPdu());

		ret.push_back(bearer);
	}
	return ret;
}

const void *ERABSetupItemCtxtSURes::itemsInfo[4] = {
	&ERabId::theInfo,
	&TransportLayerAddress::theInfo,
	&GtpTeid::theInfo,
	&ProtocolExtContainer::theInfo
};

bool ERABSetupItemCtxtSURes::itemsPres[4] = {
	1,
	1,
	1,
	0
};

const ERABSetupItemCtxtSURes::Info ERABSetupItemCtxtSURes::theInfo = {
	ERABSetupItemCtxtSURes::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	4, 1, 0
};

ERABSetupItemCtxtSURes::ERABSetupItemCtxtSURes(unsigned char erabId, IPvXAddress addr, unsigned teid, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setERabId(erabId);
	setTransportLayerAddress(addr);
	setGtpTeid(teid);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(1, true);
	setProtocolExtContainer(exts);
}

IPvXAddress ERABSetupItemCtxtSURes::getTransportLayerAddress() {
	if (static_cast<TransportLayerAddress*>(items[1])->getLength() == 32) {	// IPv4
		char *val = static_cast<TransportLayerAddress*>(items[1])->getValue();
		return IPvXAddress(IPv4Address(ntohl(*((unsigned*)(val)))));
	} else {
		return IPvXAddress();
	}
}

unsigned ERABSetupItemCtxtSURes::getGtpTeid() {
	char *val = static_cast<GtpTeid*>(items[2])->getValue();
	return ntohl(*((unsigned*)(val)));
}

void ERABSetupItemCtxtSURes::setTransportLayerAddress(IPvXAddress addr) {
	if (addr.isIPv6()) {

	} else {
		char *val = (char*)calloc(4, sizeof(char));
		*((unsigned*)val) = ntohl(addr.get4().getInt());
		static_cast<TransportLayerAddress*>(items[1])->setValue(val);
		static_cast<TransportLayerAddress*>(items[1])->setLength(32);
	}
}

void ERABSetupItemCtxtSURes::setGtpTeid(unsigned teid) {
	char *val = (char*)calloc(4, sizeof(char));
	*((unsigned*)val) = ntohl(teid);
	static_cast<GtpTeid*>(items[2])->setValue(val);
}

void ERABSetupItemCtxtSURes::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[3])->push_back(exts->at(i));
}

ERABSetupListCtxtSURes* toERABSetupListCtxtSURes(std::vector<BearerContext*> bearers) {
	ERABSetupListCtxtSURes *eRABSetupListCtxtSURes = new ERABSetupListCtxtSURes();
	for (unsigned i = 0; i < bearers.size(); i++) {
		BearerContext *bearer = bearers.at(i);
		TunnelEndpoint *te = bearer->getENBTunnEnd();
		ERABSetupItemCtxtSURes *eRABSetupItemCtxtSURes = new ERABSetupItemCtxtSURes(bearer->getId(), te->getLocalAddr(), te->getLocalId());
		eRABSetupListCtxtSURes->push_back(ProtocolIeSingleContainer(id_E_RABSetupItemCtxtSURes, reject, eRABSetupItemCtxtSURes));
	}
	return eRABSetupListCtxtSURes;
}

std::vector<BearerContext*> fromERABSetupListCtxtSURes(ERABSetupListCtxtSURes *eRABSetupListCtxtSURes) {
	std::vector<BearerContext*> ret;
	for (unsigned i = 0; i < eRABSetupListCtxtSURes->size(); i++) {
		/* TODO qos */
		ProtocolIeSingleContainer container = static_cast<ProtocolIeSingleContainer>(eRABSetupListCtxtSURes->at(i));
		ERABSetupItemCtxtSURes eRABSetupItemCtxtSURes = ERABSetupItemCtxtSURes();
		eRABSetupItemCtxtSURes.decode(container.getValue()->getValue());

		BearerContext *bearer = new BearerContext();
		GTPPath *path = new GTPPath();
		path->setLocalAddr(eRABSetupItemCtxtSURes.getTransportLayerAddress());
		TunnelEndpoint *te = new TunnelEndpoint(path);
		te->setLocalId(eRABSetupItemCtxtSURes.getGtpTeid());
		bearer->setId(eRABSetupItemCtxtSURes.getERabId());
		bearer->setENBTunnEnd(te);

		ret.push_back(bearer);
	}
	return ret;
}

const void *GlobalEnbId::itemsInfo[3] = {
	&PlmnIdentity::theInfo,
	&EnbId::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool GlobalEnbId::itemsPres[3] = {
	1,
	1,
	0
};

const GlobalEnbId::Info GlobalEnbId::theInfo = {
	GlobalEnbId::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	3, 1, 0
};

GlobalEnbId::GlobalEnbId(char *plmnId, unsigned char choice, char *enbId, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setPlmnIdentity(plmnId);
	static_cast<EnbId*>(items.at(1))->createChoice(choice);
	static_cast<EnbId*>(items.at(1))->setId(enbId);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void GlobalEnbId::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[2])->push_back(exts->at(i));
}

ServedGroupIds *toServedGroupIds(std::vector<char*> servGrIds) {
	ServedGroupIds *servedGroupIds = new ServedGroupIds();
	for (unsigned i = 0; i < servGrIds.size(); i++)
		servedGroupIds->push_back(MmeGroupId(servGrIds.at(i)));
	return servedGroupIds;
}

std::vector<char*> fromServedGroupIds(ServedGroupIds *servedGroupIds) {
	std::vector<char*> ret;
	for (unsigned i = 0; i < servedGroupIds->size(); i++)
		ret.push_back(static_cast<const MmeGroupId>(servedGroupIds->at(i)).getValue());
	return ret;
}

ServedMmecs *toServedMmecs(std::vector<char*> servMmecs) {
	ServedMmecs *servedMmecs = new ServedMmecs();
	for (unsigned i = 0; i < servMmecs.size(); i++)
		servedMmecs->push_back(MmeCode(servMmecs.at(i)));
	return servedMmecs;
}

std::vector<char*> fromServedMmecs(ServedMmecs *servedMmecs) {
	std::vector<char*> ret;
	for (unsigned i = 0; i < servedMmecs->size(); i++)
		ret.push_back(static_cast<const MmeCode>(servedMmecs->at(i)).getValue());
	return ret;
}

ServedPlmns *toServedPlmns(std::vector<char*> servPlmns) {
	ServedPlmns *servedPlmns = new ServedPlmns();
	for (unsigned i = 0; i < servPlmns.size(); i++)
		servedPlmns->push_back(PlmnIdentity(servPlmns.at(i)));
	return servedPlmns;
}

std::vector<char*> fromServedPlmns(ServedPlmns *servedPlmns) {
	std::vector<char*> ret;
	for (unsigned i = 0; i < servedPlmns->size(); i++)
		ret.push_back(static_cast<const PlmnIdentity>(servedPlmns->at(i)).getValue());
	return ret;
}

const void *ServedGummeisItem::itemsInfo[4] = {
	&ServedPlmns::theInfo,
	&ServedGroupIds::theInfo,
	&ServedMmecs::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool ServedGummeisItem::itemsPres[4] = {
	1,
	1,
	1,
	0
};

const ServedGummeisItem::Info ServedGummeisItem::theInfo = {
	ServedGummeisItem::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	4, 1, 0
};

ServedGummeisItem::ServedGummeisItem(std::vector<char*> servPlmns, std::vector<char*> servGrIds, std::vector<char*> servMmecs, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setServedPlmns(servPlmns);
	setServedGroupIds(servGrIds);
	setServedMmecs(servMmecs);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void ServedGummeisItem::setServedPlmns(std::vector<char*> servPlmns) {
	ServedPlmns *servedPlmns = toServedPlmns(servPlmns);
	for (unsigned i = 0; i < servedPlmns->size(); i++)
		static_cast<ServedPlmns*>(items[0])->push_back(servedPlmns->at(i));
}

void ServedGummeisItem::setServedGroupIds(std::vector<char*> servGrIds) {
	ServedGroupIds *servedGroupIds = toServedGroupIds(servGrIds);
	for (unsigned i = 0; i < servedGroupIds->size(); i++)
		static_cast<ServedGroupIds*>(items[1])->push_back(servedGroupIds->at(i));
}

void ServedGummeisItem::setServedMmecs(std::vector<char*> servMmecs) {
	ServedMmecs *servedMmecs = toServedMmecs(servMmecs);
	for (unsigned i = 0; i < servedMmecs->size(); i++)
		static_cast<ServedMmecs*>(items[2])->push_back(servedMmecs->at(i));
}

void ServedGummeisItem::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[3])->push_back(exts->at(i));
}

ServedGummeis *toServedGummeis(std::vector<ServedGummeiItem> servGummeis) {
	ServedGummeis *servedGummeis = new ServedGummeis();
	for (unsigned i = 0; i < servGummeis.size(); i++) {
		ServedGummeiItem item = servGummeis[i];
		servedGummeis->push_back(new ServedGummeisItem(item.servPlmns, item.servGrIds, item.servMmecs));
	}
	return servedGummeis;
}

std::vector<ServedGummeiItem> fromServedGummeis(ServedGummeis *servedGummeis) {
	std::vector<ServedGummeiItem> ret;
	for (unsigned i = 0; i < servedGummeis->size(); i++) {
		ServedGummeiItem retItem = ServedGummeiItem();
		ServedGummeisItem item = static_cast<const ServedGummeisItem>(servedGummeis->at(i));
		retItem.servPlmns = item.getServedPlmns();
		retItem.servGrIds = item.getServedGroupIds();
		retItem.servMmecs = item.getServedMmecs();
		ret.push_back(retItem);
	}
	return ret;
}

const void *SupportedTasItem::itemsInfo[3] = {
	&Tac::theInfo,
	&Bplmns::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool SupportedTasItem::itemsPres[3] = {
	1,
	1,
	0
};

const SupportedTasItem::Info SupportedTasItem::theInfo = {
	SupportedTasItem::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	3, 1, 0
};

SupportedTasItem::SupportedTasItem(char *tac, std::vector<char*> bplmns, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setTac(tac);
	setBplmns(bplmns);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void SupportedTasItem::setBplmns(std::vector<char*> bplmnsVec) {
	Bplmns *bplmns = toBplmns(bplmnsVec);
	for (unsigned i = 0; i < bplmns->size(); i++)
		static_cast<Bplmns*>(items[1])->push_back(bplmns->at(i));
}

void SupportedTasItem::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[2])->push_back(exts->at(i));
}

SupportedTas *toSupportedTas(std::vector<SupportedTaItem> suppTas) {
	SupportedTas *supportedTas = new SupportedTas();
	for (unsigned i = 0; i < suppTas.size(); i++) {
		SupportedTaItem item = suppTas[i];
		supportedTas->push_back(new SupportedTasItem(item.tac, item.bplmns));
	}
	return supportedTas;
}

std::vector<SupportedTaItem> fromSupportedTas(SupportedTas *supportedTas) {
	std::vector<SupportedTaItem> ret;
	for (unsigned i = 0; i < supportedTas->size(); i++) {
		SupportedTaItem retItem = SupportedTaItem();
		SupportedTasItem item = static_cast<const SupportedTasItem>(supportedTas->at(i));
		retItem.tac = item.getTac();
		retItem.bplmns = item.getBplmns();
		ret.push_back(retItem);
	}
	return ret;
}

const void *Tai::itemsInfo[3] = {
	&PlmnIdentity::theInfo,
	&Tac::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool Tai::itemsPres[3] = {
	1,
	1,
	0
};

const Tai::Info Tai::theInfo = {
	Tai::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	3, 1, 0
};

Tai::Tai(char *plmnId, char *tac, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setPlmnIdentity(plmnId);
	setTac(tac);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void Tai::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[2])->push_back(exts->at(i));
}

const void *UEAggregateMaximumBitrate::itemsInfo[3] = {
	&BitRate::theInfo,
	&BitRate::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool UEAggregateMaximumBitrate::itemsPres[3] = {
	1,
	1,
	0
};

const UEAggregateMaximumBitrate::Info UEAggregateMaximumBitrate::theInfo = {
	UEAggregateMaximumBitrate::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	3, 1, 0
};

UEAggregateMaximumBitrate::UEAggregateMaximumBitrate(int64_t ueAggrMaxBrDl, int64_t ueAggrMaxBrUl, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	setUEaggregateMaximumBitRateDL(ueAggrMaxBrDl);
	setUEaggregateMaximumBitRateUL(ueAggrMaxBrUl);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void UEAggregateMaximumBitrate::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[2])->push_back(exts->at(i));
}

const void *UESecurityCapabilities::itemsInfo[3] = {
	&EncryptionAlgorithms::theInfo,
	&IntegrityProtectionAlgorithms::theInfo,
	&ProtocolExtContainer::theInfo,
};

bool UESecurityCapabilities::itemsPres[3] = {
	1,
	1,
	0
};

const UESecurityCapabilities::Info UESecurityCapabilities::theInfo = {
	UESecurityCapabilities::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	3, 1, 0
};

UESecurityCapabilities::UESecurityCapabilities(char *encAlg, unsigned encAlgLen, char *intProtAlg, unsigned intProtAlgLen, ProtocolExtContainer *exts) : Sequence(&theInfo) {
	EncryptionAlgorithms encr = EncryptionAlgorithms(encAlg);
	encr.setLength(encAlgLen);
	setEncryptionAlgorithms(encr);
	IntegrityProtectionAlgorithms integr = IntegrityProtectionAlgorithms(intProtAlg);
	integr.setLength(intProtAlgLen);
	setIntegrityProtectionAlgorithms(integr);
	if (exts == NULL)
		exts = new ProtocolExtContainer();
	else
		setOptFlag(0, true);
	setProtocolExtContainer(exts);
}

void UESecurityCapabilities::setProtocolExtContainer(ProtocolExtContainer *exts) {
	for (unsigned i = 0; i < exts->size(); i++)
		static_cast<ProtocolExtContainer*>(items[2])->push_back(exts->at(i));
}
