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
 * @file GiaKeyListModule.h
 * @author Robert Palmer
 */

#ifndef __GIAKEYLISTMODULE_H_
#define __GIAKEYLISTMODULE_H_

#include <omnetpp.h>

#include <InitStages.h>

#include "GiaKeyList.h"


/**
 * This class is only for visualizing the KeyList
 */
class GiaKeyListModule : public cSimpleModule
{
  public:
    // OMNeT++ methodes

    /**
     * Sets init stage 
     */
    virtual int numInitStages() const
    {
        return MAX_STAGE_OVERLAY + 1;
    }

    /**
     * Initializes this class and set some WATCH(variable) for OMNeT++
     * @param stage Level of initialization (OMNeT++)
     */
    virtual void initialize(int stage);

    /**
     * This module doesn't handle OMNeT++ messages
     * @param msg OMNeT++ message
     */
    virtual void handleMessages(cMessage* msg);

    /**
     * Sets keyListVector for OMNeT++ WATCH_VECTOR
     * @param keyListVector Vector of search keys
     */
    virtual void setKeyListVector(const std::vector<OverlayKey>& keyListVector);

  protected:
    std::vector<OverlayKey> keyListVector;
};

#endif
