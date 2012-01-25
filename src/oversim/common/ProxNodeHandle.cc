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
 * @file ProxNodeHandle.cc
 * @author Bernhard Heep
 */


#include "ProxNodeHandle.h"

// This is the usual value for SimTIme::getMaxTime(), may change with a different SimTime scale.
// This value is declared directly a constant, since SimTime::getMaxTime()
// isn't set yet when the program starts.
#define MAXTIME_DBL 9223372036.854775807

const Prox Prox::PROX_SELF(0, 1);
const Prox Prox::PROX_UNKNOWN(MAXTIME_DBL, 0);
const Prox Prox::PROX_TIMEOUT(MAXTIME_DBL, 1);
//const Prox Prox::PROX_WAITING = {MAXTIME_DBL, 0.6};

Prox::operator double() { return proximity; };
Prox::operator simtime_t() { return (proximity >= MAXTIME_DBL)
                                   ? MAXTIME : proximity; };

Prox::Prox() {}
Prox::Prox(simtime_t prox) : proximity(SIMTIME_DBL(prox)), accuracy(1) {}
Prox::Prox(simtime_t prox, double acc) : proximity(SIMTIME_DBL(prox)), accuracy(acc) {}
Prox::Prox(double prox, double acc) : proximity(prox), accuracy(acc) {}

bool Prox::operator==(Prox p) const { return proximity == p.proximity && accuracy == p.accuracy; }
bool Prox::operator!=(Prox p) const { return !(*this == p); }


// predefined node handle
const ProxNodeHandle ProxNodeHandle::UNSPECIFIED_NODE;

ProxNodeHandle::ProxNodeHandle()
{
    // TODO Auto-generated constructor stub

}

ProxNodeHandle::~ProxNodeHandle()
{
    // TODO Auto-generated destructor stub
}

ProxNodeHandle::ProxNodeHandle(const NodeHandle& nodeHandle)
: NodeHandle(nodeHandle), prox(prox)
{
    //...
}

ProxNodeHandle::ProxNodeHandle(const NodeHandle& nodeHandle, const Prox& prox)
: NodeHandle(nodeHandle), prox(prox)
{
    //...
}

std::ostream& operator<<(std::ostream& os, const Prox& prox)
{
    if (prox == Prox::PROX_SELF) os << "[self]";
    else if (prox == Prox::PROX_UNKNOWN) os << "[unknown]";
    else if (prox == Prox::PROX_TIMEOUT) os << "[timeout]";
    else {
        os << prox.proximity;
        if (prox.accuracy != 1) os << " (a=" << prox.accuracy << ")";
    }
   return os;
}

