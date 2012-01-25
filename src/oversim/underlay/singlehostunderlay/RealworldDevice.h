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
 * @file RealworldDevice.h
 * @author Stephan Krause
 */

#ifndef _REALWORLDDEVICE_H_
#define _REALWORLDDEVICE_H_

#include "RealworldConnector.h"

#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"

/**
 * RealworldDevice is a pseudo interface that allows communcation with the real world
 * through the TunOutScheduler
 *
 * WARNING: This does ONLY work with the combination IPv4|UDP|OverlayMessage
 */
class RealworldDevice : public RealworldConnector
{
protected:

    InterfaceEntry *interfaceEntry;  // points into IInterfaceTable

    /** Register the interface in the interface table of the parent
     *
     * \return A pointer to the Interface entry
     */
    InterfaceEntry *registerInterface();

public:
    virtual int numInitStages() const
    {
        return 4;
    }

    /** Initialization of the module.
     * Registers the device at the scheduler and searches for the appropriate payload-parser
     * Will be called automatically at startup
     */
    virtual void initialize(int stage);

};

#endif


