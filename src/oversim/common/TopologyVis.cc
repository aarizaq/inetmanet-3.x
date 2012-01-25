// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file TopologyVis.cc
 * @author Bernhard Heep
 */

#include <omnetpp.h>

#include <NodeHandle.h>
#include <GlobalNodeList.h>
#include <GlobalNodeListAccess.h>
#include <PeerInfo.h>

#include "TopologyVis.h"


TopologyVis::TopologyVis()
{
    thisTerminal = NULL;
    globalNodeList = NULL;
}

void TopologyVis::initVis(cModule* terminal)
{
    thisTerminal = terminal;
    globalNodeList = GlobalNodeListAccess().get();

    // set up arrow-gates
//    thisOutGateArray = thisTerminal->gate("overlayNeighborArrowOut");
//    thisInGateArray = thisTerminal->gate("overlayNeighborArrowIn");
    thisTerminal->setGateSize("overlayNeighborArrowOut", 1);
    thisTerminal->setGateSize("overlayNeighborArrowIn", 1);

}

void TopologyVis::showOverlayNeighborArrow(const NodeHandle& neighbor,
                                           bool flush, const char* displayString)
{
    if (!ev.isGUI() || !thisTerminal)
        return;

    char red[] = "ls=red,1";

    if (displayString == NULL)
        displayString = red;

    cModule* neighborTerminal;

    // flush
    if (flush) {
        for (int l = 0; l < thisTerminal->gateSize("overlayNeighborArrowOut"); l++) {
            cGate* tempGate =
                thisTerminal->gate("overlayNeighborArrowOut", l)->getNextGate();

            thisTerminal->gate("overlayNeighborArrowOut", l)->disconnect();
            if (tempGate != NULL)
                compactGateArray(tempGate->getOwnerModule(), VIS_IN);
        }
        thisTerminal->setGateSize("overlayNeighborArrowOut" ,0);
    }

    if (globalNodeList->getPeerInfo(neighbor) == NULL)
        return;

    neighborTerminal = simulation.getModule(globalNodeList->
            getPeerInfo(neighbor)->getModuleID());

    if (neighborTerminal == NULL)
        return;

    if (thisTerminal == neighborTerminal)
        return;

    //do not draw double
    for (int i = 0; i < thisTerminal->gateSize("overlayNeighborArrowOut"); i++)
        if (thisTerminal->gate("overlayNeighborArrowOut", i)
                ->getNextGate() != NULL &&
                neighborTerminal == thisTerminal
                    ->gate("overlayNeighborArrowOut", i)
                    ->getNextGate()->getOwnerModule())
            return;

    // IN
    int i = 0;
    if (neighborTerminal->gateSize("overlayNeighborArrowIn") == 0) {
        neighborTerminal->setGateSize("overlayNeighborArrowIn", 1);
    } else {
        for (i = 0; i < neighborTerminal->gateSize("overlayNeighborArrowIn") - 1; i++) {
            if (!(neighborTerminal->gate("overlayNeighborArrowIn", i)
                    ->isConnectedOutside()))
                break;
        }
        if (neighborTerminal->gate("overlayNeighborArrowIn", i)
                ->isConnectedOutside()) {
            neighborTerminal->setGateSize("overlayNeighborArrowIn", i + 2);
            i++;
        }
    }

    // OUT
    int j = 0;
    if (thisTerminal->gateSize("overlayNeighborArrowOut") == 0)
        thisTerminal->setGateSize("overlayNeighborArrowOut", 1);
    else {
        for (j = 0; j < (thisTerminal->gateSize("overlayNeighborArrowOut") - 1); j++) {
            if (!(thisTerminal->gate("overlayNeighborArrowOut", j)
                    ->isConnectedOutside()))
                break;
        }
        if (thisTerminal->gate("overlayNeighborArrowOut", j)
                ->isConnectedOutside()) {
            thisTerminal->setGateSize("overlayNeighborArrowOut", j + 2);
            j++;
        }
    }

    thisTerminal->gate("overlayNeighborArrowOut", j)->connectTo(neighborTerminal->gate("overlayNeighborArrowIn", i));

    thisTerminal->gate("overlayNeighborArrowOut", j)->setDisplayString(displayString);
}

void TopologyVis::deleteOverlayNeighborArrow(const NodeHandle& neighbor)
{
    if (!ev.isGUI() || !thisTerminal)
        return;

    PeerInfo* peerInfo = globalNodeList->getPeerInfo(neighbor);
    if (peerInfo == NULL) {
        return;
    }

    cModule* neighborTerminal = simulation.getModule(peerInfo->getModuleID());
    if (neighborTerminal == NULL) {
        return;
    }

    //find gate
    bool compactOut = false;
    bool compactIn = false;
    for (int i = 0; i < thisTerminal->gateSize("overlayNeighborArrowOut"); i++) {
        // NULL-Gate?
        if (thisTerminal->gate("overlayNeighborArrowOut", i)
                ->getNextGate() == NULL) {
            compactOut = true;
            continue;
        }

        if (thisTerminal->gate("overlayNeighborArrowOut", i)
                ->getNextGate()->getOwnerModule()->getId() == neighborTerminal->getId()) {
            thisTerminal->gate("overlayNeighborArrowOut", i)->disconnect();
            compactOut = true;
            compactIn = true;
        }
    }

    //compact OUT-array
    if (compactOut)
        compactGateArray(thisTerminal, VIS_OUT);
    //compact IN-array
    if (compactIn)
        compactGateArray(neighborTerminal, VIS_IN);
}

void TopologyVis::compactGateArray(cModule* terminal,
                                   enum VisDrawDirection dir)
{
    const char* gateName = (dir == VIS_OUT ? "overlayNeighborArrowOut"
                                         : "overlayNeighborArrowIn");

    for (int j = 0; j < terminal->gateSize(gateName) - 1; j++) {
        if (terminal->gate(gateName, j)->isConnectedOutside())
            continue;

        cGate* tempGate = NULL;
        int k = 1;
        while ((tempGate == NULL) && ((j + k) != terminal->gateSize(gateName))) {
            tempGate = (dir == VIS_OUT ?
                        terminal->gate(gateName, j + k)->getNextGate() :
                        terminal->gate(gateName, j + k)->getPreviousGate());
            k++;
        }

        if (tempGate == NULL)
            break;

        cDisplayString tempDisplayStr;
        if (dir == VIS_OUT) {
            tempDisplayStr = terminal->gate(gateName, j + k - 1)->getDisplayString();
            terminal->gate(gateName, j + k - 1)->disconnect();
            terminal->gate(gateName, j)->connectTo(tempGate);
            terminal->gate(gateName, j)->setDisplayString(tempDisplayStr.str());
        } else {
            tempDisplayStr = tempGate->getDisplayString();
            tempGate->disconnect();
            tempGate->connectTo(terminal->gate(gateName, j));
            tempGate->setDisplayString(tempDisplayStr.str());
        }
    }

    int nullGates = 0;
    for (int j = 0; j < terminal->gateSize(gateName); j++)
        if (!terminal->gate(gateName, j)->isConnectedOutside())
            nullGates++;

    terminal->setGateSize(gateName, terminal->gateSize(gateName) - nullGates);
}
