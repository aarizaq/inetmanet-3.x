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
	#define GTP_CONTROL_PORT			2123
	#define GTP_USER_PORT				2152
	#define GTP_HEADER_MIN_SIZE			8
	#define GTP_HEADER_MAX_SIZE			12
	#define GTPV2_IE_HEADER_SIZE		4
	#define GTPV1_IE_HEADER_MIN_SIZE	1
	#define GTPV1_IE_HEADER_MAX_SIZE	2
	#define GTPV2_CAUSE_IE_MAX_SIZE		6
	#define GTPV2_CAUSE_IE_MIN_SIZE		2
	#define GTP_V1						1
	#define GTP_V2						2
}}

cplusplus {{ 
    class GTPHeader;
    typedef GTPHeader *GTPHeaderPtr; 
    class GTPInfoElem;
    typedef GTPInfoElem *GTPInfoElemPtr;
}} // C++ typedef

class noncobject GTPHeaderPtr;
class noncobject GTPInfoElemPtr;

class GTPInfoElem
{
    unsigned char type;
    unsigned char instance; // 4 bits only for GTPv2
    char value[];			// length is valueArraySize
}

class GTPHeader 
{
    unsigned char version;	// only 3 bits
    unsigned char type;
    unsigned int teid;
    unsigned int seqNr;	// 3 bytes
}

enum GTPMessageType
{
	EchoRequest 			= 1;   
	EchoResponse 			= 2;
	CreateSessionRequest 	= 32;
	CreateSessionResponse 	= 33;
	ModifyBearerRequest		= 34;
	ModifyBearerResponse	= 35;
}

enum GTPv2IEType
{
    GTPV2_IMSI 						= 1;
    GTPV2_Cause 					= 2;
    GTPV2_Recovery					= 3;
    GTPV2_APN 						= 71;
    GTPV2_AMBR 						= 72;
    GTPV2_EBI 						= 73;
    GTPV2_MEI 						= 75;
    GTPV2_MSISDN 					= 76;
    GTPV2_Indication 				= 77;
    GTPV2_PCO 						= 78;
    GTPV2_PAA 						= 79;
    GTPV2_RATType 					= 82;
    GTPV2_ServingNetwork 			= 83;
    GTPV2_ULI 						= 86;
    GTPV2_F_TEID 					= 87;
    GTPV2_BearerContext				= 93;
    GTPV2_ChargingCharacteristics 	= 95;
    GTPV2_TraceInformation 			= 96;
    GTPV2_PDNType 					= 99;
    GTPV2_UETimeZone 				= 114;
    GTPV2_APNRestriction 			= 127;
    GTPV2_SelectionMode 			= 128;
    GTPV2_FQ_CSID 					= 132;
    GTPV2_UCI 						= 145;
}

enum GTPv1IEType
{
    GTPV1_Recovery			= 14;
    GTPV1_TEIDData1			= 16;
    GTPV1_GSNAddress		= 133;
    GTPV1_PrivateExtension	= 255;
    GTPV1_ExtHdrTypeList	= 141;
}

enum GTPv2CauseValue
{
    Request_Accepted	= 16;
    Man_IE_Missing 		= 70;
    No_Res_Avail		= 73;
    Man_IE_Incorrect 	= 69;
    GRE_Key_Not_Found 	= 80;
    Request_Rejected 	= 99;
    Con_IE_Missing 		= 103;
}

enum PDNTypeValues
{
	GTP_IPv4 			= 1;
	GTP_IPv6			= 2;
	GTP_IPv4v6 			= 3;  
	GTP_Unused_PDNType	= 4; 
}

class GTPv1Extension	// only for GTPv1
{
    char content[];
    unsigned char nextType;
}

class GTPv1Header extends GTPHeader
{
    version = 1;	// only 3 bits
    bool pt;
	bool e;	
	bool s;	
  	bool pn;
    unsigned char npduNr;
    unsigned char extNextType;
    GTPv1Extension exts[];
}

class GTPv2Header extends GTPHeader
{
    version = 2;
    bool p;
	bool t;	
}

packet GTPMessage
{
    @customize(true);
    GTPHeaderPtr header;
    abstract GTPInfoElemPtr ies[];
}