//
// Copyright (C) 2006 Autonomic Networking Group,
// Department of Computer Science 7, University of Erlangen, Germany
//
// Author: Isabel Dietrich
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
 * @short Example implementation of a Traffic Generator derived from the
          TrafGen base class
 * @author Isabel Dietrich
*/

#ifndef WIMAX_QOS_APP
#define WIMAX_QOS_APP

#include "TrafGen.h"
#include "Ieee80216Primitives_m.h"

class WiMAXQoSTrafficGenerator : public TrafGen
{
  public:
    static bool seeded;

    // LIFECYCLE
    // this takes care of constructors and destructors
    //Module_Class_Members(WiMAXQoSTrafficGenerator, TrafGen, 0);

    virtual void initialize(int);
    virtual void finish();

  protected:
    // OPERATIONS
    virtual void handleSelfMsg(cPacket *);
    virtual void handleLowerMsg(cPacket *);

    virtual void SendTraf(cPacket *msg, const char *dest);

  private:
    int mLowerLayerIn;
    int mLowerLayerOut;

    int mCurrentTrafficPattern;

    double mNumTrafficMsgs;

    ip_traffic_types ipTrafficType;
    double interDepartureTime, packetSize;
};

#endif
