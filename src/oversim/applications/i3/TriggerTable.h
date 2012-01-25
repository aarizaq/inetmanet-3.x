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
 * @file TriggerTable.h
 * @author Antonio Zea
 */


#ifndef __TRIGGERTABLE_H__
#define __TRIGGERTABLE_H__

#include "I3.h"

/** Omnetpp module to wrap around I3's I3TriggerTable.
* Does little more than present the map's values.
*/

struct TriggerTable : public cSimpleModule {
    I3TriggerTable *triggerTable;

    int numInitStages() const;
    void initialize(int stage);
    void updateDisplayString();
};

#endif
