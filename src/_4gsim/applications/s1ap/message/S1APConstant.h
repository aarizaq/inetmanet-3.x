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

#ifndef S1APCONSTANT_H_
#define S1APCONSTANT_H_

enum ProcedureCodeValues {
	id_HandoverPreparation						= 0,
	id_HandoverResourceAllocation 				= 1,
	id_HandoverNotification 					= 2,
	id_PathSwitchRequest 						= 3,
	id_HandoverCancel 							= 4,
	id_E_RABSetup								= 5,
	id_E_RABModify 								= 6,
	id_E_RABRelease								= 7,
	id_E_RABReleaseIndication					= 8,
	id_InitialContextSetup						= 9,
	id_Paging									= 10,
	id_downlinkNASTransport						= 11,
	id_initialUEMessage							= 12,
	id_uplinkNASTransport						= 13,
	id_Reset									= 14,
	id_ErrorIndication							= 15,
	id_NASNonDeliveryIndication					= 16,
	id_S1Setup									= 17,
	id_UEContextReleaseRequest					= 18,
	id_DownlinkS1cdma2000tunneling				= 19,
	id_UplinkS1cdma2000tunneling				= 20,
	id_UEContextModification					= 21,
	id_UECapabilityInfoIndication				= 22,
	id_UEContextRelease							= 23,
	id_eNBStatusTransfer					 	= 24,
	id_MMEStatusTransfer						= 25,
	id_DeactivateTrace							= 26,
	id_TraceStart								= 27,
	id_TraceFailureIndication					= 28,
	id_ENBConfigurationUpdate					= 29,
	id_MMEConfigurationUpdate					= 30,
	id_LocationReportingControl					= 31,
	id_LocationReportingFailureIndication		= 32,
	id_LocationReport							= 33,
	id_OverloadStart							= 34,
	id_OverloadStop								= 35,
	id_WriteReplaceWarning						= 36,
	id_eNBDirectInformationTransfer				= 37,
	id_MMEDirectInformationTransfer				= 38,
	id_PrivateMessage							= 39,
	id_eNBConfigurationTransfer					= 40,
	id_MMEConfigurationTransfer					= 41,
	id_CellTrafficTrace							= 42,
	id_Kill										= 43,
	id_downlinkUEAssociatedLPPaTransport		= 44,
	id_uplinkUEAssociatedLPPaTransport			= 45,
	id_downlinkNonUEAssociatedLPPaTransport		= 46,
	id_uplinkNonUEAssociatedLPPaTransport		= 47
};

#define maxPrivateIEs 								65535
#define maxProtocolExtensions 						65535
#define maxProtocolIEs								65535

#define maxNrOfCSGs									256
#define maxNrOfE_RABs								256
#define maxnoofTAIs									256
#define maxnoofTACs									256
#define maxNrOfErrors								256
#define maxnoofBPLMNs								6
#define maxnoofPLMNsPerMME							32
#define maxnoofEPLMNs								15
#define maxnoofEPLMNsPlusOne						16
#define maxnoofForbLACs								4096
#define maxnoofForbTACs								4096
#define maxNrOfIndividualS1ConnectionsToReset		256
#define maxnoofCells								16
#define maxnoofTAIforWarning						65535
#define maxnoofCellID								65535
#define maxnoofEmergencyAreaID						65535
#define maxnoofCellinTAI							65535
#define maxnoofCellinEAI							65535
#define maxnoofeNBX2TLAs							2
#define maxnoofRATs									8
#define maxnoofGroupIDs								65535
#define maxnoofMMECs								256

