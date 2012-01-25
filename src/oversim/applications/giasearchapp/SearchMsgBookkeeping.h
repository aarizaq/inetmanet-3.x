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
 * @file SearchMsgBookkeeping.h
 * @author Robert Palmer, Bernhard Heep
 */

#ifndef __SEARCHMSGBOOKKEEPING_H_
#define __SEARCHMSGBOOKKEEPING_H_

#include <map>

#include <omnetpp.h>

#include <OverlayKey.h>

/**
 * struct for average statistical values of search messages and responses
 *
 * @author Bernhard Heep
 */
struct GiaSearchStats
{
    float minDelay; //!< average minimum delay of all response messages
    float maxDelay; //!< average maximum delay of all response messages
    float minHopCount; //!< average minimum hop count of all response messages
    float maxHopCount; //!< average maximum hop count of all response messages
    float responseCount; //!< average number of response messages for all search messages
};

/**
 * Class for bookkeeping sent SEARCH-Messages to gather statistical data
 *
 * @author Robert Palmer
 */
class SearchMsgBookkeeping
{
public:

    /**
     *  structure containing all necessary values to gather statistical data
     */
    struct SearchMessageItem
    {
        OverlayKey searchKey; /**< key to search for */
        simtime_t creationTime; /**< time when message was sent to overlay */
        simtime_t minDelay; /**< minimum delay of an response message for this search message */
        simtime_t maxDelay; /**< maximum delay of an response message for this search message */
        uint32_t minHopCount; /**< minimum hop count of an response message for this search message */
        uint32_t maxHopCount; /**< maximum hop count of an response message for this search message */
        uint32_t responseCount; /**< number of response messages fpr this search message */
    };

    typedef std::map<OverlayKey, SearchMessageItem>
    SearchBookkeepingList; /**< typedef for hashmap of OverlayKey and SearchMessageItem */
    typedef std::map<OverlayKey, SearchMessageItem>::iterator
    SearchBookkeepingListIterator; /**< typedef for an iterator of SearchBookkeepingList */
    typedef std::map<OverlayKey, SearchMessageItem>::const_iterator
    SearchBookkeepingListConstIterator; /**< typedef for an constant iterator of SearchBookkeepingList */

    /**
     * Destructor
     */
    ~SearchMsgBookkeeping();

    /**
     * Returns size of Search-Message-Bookkeeping-List
     *
     * @return Size of SearchMsgBookkeeping-List
     */
    uint32_t getSize() const;

    /**
     * Add SearchMessage to SearchMsgBookkeeping
     *
     * @param searchKey
     */
    void addMessage(const OverlayKey& searchKey);

    /**
     * Removes SearchMessage from SearchMsgBookkeeping
     *
     * @param searchKey
     */
    void removeMessage(const OverlayKey& searchKey);

    /**
     * checks if Search-Message-Bookkeeping-List contains a specified key
     *
     * @param searchKey Key to check
     * @return true, if SearchMsgBookkeeping contains searchKey
     */
    bool contains(const OverlayKey& searchKey) const;

    /**
     * Updates hop-count, min-response-delay, max-response-delay of given searchMessage
     *
     * @param searchKey Id of search message
     * @param hopCount New hopCount-Value
     */
    void updateItem(const OverlayKey& searchKey, uint32_t hopCount);

    /**
     * Returns statistical data
     *
     * @return collected statistical data
     */
    GiaSearchStats getStatisticalData() const;

protected:

    SearchBookkeepingList messages; /**< bookkeeping list of all sent search messages */
};

#endif
