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
 * @file GIASearchApp.cc
 * @author Robert Palmer, Bernhard Heep
 */

#include <IPvXAddressResolver.h>

#include <InitStages.h>
#include <CommonMessages_m.h>
#include <ExtAPIMessages_m.h>
#include <GiaMessage_m.h>
#include <GlobalStatistics.h>

#include "GIASearchApp.h"


Define_Module(GIASearchApp);

GIASearchApp::GIASearchApp()
{
    search_timer = keyList_timer = NULL;
    srMsgBook = NULL;
}

GIASearchApp::~GIASearchApp()
{
    cancelAndDelete(search_timer);
    cancelAndDelete(keyList_timer);
    if (srMsgBook != NULL) {
	delete srMsgBook;
	srMsgBook = NULL;
    }
}

void GIASearchApp::initializeApp(int stage)
{
    if (stage != MIN_STAGE_APP)
        return;

    // fetch parameters
    mean = par("messageDelay");
    deviation = mean / 3;
    randomNodes = par("randomNodes");
    maxResponses = par("maxResponses");

    srMsgBook = new SearchMsgBookkeeping();

    // statistics
    stat_keyListMessagesSent = 0;
    stat_keyListBytesSent = 0;
    stat_searchMessagesSent = 0;
    stat_searchBytesSent = 0;
    stat_searchResponseMessages = 0;
    stat_searchResponseBytes = 0;

    // initiate test message emision
    search_timer = new cMessage("search_timer");
    scheduleAt(simTime() + truncnormal(mean, deviation),
	       search_timer);

    keyList_timer = new cMessage("keyList_timer");
    scheduleAt(simTime() + uniform(0.0, 10.0), keyList_timer);
}

void GIASearchApp::handleTimerEvent(cMessage *msg)
{
    if(msg == keyList_timer) {
        keyList = globalNodeList->getKeyList(par("maximumKeys").longValue());
        WATCH_VECTOR(*keyList);

        // create message
        GIAput* putMsg = new GIAput("GIA-Keylist");
        putMsg->setCommand(GIA_PUT);

        putMsg->setKeysArraySize(keyList->size());

        std::vector<OverlayKey>::iterator it;
        int k;
        for(it = keyList->begin(), k = 0; it != keyList->end(); it++, k++)
            putMsg->setKeys(k, *it);

        putMsg->setBitLength(GIAPUT_L(putMsg));

        sendMessageToLowerTier(putMsg);

        if (debugOutput)
           EV << "[GIASearchApp::handleTimerEvent() @ " << overlay->getThisNode().getIp()<< "]\n"
              << "    Node sent keylist to overlay."
              << endl;

        stat_keyListMessagesSent++;
        stat_keyListBytesSent += putMsg->getByteLength();
    }
    else if(msg == search_timer) {
        // schedule next search-message
        scheduleAt(simTime() + truncnormal(mean, deviation), msg);

        // do nothing, if the network is still in the initiaization phase
        if((!par("activeNetwInitPhase")) && (underlayConfigurator->isInInitPhase()))
            return;

        OverlayKey keyListItem;
        uint32_t maximumTries = 20;
        // pic a search key we are not already searching
        do {
            if (maximumTries-- == 0)
                break;
            keyListItem = globalNodeList->getRandomKeyListItem();
        } while ((keyListItem.isUnspecified())
		 && ((srMsgBook->contains(keyListItem))));

        if (!keyListItem.isUnspecified()) {
            // create message
            GIAsearch* getMsg = new GIAsearch("GIA-Search");
            getMsg->setCommand(GIA_SEARCH);
            getMsg->setSearchKey(keyListItem);
            getMsg->setMaxResponses(maxResponses);
            getMsg->setBitLength(GIAGET_L(getMsg));

            sendMessageToLowerTier(getMsg);

            // add search key to our bookkeeping list
            srMsgBook->addMessage(keyListItem);

            if (debugOutput)
                EV << "[GIASearchApp::handleTimerEvent() @ " << overlay->getThisNode().getIp()<< "]\n"
                   << "    Node sent get-message to overlay."
                   << endl;

            stat_searchMessagesSent++;
            stat_searchBytesSent += getMsg->getByteLength();
        }
    }
}

void GIASearchApp::handleLowerMessage(cMessage* msg)
{
    GIAanswer* answer = check_and_cast<GIAanswer*>(msg);
    OverlayCtrlInfo* overlayCtrlInfo =
        check_and_cast<OverlayCtrlInfo*>(answer->removeControlInfo());

    OverlayKey searchKey = answer->getSearchKey();

    if (debugOutput)
        EV << "[GIASearchApp::handleLowerMessage() @ " << overlay->getThisNode().getIp()<< "]\n"
           << "    Node received answer-message from overlay:"
           << " (key: " << searchKey
           << " at node " << answer->getNode() << ")"
           << endl;

    stat_searchResponseMessages++;
    stat_searchResponseBytes += answer->getByteLength();

    if (srMsgBook->contains(searchKey))
        srMsgBook->updateItem(searchKey, overlayCtrlInfo->getHopCount());

    delete answer;
}

void GIASearchApp::finishApp()
{
    // record scalar data
    GiaSearchStats stats = srMsgBook->getStatisticalData();

    if (stats.minDelay == 0 &&
        stats.maxDelay == 0 &&
        stats.minHopCount == 0 &&
        stats.maxHopCount == 0 &&
        stats.responseCount == 0) return;

	globalStatistics->addStdDev("GIASearchApp: SearchMsg avg. min delay",
	                            stats.minDelay);
    globalStatistics->addStdDev("GIASearchApp: SearchMsg avg. max delay",
                                stats.maxDelay);
    globalStatistics->addStdDev("GIASearchApp: SearchMsg avg. min hops",
                                stats.minHopCount);
    globalStatistics->addStdDev("GIASearchApp: SearchMsg avg. max hops",
                                stats.maxHopCount);
    globalStatistics->addStdDev("GIASearchApp: SearchMsg avg. response count",
                                stats.responseCount);
}