enum ProtocolIeIds {
	id_MME_UE_S1AP_ID									= 0,
	id_HandoverType										= 1,
	id_Cause											= 2,
	id_SourceID											= 3,
	id_TargetID											= 4,
	id_eNB_UE_S1AP_ID									= 8,
	id_E_RABSubjecttoDataForwardingList					= 12,
	id_E_RABtoReleaseListHOCmd							= 13,
	id_E_RABDataForwardingItem							= 14,
	id_E_RABReleaseItemBearerRelComp					= 15,
	id_E_RABToBeSetupListBearerSUReq					= 16,
	id_E_RABToBeSetupItemBearerSUReq					= 17,
	id_E_RABAdmittedList								= 18,
	id_E_RABFailedToSetupListHOReqAck					= 19,
	id_E_RABAdmittedItem								= 20,
	id_E_RABFailedtoSetupItemHOReqAck					= 21,
	id_E_RABToBeSwitchedDLList							= 22,
	id_E_RABToBeSwitchedDLItem							= 23,
	id_E_RABToBeSetupListCtxtSUReq						= 24,
	id_TraceActivation									= 25,
	id_NAS_PDU											= 26,
	id_E_RABToBeSetupItemHOReq							= 27,
	id_E_RABSetupListBearerSURes						= 28,
	id_E_RABFailedToSetupListBearerSURes				= 29,
	id_E_RABToBeModifiedListBearerModReq				= 30,
	id_E_RABModifyListBearerModRes						= 31,
	id_E_RABFailedToModifyList							= 32,
	id_E_RABToBeReleasedList							= 33,
	id_E_RABFailedToReleaseList							= 34,
	id_E_RABItem										= 35,
	id_E_RABToBeModifiedItemBearerModReq				= 36,
	id_E_RABModifyItemBearerModRes						= 37,
	id_E_RABReleaseItem									= 38,
	id_E_RABSetupItemBearerSURes						= 39,
	id_SecurityContext									= 40,
	id_HandoverRestrictionList							= 41,
	id_UEPagingID 										= 43,
	id_pagingDRX 										= 44,
	id_TAIList											= 46,
	id_TAIItem											= 47,
	id_E_RABFailedToSetupListCtxtSURes					= 48,
	id_E_RABReleaseItemHOCmd							= 49,
	id_E_RABSetupItemCtxtSURes							= 50,
	id_E_RABSetupListCtxtSURes							= 51,
	id_E_RABToBeSetupItemCtxtSUReq						= 52,
	id_E_RABToBeSetupListHOReq							= 53,
	id_GERANtoLTEHOInformationRes						= 55,
	id_UTRANtoLTEHOInformationRes						= 57,
	id_CriticalityDiagnostics 							= 58,
	id_Global_ENB_ID									= 59,
	id_eNBname											= 60,
	id_MMEname											= 61,
	id_ServedPLMNs										= 63,
	id_SupportedTAs										= 64,
	id_TimeToWait										= 65,
	id_uEaggregateMaximumBitrate						= 66,
	id_TAI												= 67,
	id_E_RABReleaseListBearerRelComp					= 69,
	id_cdma2000PDU										= 70,
	id_cdma2000RATType									= 71,
	id_cdma2000SectorID									= 72,
	id_SecurityKey										= 73,
	id_UERadioCapability								= 74,
	id_GUMMEI_ID										= 75,
	id_E_RABInformationListItem							= 78,
	id_Direct_Forwarding_Path_Availability				= 79,
	id_UEIdentityIndexValue								= 80,
	id_cdma2000HOStatus									= 83,
	id_cdma2000HORequiredIndication						= 84,
	id_E_UTRAN_TracE_ID									= 86,
	id_RelativeMMECapacity								= 87,
	id_SourceMME_UE_S1AP_ID								= 88,
	id_Bearers_SubjectToStatusTransfer_Item				= 89,
	id_eNB_StatusTransfer_TransparentContainer			= 90,
	id_UE_associatedLogicalS1_ConnectionItem			= 91,
	id_ResetType										= 92,
	id_UE_associatedLogicalS1_ConnectionListResAck		= 93,
	id_E_RABToBeSwitchedULItem							= 94,
	id_E_RABToBeSwitchedULList							= 95,
	id_S_TMSI											= 96,
	id_cdma2000OneXRAND									= 97,
	id_RequestType										= 98,
	id_UE_S1AP_IDs										= 99,
	id_EUTRAN_CGI										= 100,
	id_OverloadResponse									= 101,
	id_cdma2000OneXSRVCCInfo							= 102,
	id_E_RABFailedToBeReleasedList						= 103,
	id_SourcE_ToTarget_TransparentContainer				= 104,
	id_ServedGUMMEIs									= 105,
	id_SubscriberProfileIDforRFP						= 106,
	id_UESecurityCapabilities							= 107,
	id_CSFallbackIndicator								= 108,
	id_CNDomain											= 109,
	id_E_RABReleasedList								= 110,
	id_MessageIdentifier								= 111,
	id_SerialNumber										= 112,
	id_WarningAreaList									= 113,
	id_RepetitionPeriod									= 114,
	id_NumberofBroadcastRequest							= 115,
	id_WarningType										= 116,
	id_WarningSecurityInfo								= 117,
	id_DataCodingScheme									= 118,
	id_WarningMessageContents							= 119,
	id_BroadcastCompletedAreaList						= 120,
	id_Inter_SystemInformationTransferTypeEDT			= 121,
	id_Inter_SystemInformationTransferTypeMDT			= 122,
	id_Target_ToSourcE_TransparentContainer				= 123,
	id_SRVCCOperationPossible							= 124,
	id_SRVCCHOIndication								= 125,
	id_NAS_DownlinkCount								= 126,
	id_CSG_Id											= 127,
	id_CSG_IdList										= 128,
	id_SONConfigurationTransferECT						= 129,
	id_SONConfigurationTransferMCT						= 130,
	id_TraceCollectionEntityIPAddress					= 131,
	id_MSClassmark2										= 132,
	id_MSClassmark3										= 133,
	id_RRC_Establishment_Cause							= 134,
	id_NASSecurityParametersfromE_UTRAN					= 135,
	id_NASSecurityParameterstoE_UTRAN					= 136,
	id_DefaultPagingDRX									= 137,
	id_SourcE_ToTarget_TransparentContainer_Secondary	= 138,
	id_Target_ToSourcE_TransparentContainer_Secondary	= 139,
	id_EUTRANRoundTripDelayEstimationInfo				= 140,
	id_BroadcastCancelledAreaList						= 141,
	id_ConcurrentWarningMessageIndicator				= 142,
	id_Data_Forwarding_Not_Possible             		= 143,
	id_ExtendedRepetitionPeriod							= 144,
	id_CellAccessMode									= 145,
	id_CSGMembershipStatus 								= 146,
	id_LPPa_PDU											= 147,
	id_Routing_ID										= 148,
	id_TimE_Synchronization_Info						= 149,
	id_PS_ServiceNotAvailable							= 150,
};

#endif /* S1APCONSTANT_H_ */
