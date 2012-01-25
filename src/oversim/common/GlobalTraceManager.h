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
 * @file GlobalTraceManager.h
 * @author Stephan Krause, Ingmar Baumgart
 */

#ifndef __GLOBALTRACEMANAGER_H_
#define __GLOBALTRACEMANAGER_H_

#include <omnetpp.h>

class UnderlayConfigurator;
class GlobalNodeList;

/**
 * Parse a trace file and schedule node joins/leaves according to
 * trace data. If trace includes user action, send actions to application
 */

class GlobalTraceManager : public cSimpleModule
{
  public:
    GlobalTraceManager();
    ~GlobalTraceManager();
    void handleMessage(cMessage* msg);
    void initialize(int stage);

  protected:
    void readNextBlock();
    void scheduleNextEvent(double time, int nodeId, char* buf, int line);
    void createNode(int nodeId);
    void deleteNode(int nodeId);
    cGate* getAppGateById(int nodeId);
    UnderlayConfigurator* underlayConfigurator;
    GlobalNodeList* globalNodeList; /**< pointer to GlobalNodeList */

  private:
    int fd, filesize, chunksize, remain, marginsize, offset;
    char* buf, *start;
    static const int readPages = 32; // Map tracefiles in chunks of 32 pages (i.e. 128k on intel x86)
    // MUST be bigger than 2!

    cMessage* nextRead;
};

#endif
