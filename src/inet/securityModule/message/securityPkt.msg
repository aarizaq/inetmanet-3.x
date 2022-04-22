
import inet.linklayer.common.MACAddress;
import inet.linklayer.ieee80211.mgmt.Ieee80211MgmtFrames;

cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/securityModule/Security.h"
#include "inet/securityModule/SecurityKeys.h"
#include <iostream>
#include <vector>
}}

namespace inet::ieee80211;
    
//class  Security  { @existingClass(true); @descriptor(readonly);}

cplusplus {{
typedef SecurityKeys::nonce nonce;
typedef SecurityKeys::key128 key128;
typedef SecurityKeys::key256 key256;
typedef SecurityKeys::key384 key384;
typedef SecurityKeys::mic mic;
typedef SecurityKeys::unit64_t_ unit64_t_;
}}

class nonce { @existingClass; @opaque;}
class key128 { @existingClass; @opaque;}
class key256 { @existingClass; @opaque;}
class key384 { @existingClass; @opaque;}
class mic { @existingClass; @opaque;}

packet SecurityPkt //extends Ieee80211ManagementFrame
{
    char Descriptor_Type[12];
   // char Descriptor_Type=0;
    short Key_Info = 0;
    short Key_Length = 0;
    uint64_t Key_RC;
    nonce Key_Nonce;
    key128 EAPOL_KeyIV;
    uint64_t Key_RSC = 0; //Key Replay Conter
    uint64_t Reserved = 0;
    key128 Mic;
  	short Key_Data_Length = 0;  	
    key128 Key_Data128;
  	key256 Key_Data256;
}


// Ieee80211DataFrame
packet CCMPFrame //extends Ieee80211MeshFrame
{
    uint64_t CCMP_Header; //8 octets
    uint64_t CCMP_Mic; 	
  //  key128 encryptedData128;  
}

packet SAEMsg
{
    short SAE_sendConfirm;
    int SAE_commitScalar;
    int SAE_commitElement;
    int SAE_confirmfield;
    short SAE_finiteCyclicGroup;
}

packet AMPEMsg //extends Ieee80211ManagementFrame
{  	
    key128 Key_Data128;
    int sec=1;
}
