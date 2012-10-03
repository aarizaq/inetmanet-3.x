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

#include <errno.h>
#include "DCTDump.h"
#include "IPv4Serializer.h"

#define MAXBUFLENGTH 65536
#define MAXDCTLENGTH 100

Define_Module(DCTDump);

DCTDump::DCTDump() {
    // TODO Auto-generated constructor stub

}

DCTDump::~DCTDump() {
    // TODO Auto-generated destructor stub
}

void DCTDump::initialize() {
    const char *file = this->par("dumpFile");
    const char *first_line = "Session Transcript\n";

    // TODO Real timestamp
    const char *second_line = "February 21, 2006     16:45:20.5186\n";
    if (strcmp(file,"")!=0)
    {
        dumpfile = fopen(file, "wb");
        if (!dumpfile)
        {
            fprintf(stderr, "Cannot open file [%s] for writing: %s\n", file, strerror(errno));
            exit(-1);
        }

        fwrite(first_line, 19, 1, dumpfile);
        fwrite(second_line, 36, 1, dumpfile);
    }
    else
        dumpfile = NULL;

}

void DCTDump::handleMessage(cMessage *msg) {

    if (dumpfile != NULL) {
        const simtime_t stime = simulation.getSimTime();
        std::stringstream vers;
        const char *time = timestamp(stime);
        uint8 buf[MAXBUFLENGTH];
        int32 buf_len = 0;
        std::string ascii_buf;
        char dh[MAXDCTLENGTH];
        char *p = dh;
        bool write = false;

        // Write dump
        memset((void*)&buf, 0, sizeof(buf));

        // Write dct2000 header
        memset((void*)&dh, 0, sizeof(dh));

        // context name
        IPv4Datagram *ipPacket = dynamic_cast<IPv4Datagram*>(msg);
        if (ipPacket) {
            if (ipPacket->getTransportProtocol()==IP_PROT_SCTP) {
                strncpy(p, "SCTP.", 5);
                p += 5;
            }
            write = true;

            buf_len = IPv4Serializer().serialize(ipPacket, buf, sizeof(buf));
        }

        // context port number - always 1
        strncpy(p, "1/", 5);
        p += 2;

        // protocol
        if (ipPacket) {
            vers << ipPacket->getVersion();
            // protocol name
            strncpy(p, "ip/", 3);
            p += 3;
            //protocol version
            strncpy(p, vers.str().c_str(), strlen(vers.str().c_str()));
            p += strlen(vers.str().c_str());
            *p = '/';
            p++;
        }

        // direction
        if (msg->getArrivalGate()->isName("ifIn")) {
            strncpy(p, " r", 2);
        } else
            strncpy(p, " s", 2);
        p += 2;

        // timestamp
        strncpy(p, " tm ", 4);
        p += 4;
        strncpy(p, time, strlen(time));
        p += strlen(time);

        if (write) {
            fwrite(&dh, p - dh, 1, dumpfile);
            dumpPacket(buf, buf_len);
        }
    }
    // forward
    int index = msg->getArrivalGate()->getIndex();
    int32 id = -1;
    if (msg->getArrivalGate()->isName("ifIn"))
        id = findGate("netOut",index);
    else
        id = findGate("ifOut",index);

    send(msg, id);
}

const char *DCTDump::timestamp(simtime_t stime) {
    std::stringstream out;
    out << (int32)stime.dbl();
    out << ".";
    out << (uint32)((stime.dbl() - (int32)stime.dbl())*1000000);
    return out.str().c_str();
}

void DCTDump::finish() {
     if (strcmp(this->par("dumpFile"),"")!=0) {
         fclose(dumpfile);
     }
}

void DCTDump::dumpPacket(uint8 *buf, int32 len) {
    if (len > 0) {
        std::string dump = " $";
        int32 j = 0;
        for (int32 i = 0; i < len * 2; i++) {
            if (((buf[j]) >> (4 * ((i + 1) % 2)) & 0x0f) < 10)
                dump += ((buf[j]) >> (4 * ((i + 1) % 2)) & 0x0f) + 48;
            else
                dump += ((buf[j]) >> (4 * ((i + 1) % 2)) & 0x0f) + 87;
            j = j + i % 2;
        }
        dump += "\n";
        fwrite(dump.c_str(), dump.size(), 1, dumpfile);
    }
}




