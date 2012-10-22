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
#include "IPvXAddress.h"

#define DIAMETER_HEADER_SIZE  20
#define AVP_MIN_HEADER_SIZE	8
#define AVP_MAX_HEADER_SIZE 12

class AVP;
typedef AVP *AVPPtr;

}}

class noncobject AVPPtr;

enum DisconnectCauses
{
	 REBOOTING 					= 0;
	 BUSY 						= 1;
	 DO_NOT_WANT_TO_TALK_TO_YOU = 2;
}

enum ResultCodes
{
    DIAMETER_PEER_FOUND 		= 0;	// peer already existent
    DIAMETER_SUCCESS 			= 2001;
	DIAMETER_MISSING_AVP 		= 5005;
	DIAMETER_ERROR_USER_UNKNOWN	= 5001;
}

enum AVPCodes 
{
    AVP_UserName			= 1;
    AVP_HostIPAddress 		= 257;
    AVP_AuthApplId			= 258;
    AVP_VendorSpecApplId	= 260;
    AVP_SessionId			= 263;
    AVP_OriginHost 			= 264;
    AVP_SuppVendorId		= 265;
    AVP_VendorId 			= 266;
    AVP_ResultCode 			= 268;
    AVP_ProductName 		= 269;
    AVP_DisconnectCause 	= 273;
    AVP_AuthSessionState	= 277;
	AVP_DestinationRealm 	= 283;
    AVP_DestinationHost 	= 293;
    AVP_OriginRealm 		= 296;
    AVP_ExpResultCode		= 298;
    AVP_MIPHomeAgentAddr	= 334;
    AVP_MIP6AgentInfo		= 486;
    AVP_ServiceSelection	= 493;
    AVP_MSISDN				= 701;
    AVP_ServPartyIPAddr		= 848;
    AVP_RATType				= 1032;
    AVP_SubscriptionData	= 1400;
    AVP_ULRFlags			= 1405;
    AVP_ULAFlags			= 1404;
    AVP_VisitedPLMNId		= 1407;
    AVP_ContextIdentifier	= 1423;
    AVP_AllAPNConfigInclInd	= 1428;
    AVP_APNConfigProfile	= 1429;
    AVP_APNConfig			= 1430;
    AVP_PDNGWAllocType		= 1438;
    AVP_PDNType				= 1456;
}

enum DiameterCommandCodes
{
    CapabilitiesExchange 	= 257;
    DeviceWatchdog 			= 280;
    DisconnectPeer 			= 282;
    UpdateLocation			= 316;
}

enum AuthSessionStates
{
    STATE_MAINTAINED	= 0;
    NO_STATE_MAINTAINED	= 1;    
}

enum RATTypes
{
    DIAMETER_WLAN			= 0;
    DIAMETER_UTRAN			= 1000;
    DIAMETER_GERAN			= 1001;
    DIAMETER_GAN			= 1002;
    DIAMETER_HSPA_EVOLUTION	= 1003;
    DIAMETER_EUTRAN			= 1004;
    DIAMETER_CDMA2000_1X	= 2000;
    DIAMETER_HRPD			= 2001;
    DIAMETER_UMB			= 2002;
    DIAMETER_EHRPD			= 2003;
}

enum AllAPNConfigsInclInds 
{
    All_APN_CONFIGURATIONS_INCLUDED 			= 0;
	MODIFIED_ADDED_APN_CONFIGURATIONS_INCLUDED 	= 1;
}

enum PDNTypes
{
	DIAMETER_IPv4			= 0;
	DIAMETER_IPv6			= 1;
	DIAMETER_IPv4v6			= 2;
	DIAMETER_IPv4_OR_IPv6   = 3; 
}

enum PDNGWAllocTypes
{
	STATIC	= 0;
	DYNAMIC	= 1;   
}

class DiameterHeader
{
    unsigned char version = 1;
    //unsigned int messageLength;	generated during serialize
    bool reqFlag = 0;
    bool prxyFlag = 0;
    bool errFlag = 0;
    bool retrFlag = 0; // 4 more reserved bits = 0
    unsigned int commandCode;
    unsigned int applId;
    unsigned int hopByHopId;
    unsigned int endToEndId; 
}

class AVP extends cPacket
{
    @customize(false);
    unsigned int avpCode = 0;
    bool vendFlag;
    bool manFlag;
    bool privFlag; // 5 more reserved bits = 0 	
    unsigned int vendorId;
    char value[];
}

message DiameterMessage
{
    @customize(true);
	DiameterHeader hdr;
	abstract AVPPtr avps[];
}
