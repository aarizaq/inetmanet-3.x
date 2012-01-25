//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file CoordBasedRoutingAccess.h
 * @author Bernhard Heep
 */

#ifndef __COORDBASEDROUTING_ACCESS_H__
#define __COORDBASEDROUTING_ACCESS_H__

#include <omnetpp.h>
#include "CoordBasedRouting.h"


/**
 * Gives access to the CoordBasedRouting.
 */
class CoordBasedRoutingAccess
{
public:
    CoordBasedRouting* get()
    {
        CoordBasedRouting* temp =
            (CoordBasedRouting*)simulation.getModuleByPath("globalObserver."
                "globalFunctions[0].function.coordBasedRouting");
        if (temp) return temp;
        return (CoordBasedRouting*)simulation.getModuleByPath("globalObserver."
                "globalFunctions[1].function.coordBasedRouting"); //TODO
    }
};

#endif

