class noncobject MACAddress;

cplusplus {{
#include "INETDefs.h"
}}

cplusplus {{

#include "MACAddress.h"
#include "Ieee80211MgmtFrames_m.h"
#include "Security.h"
#include <iostream>
#include <vector>

}}

class noncobject Security;

cplusplus {{
typedef Security::nonce nonce;
typedef Security::key128 key128;
typedef Security::key256 key256;
typedef Security::key384 key384;
typedef Security::mic mic;
typedef Security::unit64_t_ unit64_t_;
}}
class noncobject nonce;
class noncobject key128;
class noncobject key256;
class noncobject key384;
class noncobject mic;
class noncobject std;
class noncobject unit64_t_;
//packet Ieee80211ManagementFrame;
//packet Ieee80211MeshFrame;


packet SecurityPkt //extends Ieee80211ManagementFrame
{
    char Descriptor_Type[12];
   // char Descriptor_Type=0;
    short Key_Info=0;
    short Key_Length=0;
    unit64_t_ Key_RC;
    nonce Key_Nonce;
    key128 EAPOL_KeyIV;
    unit64_t_ Key_RSC=0; //Key Replay Conter
    unit64_t_ Reserved=0;
    key128 Mic;
  	short Key_Data_Length=0;
  	
    key128 Key_Data128;
  	key256 Key_Data256;
}


// Ieee80211DataFrame
packet CCMPFrame //extends Ieee80211MeshFrame
{
    unit64_t_ CCMP_Header; //8 octets
    unit64_t_ CCMP_Mic; 	
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
