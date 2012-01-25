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
 * @file GIASearchApp.h 
 * @author Robert Palmer 
 */

#ifndef __GIASEARCHAPP_H_
#define __GIASEARCHAPP_H_

#include <set>

#include <omnetpp.h>

#include <BaseApp.h>
#include <GlobalNodeList.h>
#include <UnderlayConfigurator.h>
#include <OverlayKey.h>
#include "SearchMsgBookkeeping.h"


/**
 * Gia search test application
 * 
 * Gia search test application, sends periodically SEARCH-Messages and collects statistical data. 
 *
 * @see BaseApp
 */
class GIASearchApp : public BaseApp
{
  public:
    GIASearchApp();
    virtual ~GIASearchApp();

private:

    std::vector<OverlayKey>* keyList; /**< list of all maintained key of this application */

protected:

    /**
     * initializes base class-attributes
     *
     * @param stage the init stage
     */
    virtual void initializeApp(int stage);

    void handleLowerMessage(cMessage* msg);

    virtual void handleTimerEvent(cMessage *msg);

    /**
     * collects statistical data
     */
    virtual void finishApp();

    SearchMsgBookkeeping* srMsgBook; /**< pointer to Search-Message-Bookkeeping-List in this node */

    // parameters
    double mean; /**< mean interval for next message */
    double deviation; /**< deviation of mean interval */
    bool randomNodes; /**< use random destination nodes or only nodes from GlobalNodeList? */
    int maxResponses; /**< maximum number of responses per search message */

    // message field lengths (bit)
    static const uint32_t ID_L = 16;
    static const uint32_t SEQNUM_L = 16;
    int msgByteLength;

    // statistics
    int stat_keyListMessagesSent; /**< number of keyList-Messages sent */
    int stat_keyListBytesSent; /**< number of keyList-Bytes sent */
    int stat_searchMessagesSent; /**< number of search-Messages sent */
    int stat_searchBytesSent; /**< number of search-Messages-Bytes sent */
    int stat_searchResponseMessages; /**< number of received search-Response-Messages */
    int stat_searchResponseBytes; /**< number of received search-Response-Messages-Bytes */

    cMessage* search_timer; //!< timer for search messages
    cMessage* keyList_timer; //!< timer for initial key list packet to overlay
};

#endif
