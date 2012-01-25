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
 * @file SingleHostUnderlayConfigurator.h
 * @author Stephan Krause
 * @author Ingmar Baumgart
 */

#ifndef __SINGLEHOSTUNDERLAYCONFIGURATOR_H__
#define __SINGLEHOSTUNDERLAYCONFIGURATOR_H__


#include <omnetpp.h>
#include <BasicModule.h>

#include <UnderlayConfigurator.h>
#include <InitStages.h>


class SingleHostUnderlayConfigurator : public UnderlayConfigurator
{
protected:

    void initializeUnderlay(int stage);
    void finishUnderlay();
    void setDisplayString();
    void handleTimerEvent(cMessage* msg);

    // Not used here:
    TransportAddress* createNode(NodeType type, bool initialize) {error("createNode can't be used with singleHostUnderlay!"); return NULL;}
    void preKillNode(NodeType type, TransportAddress* addr=NULL) {error("preKillNode can't be used with singleHostUnderlay!");}
    void migrateNode(NodeType type, TransportAddress* addr=NULL) {error("migrateNode can't be used with singleHostUnderlay!");}
};


#endif
