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

#ifndef S1APIE_H_
#define S1APIE_H_

#include "S1APContainer.h"
#include "S1APUtils.h"
#include "S1APControlInfo_m.h"
#include "IPvXAddress.h"
#include "BearerContext.h"

/*
 * Simple S1AP message information elements.
 */

typedef Integer<CONSTRAINED, 0, 10000000000> BitRate;

enum CauseMiscValues {
	control_processing_overload					= 0,
	not_enough_user_plane_processing_resources	= 1,
	hardware_failure							= 2,
	om_intervention								= 3,
	unspecified_0								= 4,
	unknown_PLMN								= 5
};

typedef Enumerated<EXTCONSTRAINED, 5> CauseMisc;

enum CauseProtocolValues {
	transfer_syntax_error								= 0,
	abstract_syntax_error_reject						= 1,
	abstract_syntax_error_ignore_and_notify				= 2,
	message_not_compatible_with_receiver_state			= 3,
	semantic_error										= 4,
	abstract_syntax_error_falsely_constructed_message	= 5,
	unspecified_1										= 6
};

typedef Enumerated<EXTCONSTRAINED, 6> CauseProtocol;

enum CauseRadioNetworkValues {
	unspecified_2                                                   = 0,
	tx2relocoverall_expiry                                          = 1,
	successful_handover                                             = 2,
	release_due_to_eutran_generated_reason                          = 3,
	handover_cancelled	                                        	= 4,
	partial_handover	                                        	= 5,
	ho_failure_in_target_EPC_eNB_or_target_system	                = 6,
	ho_target_not_allowed                                           = 7,
	tS1relocoverall_expiry                                          = 8,
	tS1relocprep_expiry                                             = 9,
	cell_not_available                                              = 10,
	unknown_targetID                                                = 11,
	no_radio_resources_available_in_target_cell                     = 12,
	unknown_mme_ue_s1ap_id                                          = 13,
	unknown_enb_ue_s1ap_id                                          = 14,
	unknown_pair_ue_s1ap_id                                         = 15,
	handover_desirable_for_radio_reason                             = 16,
	time_critical_handover                                          = 17,
	resource_optimisation_handover                                  = 18,
	reduce_load_in_serving_cell                                     = 19,
	user_inactivity                                                 = 20,
	radio_connection_with_ue_lost                                   = 21,
	load_balancing_tau_required                                     = 22,
	cs_fallback_triggered                                           = 23,
	ue_not_available_for_ps_service                                 = 24,
	radio_resources_not_available                                   = 25,
	failure_in_radio_interface_procedure                            = 26,
	invalid_qos_combination                                         = 27,
	interrat_redirection                                            = 28,
	interaction_with_other_procedure                                = 29,
	unknown_E_RAB_ID                                                = 30,
	multiple_E_RAB_ID_instances                                     = 31,
	encryption_and_or_integrity_protection_algorithms_not_supported	= 32,
	s1_intra_system_handover_triggered                              = 33,
	s1_inter_system_handover_triggered                              = 34,
	x2_handover_triggered                                           = 35,
	redirection_towards_1xRTT                                       = 36,
	not_supported_QCI_value                                         = 37,
	invalid_CSG_Id                                                  = 38
};

typedef Enumerated<EXTCONSTRAINED, 38> CauseRadioNetwork;

enum CauseTransportValues {
	transport_resource_unavailable	= 0,
	unspecified_3					= 1
};

typedef Enumerated<EXTCONSTRAINED, 1> CauseTransport;

enum CauseNasValues {
	normal_release			= 0,
	authentication_failure	= 1,
	detach					= 2,
	unspecified_4			= 3,
	csg_subscription_expiry	= 4
};

typedef Enumerated<EXTCONSTRAINED, 4> CauseNas;

typedef BitString<CONSTRAINED, 28, 28> CellIdentity;

typedef PrintableString<EXTCONSTRAINED, 1, 150> EnbName;

typedef Integer<CONSTRAINED, 0, 16777215> EnbUeS1apId;

typedef BitString<EXTCONSTRAINED, 16, 16> EncryptionAlgorithms;

typedef Integer<EXTCONSTRAINED, 0, 15> ERabId;

typedef OctetString<CONSTRAINED, 4, 4> GtpTeid;

typedef BitString<CONSTRAINED, 28, 28> HomeEnbId;

