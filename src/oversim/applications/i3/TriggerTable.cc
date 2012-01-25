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
 * @file TriggerTable.cc
 * @author Antonio Zea
 */

#include "TriggerTable.h"
#include <string>
#include <sstream>

using namespace std;

Define_Module(TriggerTable);

int TriggerTable::numInitStages() const
{
    return 6;
}

void TriggerTable::initialize(int stage)
{
    if (stage != 5) return;

    I3 *i3 = check_and_cast<I3*>(getParentModule()->getSubmodule("i3"));
    triggerTable = &i3->getTriggerTable();
    WATCH_MAP(*triggerTable);
    getDisplayString().setTagArg("t", 0, "0 identifiers,\n0 triggers");
}


void TriggerTable::updateDisplayString()
{
    ostringstream os;
    int numTriggers = 0;

    os << triggerTable->size() << " identifiers,\n";

    I3TriggerTable::iterator it;
    for (it = triggerTable->begin(); it != triggerTable->end(); it++) {
        numTriggers += it->second.size();
    }

    os << numTriggers << " triggers";

    getDisplayString().setTagArg("t", 0, os.str().c_str());
}
