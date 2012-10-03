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

#ifndef DCTDUMP_H_
#define DCTDUMP_H_

#include <omnetpp.h>
#include <assert.h>
#include "TCPDump.h"

/*
 * Module for dumping packets in DCT2000 .out files. The class was
 * created according to Wireshark implementation for reading and
 * parsing DCT2000 .out files. Below some info regarding such a file:
 *
 * out file:
 *  - first line at least "Session Transcript"
 *  - second line, timestamp "February 21, 2006     16:45:20.5186"
 *  - packet:
 *      - context name "test_ETSI" followed by '.'
 *      - port number "1" followed by '/'
 *      - protocol name "isdn_l3" followed by '/'
 *      - protocol variant "1" followed by ','
 *          if outhdr is present else followed by '/'
 *      - direction " s"
 *      - timestamp " tm 17.1505 "
 *      - start of dump "$"
 *      - dump of packet in ascii format (0x03 -> 0x30 0x33)
 *      - new line after each dump
 *
 * Unfortunately there is no info besides Wireshark implementation :(
 */
class DCTDump : public cSimpleModule {
private:
    FILE *dumpfile;
    const char *timestamp(simtime_t stime);
    void dumpPacket(uint8 *buf, int32 len);
public:
    DCTDump();
    virtual ~DCTDump();

    /*
     * Method for initializing the module. During initialization,
     * the dump file will be opened and a specific header for .out
     * files will be written in it.
     */
    virtual void initialize();

    /*
     * Method for handling messages. The incoming message will be
     * processed and written in the dump file, configured before the
     * simulation. Afterwards, the message will be forwarded to the
     * appropriate gate.
     */
    virtual void handleMessage(cMessage *msg);

    /*
     * Method for finishing the module. During finish phase the dump
     * file will be closed.
     */
    virtual void finish();
};

#endif /* DCTDUMP_H_ */
