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
 * @file PeerInfo.cc
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#include "PeerInfo.h"

PeerInfo::PeerInfo(uint32_t type, int moduleId, cObject** context)
{
    bootstrapped = false;
    malicious = false;
    this->moduleId = moduleId;
    this->type = type;
    this->npsLayer = -1;   // layer not determined yet
    this->context = context;
}

void PeerInfo::dummy() {}

//public

std::ostream& operator<<(std::ostream& os, const PeerInfo info)
{
    os  << "ModuleId: " << info.moduleId << "Bootstrapped: "
        << (info.bootstrapped ? "true" : "false");

    if (info.npsLayer >= 0) os << "; NPS Layer: " << info.npsLayer;
    return os;
}

