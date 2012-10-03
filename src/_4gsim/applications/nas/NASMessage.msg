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

cplusplus {{ 
    #define NAS_ESM_HEADER_SIZE		3
    #define NAS_EMM_HEADER_SIZE		2
}}

enum InfoElemFormats
{
	IE_V	= 1;
	IE_LV	= 2;
	IE_LVE	= 3;
	IE_TV	= 4;
	IE_TLV	= 5;
	IE_TLVE	= 6;
}

enum InfoElemTypes
{
	IEType1	= 1;	// TV or V with value length 1/2
	IEType2	= 2;   	// T with value length 0
	IEType3	= 3;	// V or TV with value fixed length >= 1
	IEType4	= 4;	// LV or TLV with value variable length < 255
	IEType6	= 6;	// TLVE and LVE	with value variable length > 255
}

enum ProtocolDiscriminator
{
    ESMMessage	= 2;
    EMMMessage	= 7;
}

enum SecurityHeaderTypes
{
    PlainNASMessage = 0;
}

enum NASMessageTypes
{
    AttachRequest			= 65;
    AttachAccept			= 66;
    AttachComplete			= 67;
    AttachReject			= 68;
    ActDefEPSBearerCtxtReq	= 193;
    ActDefEPSBearerCtxtAcc	= 194;
    PDNConnectivityRequest	= 208;
}

enum RequestTypeValues
{
    InitialRequest 		= 1;
    Handover			= 2;
    Unused_RequestType	= 3;
    Emergency			= 4;
}

enum EPSAttachTypes
{
	EPSAttach 			= 1;
	CombinedAttach 		= 2;
	EmergencyAttach 	= 6;
	Reserved_EPSAttach  = 7;    
}

enum EPSMobileIdentityTypes
{
	IMSI_ID = 1;
	GUTI_ID = 6;
	IMEI_ID = 3;    
}

enum NASInfoElemTypes
{
	EPSMobileId = 50;   
}

enum EMMCauses
{
    IMSIUnknownInHSS 			= 2;
    IllegalUE					= 3;
    EPSServNotAllowed			= 7;
    EPSServNonEPSServNotAllowed	= 8;
}

class NASHeader
{
    unsigned char epsBearerId;	// for ESM
    unsigned char secHdrType;	// for EMM
    unsigned char protDiscr;
    unsigned char procTransId;	// only for ESM
    unsigned char msgType;			// 01------ for EMM
    								// 11------ for ESM 
   	unsigned char length = 0;	// only for serializing/parsing usage
}

class NASInfoElem
{
    unsigned char format;
    unsigned char ieType;			
    unsigned char type;	
    char value[];			// length = value array size
}

packet NASPlainMessage
{
	NASHeader hdr;
    NASInfoElem ies[];
    int encapPos;		// position of encapsulated packet
}