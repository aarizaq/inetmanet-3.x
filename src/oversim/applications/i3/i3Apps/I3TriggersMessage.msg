cplusplus {{
#include "I3Identifier.h"
}}



class noncobject I3Identifier;

packet TriggersHandshakeMsg
{
		int value;
		I3Identifier triggerId;
}

packet TriggersMsg
{
		int value;
}
