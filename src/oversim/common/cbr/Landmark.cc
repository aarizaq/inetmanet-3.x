//
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
 * @file Landmark.cc
 * @author Fabian Hartmann, Bernhard Heep
 */

#include <cassert>

#include <SimpleUnderlayConfigurator.h>
#include <BootstrapList.h>
#include <NeighborCache.h>
#include <GlobalNodeList.h>
#include <GlobalStatistics.h>

#include "Landmark.h"

Define_Module(Landmark);

Landmark::~Landmark() {
}

void Landmark::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    SimpleNodeEntry* entry =
        dynamic_cast<SimpleInfo*>(globalNodeList->
                                  getPeerInfo(thisNode.getIp()))->getEntry();

    // Get the responsible Landmark churn generator
    /*
    ChurnGenerator* lmChurnGen = NULL;
    for (uint8_t i = 0; i < underlayConfigurator->getChurnGeneratorNum(); i++) {
        ChurnGenerator* searchedGen;
        searchedGen = underlayConfigurator->getChurnGenerator(i);
        if (searchedGen->getNodeType().overlayType == "oversim.common.cbr.LandmarkModules") {
            lmChurnGen = searchedGen;
        }
    }
    */

    if (true) { //TODO
        // magic placement using underlays coords
        std::vector<double> ownCoords;
        for (uint8_t i = 0; i < entry->getDim(); i++) {
            ownCoords.push_back(entry->getCoords(i));
        }

        Nps& nps = (Nps&)neighborCache->getNcsAccess();
        nps.setOwnCoordinates(ownCoords);
        nps.setOwnLayer(0);

        thisNode = overlay->getThisNode();
        globalNodeList->setOverlayReadyIcon(getThisNode(), true);
        globalNodeList->refreshEntry(getThisNode());
    } else {
        //TODO
    }
}

void Landmark::finishApp()
{
    if (((Nps&)(neighborCache->getNcsAccess())).getReceivedCalls() != 0) {
        globalStatistics->recordOutVector("Calls to Landmarks",
            ((Nps&)(neighborCache->getNcsAccess())).getReceivedCalls());
    }
}

