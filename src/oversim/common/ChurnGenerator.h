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
 * @file ChurnGenerator.h
 * @author Helge Backhaus
 */

#ifndef __CHURNGENERATOR_H_
#define __CHURNGENERATOR_H_

#define MAX_NODETYPES 100

#include <string>
#include <vector>
#include <omnetpp.h>

#include <InitStages.h>

class UnderlayConfigurator;

/**
 * Enum specifying node properties
 *
 * @author Stephan Krause
 */
class NodeType
{
  public:
    NodeType() : typeID(-1) {};
    int32_t typeID;
    std::string terminalType;
    std::vector<std::string> channelTypesRx, channelTypesTx;
    cObject** context;
};


/**
 * Base class for different churn models
 */

class ChurnGenerator : public cSimpleModule
{
  public:
    virtual int numInitStages() const { return MAX_STAGE_UNDERLAY + 1; }
    virtual void initialize(int stage);
    virtual void initializeChurn() = 0;
    virtual void handleMessage(cMessage* msg) = 0;
    void setNodeType(const NodeType& t) { type = t; }
    const NodeType& getNodeType() { return type; }
    bool init; //!< still in initialization phase?
    int terminalCount; //!< current number of overlay terminals

  protected:
    UnderlayConfigurator* underlayConfigurator;
    virtual void updateDisplayString() = 0;
    int targetOverlayTerminalNum; //!< final number of overlay terminals
    NodeType type; //!< the nodeType this generator is responsible for
};

#endif
