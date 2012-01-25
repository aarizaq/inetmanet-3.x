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
 * @file TraceChurn.h
 * @author Stephan Krause
 */

#ifndef __TRACECHURN_H_
#define __TRACECHURN_H_

#include <ChurnGenerator.h>

#include <oversim_mapset.h>

/**
 * Parse a trace file and schedule node joins/leaves according to
 * trace data. If trace includes user action, send actions to application
 */

class TraceChurn : public ChurnGenerator
{
  public:
    void handleMessage(cMessage* msg);
    void initializeChurn();
    void createNode(int nodeId);
    void deleteNode(int nodeId);
    cGate* getAppGateById(int nodeId);

  protected:
    void updateDisplayString();
    TransportAddress* getTransportAddressById(int nodeId);


  private:
    char *maxTier;

    bool initAddMoreTerminals; //!< true, if we're still adding more terminals in the init phase
    cMessage* nextRead;
    typedef std::pair<TransportAddress*, cGate*> nodeMapEntry;
    UNORDERED_MAP<int, nodeMapEntry*> nodeMap;
};

#endif