typedef EncryptionAlgorithms IntegrityProtectionAlgorithms;

typedef BitString<CONSTRAINED, 20, 20> MacroEnbId;

typedef OctetString<CONSTRAINED, 1, 1> MmeCode;

typedef Integer<CONSTRAINED, 0, 4294967295> MmeUeS1apId;

typedef EnbName MmeName;

class NasPdu : public OctetStringBase {
public:
	NasPdu(char *val = NULL, int64_t len = 0);
	NasPdu(S1APControlInfo *ctrl);
};

enum PagingDrxValues {
	v32		= 0,
	v64		= 1,
	v128	= 2,
	v256	= 3
};

typedef Enumerated<EXTCONSTRAINED, 3> PagingDrx;

enum PreEmptionCapabilityValues {
	shall_not_trigger_pre_emption	= 0,
	may_trigger_pre_emption			= 1,
};

typedef Enumerated<CONSTRAINED, 1> PreEmptionCapability;

enum PreEmptionVulnerabilityValues {
	not_pre_emptable	= 0,
	pre_emptable		= 1,
};

typedef Enumerated<CONSTRAINED, 1> PreEmptionVulnerability;

enum PriorityLevelValues {
	spare 		= 0,
	highest		= 1,
	lowest		= 14,
	no_priority	= 15
};

typedef Integer<CONSTRAINED, 0, 15> PriorityLevel;

typedef Integer<CONSTRAINED, 0, 255> Qci;

typedef Integer<CONSTRAINED, 0, 255> RelativeMmeCapacity;

enum RrcEstablishmentCauseValues {
	emergency				= 0,
	highPriorityAccess		= 1,
	mt_Access				= 2,
	mo_Signalling			= 3,
	mo_Data					= 4
};

typedef Enumerated<EXTCONSTRAINED, 4> RrcEstablishmentCause;

typedef BitString<CONSTRAINED, 256, 256> SecurityKey;

typedef OctetString<CONSTRAINED, 2, 2> Tac;

typedef Tac MmeGroupId;

typedef OctetString<CONSTRAINED, 3, 3> TbcdString;

typedef TbcdString PlmnIdentity;

typedef BitString<EXTCONSTRAINED, 1, 160> TransportLayerAddress;

/*
 * Compound S1AP message information elements.
 */

class AllocationAndRetentionPriority : public Sequence {
private:
	static const void *itemsInfo[4];
	static bool itemsPres[4];
public:
	static const Info theInfo;
	AllocationAndRetentionPriority() : Sequence(&theInfo) {}
	AllocationAndRetentionPriority(unsigned char prioLvl = 0, unsigned char preCapab = 0, unsigned char preVul = 0, ProtocolExtContainer *exts = NULL);

	unsigned char getPriorityLevel() const { return static_cast<PriorityLevel*>(items[0])->getValue(); }
	unsigned char getPreEmptionCapability() const { return static_cast<PreEmptionCapability*>(items[1])->getValue(); }
	unsigned char getPreEmptionVulnerability() const { return static_cast<PreEmptionVulnerability*>(items[2])->getValue(); }

	void setPriorityLevel(unsigned char prioLvl) { static_cast<PriorityLevel*>(items[0])->setValue(prioLvl); }
	void setPreEmptionCapability(unsigned char preCapab) { static_cast<PriorityLevel*>(items[1])->setValue(preCapab); }
	void setPreEmptionVulnerability(unsigned char preVul) { static_cast<PriorityLevel*>(items[2])->setValue(preVul); }
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

typedef SequenceOf<PlmnIdentity, CONSTRAINED, 1, maxnoofBPLMNs> Bplmns;
Bplmns *toBplmns(std::vector<char*> bplmns);
std::vector<char*> fromBplmns(Bplmns *bplmns);

enum CauseChoices {
	causeRadioNetwork	= 0,
	causeTransport		= 1,
	causeNas			= 2,
	causeProtocol		= 3,
	causeMisc			= 4
};

class Cause : public Choice {
private:
	static const void *choicesInfo[5];
public:
	static const Info theInfo;
	Cause(unsigned char choice = 0, unsigned char cause = 0);

	void setValue(unsigned char cause);
};

enum EnbIdChoices {
	macroEnbId = 0,
	homeEnbId = 1
};

class EnbId : public Choice {
private:
	static const void *choicesInfo[2];
public:
	static const Info theInfo;
	EnbId() : Choice(&theInfo) {}

