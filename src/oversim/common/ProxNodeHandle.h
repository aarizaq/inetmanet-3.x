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
 * @file ProxNodeHandle.h
 * @author Bernhard Heep
 */


#ifndef PROXNODEHANDLE_H_
#define PROXNODEHANDLE_H_

#include <cfloat>

#include "NodeHandle.h"


struct Prox
{
    static const Prox PROX_SELF;
    static const Prox PROX_UNKNOWN;
    static const Prox PROX_TIMEOUT;
    //static const Prox PROX_WAITING;

    double proximity; // [0 - INF)
    double accuracy;  // [0 - 1] 1: exact value, 0: no information available

    operator double();
    operator simtime_t();

    Prox();
    Prox(simtime_t prox);
    Prox(simtime_t prox, double acc);
    Prox(double prox, double acc);

    bool operator==(Prox p) const;
    bool operator!=(Prox p) const;

    friend std::ostream& operator<<(std::ostream& os, const Prox& prox);
};

struct ProxKey
{
    ProxKey(const Prox& prox, const OverlayKey& key) : prox(prox), key(key) { };
    Prox prox;
    OverlayKey key;
};


class ProxNodeHandle : public NodeHandle
{
  protected:
    Prox prox;

  public:
    static const ProxNodeHandle UNSPECIFIED_NODE; /**< the unspecified ProxNodeHandle */
    ProxNodeHandle();
    ProxNodeHandle(const NodeHandle& nodeHandle);
    ProxNodeHandle(const NodeHandle& nodeHandle, const Prox& prox);
    virtual ~ProxNodeHandle();

    inline void setProx(Prox prox) { this->prox = prox; };
    inline Prox getProx() const { return prox; };
};

#endif /* PROXNODEHANDLE_H_ */
