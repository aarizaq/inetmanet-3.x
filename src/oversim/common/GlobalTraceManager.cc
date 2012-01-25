//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
 * @file GlobalTraceManager.cc
 * @author Stephan Krause, Ingmar Baumgart
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif

#include <UnderlayConfiguratorAccess.h>
#include <GlobalNodeListAccess.h>
#include <GlobalTraceManager_m.h>
#include <TraceChurn.h>

#include "GlobalTraceManager.h"

Define_Module(GlobalTraceManager);

void GlobalTraceManager::initialize(int stage)
{
    Enter_Method_Silent();

    // Nothing to do for us if there is no traceFile
    if (strlen(par("traceFile")) == 0)
        return;

    underlayConfigurator = UnderlayConfiguratorAccess().get();
    globalNodeList = GlobalNodeListAccess().get();

    offset = 0;

    // Open file and get size
    fd = open(par("traceFile"), O_RDONLY);

    if (!fd) {
        throw cRuntimeError(("Can't open file " + par("traceFile").stdstringValue() +
                std::string(strerror(errno))).c_str());
    }

    struct stat filestat;

    if (fstat(fd, &filestat)) {
        throw cRuntimeError(("Error calling stat: " + std::string(strerror(errno))).c_str());
    }

    filesize = filestat.st_size;
    remain = filesize;
    EV << "[GlobalTraceManager::initialize()]\n"
       << "    Successfully opened trace file " << par("traceFile").stdstringValue()
       << ". Size: " << filesize
       << endl;

    nextRead = new cMessage("NextRead");
    scheduleAt(0, nextRead);

}

GlobalTraceManager::GlobalTraceManager()
{
    nextRead = NULL;
}

GlobalTraceManager::~GlobalTraceManager()
{
    cancelAndDelete(nextRead);
}

void GlobalTraceManager::readNextBlock()
{
#ifdef _WIN32
	// TODO: port for MINGW
	throw cRuntimeError("GlobalTraceManager::readNextBlock():"
			"Not available on WIN32 yet!");
#else
	double time = -1;
    int nodeID;
    char* bufend;
    int line = 1;

    if (remain > 0) {
        // If rest of the file is bigger than maximal chunk size, set chunksize
        // to max; the last mapped page is used as margin.
        // Else map the whole remainder of the file at a time, no margin is needed
        // in this case
        chunksize = remain < getpagesize()*readPages ? remain : getpagesize()*readPages;
        marginsize = remain == chunksize ? 0 : getpagesize();

        start = (char*) mmap(0, chunksize, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, filesize - remain);

        if (start == MAP_FAILED) {
            throw cRuntimeError(("Error mapping file to memory:" +
                    std::string(strerror(errno))).c_str());
        }

        buf = start + offset;
        // While the read pointer has not reached the margin, continue parsing
        while( buf < start + chunksize - marginsize) { // FIXME: Assuming max line length of getpagesize()

            time = strtod(buf, &bufend);
            if (bufend==buf) {
                throw cRuntimeError("Error parsing file: Expected time as double");
            }
            buf = bufend;

            nodeID = strtol(buf, &bufend, 0);
            if (bufend==buf) {
                throw cRuntimeError("Error parsing file: Expected ID as long int");
            }
            buf = bufend;

            while( isspace(buf[0]) ) buf++;

            bufend = strchr(buf, '\n');
            if (!bufend) {
                throw cRuntimeError("Error parsing file: Missing command or no newline at end of line!");
            }
            bufend[0]='\0';
            scheduleNextEvent(time, nodeID, buf, line++);
            buf += strlen(buf)+1;

            while( isspace(buf[0]) ) buf++;
        }

        // Compute offset for the next read
        offset = (buf - start) - (chunksize - marginsize);
        remain -= chunksize - marginsize;
        munmap( start, chunksize );
    }

    if (time > 0) {
        scheduleAt(time, nextRead);
    } else {
        // TODO: Schedule simulation end?
    }
#endif
}

void GlobalTraceManager::scheduleNextEvent(double time, int nodeId, char* buf, int line)
{
    GlobalTraceManagerMessage* msg = new GlobalTraceManagerMessage(buf);

    msg->setInternalNodeId(nodeId);
    msg->setLineNumber(line);
    scheduleAt(time, msg);
}

void GlobalTraceManager::createNode(int nodeId)
{
    check_and_cast<TraceChurn*>(underlayConfigurator->getChurnGenerator(0))
                                                            ->createNode(nodeId);
}

void GlobalTraceManager::deleteNode(int nodeId)
{
    // TODO: calculate proper deletion time with *.underlayConfigurator.gracefulLeaveDelay
    check_and_cast<TraceChurn*>(underlayConfigurator->getChurnGenerator(0))
                                                            ->deleteNode(nodeId);
}

void GlobalTraceManager::handleMessage(cMessage* msg)
{
    if (!msg->isSelfMessage()) {
        delete msg;
        return;
    } else if ( msg == nextRead ) {
        readNextBlock();
        return;
    }

    GlobalTraceManagerMessage* traceMsg = check_and_cast<GlobalTraceManagerMessage*>(msg);

    if (strstr(traceMsg->getName(), "JOIN") == traceMsg->getName()) {
        createNode(traceMsg->getInternalNodeId());
    } else if (strstr(traceMsg->getName(), "LEAVE") == traceMsg->getName()) {
        deleteNode(traceMsg->getInternalNodeId());
    } else if (strstr(traceMsg->getName(), "CONNECT_NODETYPES") == traceMsg->getName()) {
        std::vector<std::string> strVec = cStringTokenizer(msg->getName()).asVector();

        if (strVec.size() != 3) {
            throw cRuntimeError("GlobalTraceManager::"
                                 "handleMessage(): Invalid command");
        }

        int firstNodeType = atoi(strVec[1].c_str());
        int secondNodeType = atoi(strVec[2].c_str());

        globalNodeList->connectNodeTypes(firstNodeType, secondNodeType);
    } else if (strstr(traceMsg->getName(), "DISCONNECT_NODETYPES") == traceMsg->getName()) {
        std::vector<std::string> strVec = cStringTokenizer(msg->getName()).asVector();

        if (strVec.size() != 3) {
            throw cRuntimeError("GlobalTraceManager::"
                                 "handleMessage(): Invalid command");
        }

        int firstNodeType = atoi(strVec[1].c_str());
        int secondNodeType = atoi(strVec[2].c_str());

        globalNodeList->disconnectNodeTypes(firstNodeType, secondNodeType);
    } else if (strstr(traceMsg->getName(), "MERGE_BOOTSTRAPNODES") == traceMsg->getName()) {
        std::vector<std::string> strVec = cStringTokenizer(msg->getName()).asVector();

        if (strVec.size() != 4) {
            throw cRuntimeError("GlobalTraceManager::"
                                 "handleMessage(): Invalid command");
        }

        int toPartition = atoi(strVec[1].c_str());
        int fromPartition = atoi(strVec[2].c_str());
        int numNodes = atoi(strVec[3].c_str());

        globalNodeList->mergeBootstrapNodes(toPartition, fromPartition, numNodes);
    } else {
        sendDirect(msg, getAppGateById(traceMsg->getInternalNodeId()));
        return; // don't delete Message
    }

    delete msg;
}

cGate* GlobalTraceManager::getAppGateById(int nodeId) {
    return check_and_cast<TraceChurn*>(underlayConfigurator->getChurnGenerator(0))
                                                                ->getAppGateById(nodeId);
}