	void setId(char *id);
	char *getId();
};

class EutranCgi : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	EutranCgi() : Sequence(&theInfo) {}
	EutranCgi(char *plmnId, char *cellId, ProtocolExtContainer *exts = NULL);

	char *getPlmnIdentity() const { return static_cast<PlmnIdentity*>(items[0])->getValue(); }
	char *getCellIdentity() const { return static_cast<CellIdentity*>(items[1])->getValue(); }

	void setPlmnIdentity(char *plmnId) { static_cast<PlmnIdentity*>(items[0])->setValue(plmnId); }
	void setCellIdentity(char *cellId) { static_cast<CellIdentity*>(items[1])->setValue(cellId); }
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

class GbrQosInformation : public Sequence {
private:
	static const void *itemsInfo[5];
	static bool itemsPres[5];
public:
	static const Info theInfo;
	GbrQosInformation() : Sequence(&theInfo) {}
	GbrQosInformation(int64_t maxBrDl, int64_t maxBrUl, int64_t guarBrDl, int64_t guarBrUl, ProtocolExtContainer *exts = NULL);

	int64_t getMaximumBitrateDL() const { return static_cast<BitRate*>(items[0])->getValue(); }
	int64_t getMaximumBitrateUL() const { return static_cast<BitRate*>(items[1])->getValue(); }
	int64_t getGuaranteedBitrateDL() const { return static_cast<BitRate*>(items[2])->getValue(); }
	int64_t getGuaranteedBitrateUL() const { return static_cast<BitRate*>(items[3])->getValue(); }

	void setMaximumBitrateDL(int64_t maxBrDl) { static_cast<BitRate*>(items[0])->setValue(maxBrDl); }
	void setMaximumBitrateUL(int64_t maxBrUl) { static_cast<BitRate*>(items[1])->setValue(maxBrUl); }
	void setGuaranteedBitrateDL(int64_t guarBrDl) { static_cast<BitRate*>(items[2])->setValue(guarBrDl); }
	void setGuaranteedBitrateUL(int64_t guarBrUl) { static_cast<BitRate*>(items[3])->setValue(guarBrUl); }
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

class ERabLevelQosParameters : public Sequence {
private:
	static const void *itemsInfo[4];
	static bool itemsPres[4];
	static GbrQosInformation gbrDef;
public:
	static const Info theInfo;
	ERabLevelQosParameters() : Sequence(&theInfo) {}
	ERabLevelQosParameters(unsigned char qci, AllocationAndRetentionPriority& alloc, GbrQosInformation& gbr = gbrDef, ProtocolExtContainer *exts = NULL);

	unsigned char getQci() { return static_cast<Qci*>(items[0])->getValue(); }
	AllocationAndRetentionPriority *getAllocationAndRetentionPriority() { return static_cast<AllocationAndRetentionPriority*>(items[1]); }
	GbrQosInformation *getGbrQosInformation() { return static_cast<GbrQosInformation*>(items[2]); }
	ProtocolExtContainer *getProtocolExtContainer() { return static_cast<ProtocolExtContainer*>(items[3]); }

	void setQci(unsigned char qci) { static_cast<Qci*>(items[0])->setValue(qci); }
	void setAllocationAndRetentionPriority(const AllocationAndRetentionPriority& alloc) { *static_cast<AllocationAndRetentionPriority*>(items[1]) = alloc; }
	void setGbrQosInformation(const GbrQosInformation& gbr) { *static_cast<GbrQosInformation*>(items[2]) = gbr; }
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

class ERabToBeSetupItemCtxtSuReq : public Sequence {
private:
	static const void *itemsInfo[6];
	static bool itemsPres[6];
public:
	static const Info theInfo;
	ERabToBeSetupItemCtxtSuReq() : Sequence(&theInfo) {}
	ERabToBeSetupItemCtxtSuReq(unsigned char erabId, ERabLevelQosParameters& qos, IPvXAddress addr, unsigned teid, NasPdu *nasPdu = NULL, ProtocolExtContainer *exts = NULL);

	unsigned char getERabId() { return static_cast<ERabId*>(items[0])->getValue(); }
	ERabLevelQosParameters *getERabLevelQosParameters() { return static_cast<ERabLevelQosParameters*>(items[1]); }
	IPvXAddress getTransportLayerAddress();
	unsigned getGtpTeid();
	NasPdu *getNasPdu() { return static_cast<NasPdu*>(items[4]); }

