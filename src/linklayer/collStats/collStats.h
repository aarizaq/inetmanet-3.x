/*
 * collStats.h
 *
 *  Created on: 11.10.2011
 *      Author: Eugen Paul
 */

#ifndef collStats_H_
#define collStats_H_

#include "compatibility.h"
#include "AirFrame_m.h"
#include "Ieee80211Frame_m.h"

class collStats {

public:
    ~collStats();

    static collStats *getInstance();
    static void destroy();

    void addCollPacket(AirFrame * airframe);
    void addCollPacketAirFrame(AirFrame* airframe);

    void incUdpDataColl();
    void incProtocolColl();
    void incMacColl();

    long getUdpDataColl();
    long getProtocolColl();
    long getMacColl();

private:
    long udpDataColl;
    long protocolColl;
    long macColl;

    void readPaket(Ieee80211DataFrame* paket);

private:
    static collStats* instance;
    collStats();

};

#endif /* collStats_H_ */
