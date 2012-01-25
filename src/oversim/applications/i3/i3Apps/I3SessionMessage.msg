cplusplus {{
#include "I3Identifier.h"
}}



class noncobject I3Identifier;

enum SessionMsgType
{

     PAYLOAD = 0;
     CHANGE_SESSION = 1;
     TRIGGER_CONFIRMATION = 2;
}
	
packet SessionMsg
{
		int type enum(SessionMsgType);
		double value;
                I3Identifier source;
}