	void setERabId(unsigned char erabId) { static_cast<ERabId*>(items[0])->setValue(erabId); }
	void setERabLevelQosParameters(const ERabLevelQosParameters& qos) { *static_cast<ERabLevelQosParameters*>(items[1]) = qos; }
	void setTransportLayerAddress(IPvXAddress addr);
	void setGtpTeid(unsigned teid);
	void setNasPdu(NasPdu *nasPdu);
	void setProtocolExtContainer(ProtocolExtContainer *exts);

//	BearerContext *getBearerContext(PDNConnection *conn);
};

typedef SequenceOf<ProtocolIeSingleContainer, CONSTRAINED, 1, maxNrOfE_RABs> ERabToBeSetupListCtxtSuReq;
ERabToBeSetupListCtxtSuReq *toERabToBeSetupListCtxtSuReq(std::vector<BearerContext*> bearers);
std::vector<BearerContext*> fromERabToBeSetupListCtxtSuReq(ERabToBeSetupListCtxtSuReq *eRabToBeSetupListCtxtSuReq);

class ERABSetupItemCtxtSURes : public Sequence {
private:
	static const void *itemsInfo[4];
	static bool itemsPres[4];
public:
	static const Info theInfo;
	ERABSetupItemCtxtSURes() : Sequence(&theInfo) {}
	ERABSetupItemCtxtSURes(unsigned char erabId, IPvXAddress addr, unsigned teid, ProtocolExtContainer *exts = NULL);

	unsigned char getERabId() { return static_cast<ERabId*>(items[0])->getValue(); }
	IPvXAddress getTransportLayerAddress();
	unsigned getGtpTeid();

	void setERabId(unsigned char erabId) { static_cast<ERabId*>(items[0])->setValue(erabId); }
	void setTransportLayerAddress(IPvXAddress addr);
	void setGtpTeid(unsigned teid);
	void setProtocolExtContainer(ProtocolExtContainer *exts);

//	BearerContext *getBearerContext(PDNConnection *conn);
};

typedef SequenceOf<ProtocolIeSingleContainer, CONSTRAINED, 1, maxNrOfE_RABs> ERABSetupListCtxtSURes;
ERABSetupListCtxtSURes *toERABSetupListCtxtSURes(std::vector<BearerContext*> bearers);
std::vector<BearerContext*> fromERABSetupListCtxtSURes(ERABSetupListCtxtSURes *eRABSetupListCtxtSURes);

class GlobalEnbId : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	GlobalEnbId() : Sequence(&theInfo) {}
	GlobalEnbId(char *plmnId, unsigned char choice, char *enbId, ProtocolExtContainer *exts = NULL);

	char *getPlmnIdentity() const { return static_cast<PlmnIdentity*>(items[0])->getValue(); }
	EnbId *getEnbId() const { return static_cast<EnbId*>(items[1]); }

	void setPlmnIdentity(char *plmnId) { static_cast<PlmnIdentity*>(items[0])->setValue(plmnId); }
//	void setEnbId(EnbId *enbId);
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

typedef SequenceOf<MmeGroupId, CONSTRAINED, 1, maxnoofGroupIDs> ServedGroupIds;
ServedGroupIds *toServedGroupIds(std::vector<char*> servGrIds);
std::vector<char*> fromServedGroupIds(ServedGroupIds *servedGroupIds);

typedef SequenceOf<MmeCode, CONSTRAINED, 1, maxnoofMMECs> ServedMmecs;
ServedMmecs *toServedMmecs(std::vector<char*> servMmecs);
std::vector<char*> fromServedMmecs(ServedMmecs *servedMmecs);

typedef SequenceOf<PlmnIdentity, CONSTRAINED, 1, maxnoofPLMNsPerMME> ServedPlmns;
ServedPlmns *toServedPlmns(std::vector<char*> servPlmns);
std::vector<char*> fromServedPlmns(ServedPlmns *servedPlmns);

class ServedGummeisItem : public Sequence {
private:
	static const void *itemsInfo[4];
	static bool itemsPres[4];
public:
	static const Info theInfo;
	ServedGummeisItem() : Sequence(&theInfo) {}
	ServedGummeisItem(std::vector<char*> servPlmns, std::vector<char*> servGrIds, std::vector<char*> servMmecs, ProtocolExtContainer *exts = NULL);

