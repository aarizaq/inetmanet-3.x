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
 * @file OverlayAccess.h
 * @author Ingmar Baumgart
 */

#ifndef __OVERLAY_ACCESS_H__
#define __OVERLAY_ACCESS_H__


#include <omnetpp.h>

#include <BaseOverlay.h>
#include <NotificationBoard.h>

/**
 * Gives access to the overlay.
 */
class OverlayAccess
{
public:
    
    BaseOverlay* get
        (cModule* refMod)
    {
        // obtains the overlay related to the module, taking in account the index in case of SMOHs
        BaseOverlay *overlay = NULL;
        cModule *tmpMod = refMod;
        cModule *tmpParent = NULL; // parent of tmpMod

        // go up from refMod until we get a NotificationBoard module, then we're at root
        // this will fail if the overlay is not in a container module!
        while (true) {
            tmpParent = tmpMod->getParentModule(); // get parent
            // search for a "notificationBoard" module
            cModule *notBoard = tmpParent->getSubmodule("notificationBoard"); 
            // is this a real NotificationBoard? then we're at root
            if (dynamic_cast<NotificationBoard*>(notBoard) != NULL) break; 
            tmpMod = tmpParent; // else keep going up
            if (!tmpParent) throw cRuntimeError("OverlayAccess::get(): Overlay module not found!");
        }
        // get the overlay container, with the proper index
        cModule *overlayContainer = tmpParent->getSubmodule("overlay", tmpMod->getIndex()); 
        overlay = dynamic_cast<BaseOverlay*>
                (overlayContainer->gate("appIn")->getNextGate()->getOwnerModule()); // get the contained overlay module
        
        if (!overlay) throw cRuntimeError("OverlayAccess::get(): Overlay module not found!");
        
        return overlay;
    }
};

#endif
