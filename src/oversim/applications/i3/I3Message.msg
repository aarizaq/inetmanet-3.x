cplusplus {{
#include "I3Trigger.h"
#include "I3IdentifierStack.h"
#include "I3Identifier.h"
#include "I3IPAddress.h"

#define I3TYPE_L    2
#define I3BOOL_L    1
#define I3TIME_L    32
#define INSERT_TRIGGER_L(msg) (I3TYPE_L + I3BOOL_L + msg->getTrigger().length() + msg->getSource().length())
#define QUERY_REPLY_L(msg)  (I3TYPE_L + msg->getSource().length() + msg->getIdentifier().length() + I3TIME_L)
#define REMOVE_TRIGGER_L(msg) (I3TYPE_L + msg->getTrigger().length())
#define SEND_PACKET_L(msg)    (I3TYPE_L + msg->getIdentifierStack().length() + msg->getMatchedTrigger().length())

}}



class noncobject I3Trigger;

class noncobject I3IdentifierStack;

class noncobject I3Identifier;

class noncobject I3IPAddress;

class BaseAppMessage;



enum I3MessageType
{

	INSERT_TRIGGER = 0;
	QUERY_REPLY = 1;	
	REMOVE_TRIGGER = 2;
	SEND_PACKET = 3;
}

packet I3Message
{
		int type enum(I3MessageType);
}

packet I3InsertTriggerMessage extends I3Message
{
    @customize(true);
		bool sendReply;
		I3Trigger trigger;
		I3IPAddress source;
}

packet I3QueryReplyMessage extends I3Message
{
    @customize(true);
		I3IPAddress source;
		I3Identifier identifier;
		simtime_t sendingTime;
}

packet I3RemoveTriggerMessage extends I3Message
{
    @customize(true);
		I3Trigger trigger;
}

packet I3SendPacketMessage extends I3Message
{
    @customize(true);
		I3IdentifierStack identifierStack;
		I3Trigger matchedTrigger;
		bool sendReply;
		I3IPAddress source;
}

