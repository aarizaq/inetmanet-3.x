/*
 * I3Anycast.h
 *
 *  Created on: 24/01/2012
 *      Author: alfonso
 */

#ifndef I3ANYCAST_H_
#define I3ANYCAST_H_
#include "I3BaseApp.h"

class I3Anycast : public I3BaseApp
{
private:
    static int index; //HACK Change to use index module when it's done
public:
    int myIndex;
    cMessage *sendPacketTimer;
    I3Anycast(){}
    ~I3Anycast(){}

    void initializeApp(int stage);
    void initializeI3();
    void deliver(I3Trigger &trigger, I3IdentifierStack &stack, cPacket *msg);
    void handleTimerEvent(cMessage *msg);
};



#endif /* I3ANYCAST_H_ */