	std::vector<char*> getServedPlmns() const { return fromServedPlmns(static_cast<ServedPlmns*>(items[0])); }
	std::vector<char*> getServedGroupIds() const { return fromServedGroupIds(static_cast<ServedGroupIds*>(items[1])); }
	std::vector<char*> getServedMmecs() const { return fromServedMmecs(static_cast<ServedMmecs*>(items[2])); }


	void setServedGroupIds(std::vector<char*> servGrIds);
	void setServedMmecs(std::vector<char*> servMmecs);
	void setServedPlmns(std::vector<char*> servPlmns);
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

typedef SequenceOf<ServedGummeisItem, CONSTRAINED, 1, maxnoofRATs> ServedGummeis;
ServedGummeis *toServedGummeis(std::vector<ServedGummeiItem> servGummeis);
std::vector<ServedGummeiItem> fromServedGummeis(ServedGummeis *servedGummeis);

class SupportedTasItem : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	SupportedTasItem(char *tac, std::vector<char*> bplmns, ProtocolExtContainer *exts = NULL);

	char *getTac() const { return static_cast<Tac*>(items[0])->getValue(); }
	std::vector<char*> getBplmns() { return fromBplmns(static_cast<Bplmns*>(items[1])); }

	void setTac(char *tac) { static_cast<Tac*>(items[0])->setValue(tac); }
	void setBplmns(std::vector<char*> bplmnsVec);
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

typedef SequenceOf<SupportedTasItem, CONSTRAINED, 1, maxnoofTACs> SupportedTas;
SupportedTas *toSupportedTas(std::vector<SupportedTaItem> suppTas);
std::vector<SupportedTaItem> fromSupportedTas(SupportedTas *supportedTas);

class Tai : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	Tai() : Sequence(&theInfo) {}
	Tai(char *plmnId, char *tac, ProtocolExtContainer *exts = NULL);

	char *getPlmnIdentity() const { return static_cast<PlmnIdentity*>(items[0])->getValue(); }
	char *getTac() const { return static_cast<Tac*>(items[1])->getValue(); }

	void setPlmnIdentity(char *plmnId) { static_cast<PlmnIdentity*>(items[0])->setValue(plmnId); }
	void setTac(char *tac) { static_cast<Tac*>(items[1])->setValue(tac); }
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

class UEAggregateMaximumBitrate : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	UEAggregateMaximumBitrate() : Sequence(&theInfo) {}
	UEAggregateMaximumBitrate(int64_t ueAggrMaxBrDl, int64_t ueAggrMaxBrUl, ProtocolExtContainer *exts = NULL);

	int64_t getUEaggregateMaximumBitRateDL() const { return static_cast<BitRate*>(items[0])->getValue(); }
	int64_t getUEaggregateMaximumBitRateUL() const { return static_cast<BitRate*>(items[1])->getValue(); }

	void setUEaggregateMaximumBitRateDL(int64_t ueAggrMaxBrDl) { static_cast<BitRate*>(items[0])->setValue(ueAggrMaxBrDl); }
	void setUEaggregateMaximumBitRateUL(int64_t ueAggrMaxBrUl) { static_cast<BitRate*>(items[1])->setValue(ueAggrMaxBrUl); }
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

class UESecurityCapabilities : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	UESecurityCapabilities() : Sequence(&theInfo) {}
	UESecurityCapabilities(char *encAlg, unsigned encAlgLen, char *intProtAlg, unsigned intProtAlgLen, ProtocolExtContainer *exts = NULL);

	EncryptionAlgorithms *getEncryptionAlgorithms() const { return static_cast<EncryptionAlgorithms*>(items[0]); }
	IntegrityProtectionAlgorithms *getIntegrityProtectionAlgorithms() const { return static_cast<IntegrityProtectionAlgorithms*>(items[1]); }

	void setEncryptionAlgorithms(EncryptionAlgorithms& encAlg) { *static_cast<EncryptionAlgorithms*>(items[0]) = encAlg; }
	void setIntegrityProtectionAlgorithms(IntegrityProtectionAlgorithms& intProtAlg) { *static_cast<IntegrityProtectionAlgorithms*>(items[1]) = intProtAlg; }
	void setProtocolExtContainer(ProtocolExtContainer *exts);
};

#endif /* S1APIE_H_ */
