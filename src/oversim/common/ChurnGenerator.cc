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
 * @file ChurnGenerator.cc
 * @author Helge Backhaus
 */

#include "ChurnGenerator.h"
#include <UnderlayConfiguratorAccess.h>

void ChurnGenerator::initialize(int stage)
{
    // because of IPAddressResolver, we need to wait until interfaces are registered,
    // address auto-assignment takes place etc.
    if (stage != MAX_STAGE_UNDERLAY)
        return;

    if (type.typeID == -1) {
        opp_error("NodeType not set when initializing ChurnGenerator");
    }

    underlayConfigurator = UnderlayConfiguratorAccess().get();
    // get desired # of terminals
    targetOverlayTerminalNum = par("targetOverlayTerminalNum");

    type.channelTypesTx = cStringTokenizer(par("channelTypes"), " ").asVector();
    type.channelTypesRx = cStringTokenizer(par("channelTypesRx"), " ").asVector();
    
    if (type.channelTypesRx.size() != type.channelTypesTx.size()) {
        type.channelTypesRx = type.channelTypesTx;
    }
    
    terminalCount = 0;
    init = true;

    updateDisplayString();
    WATCH(terminalCount);

    initializeChurn();
}
