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

#include <omnetpp.h>
#include <cnetcommbuffer.h>
#include "OverSimMessage_m.h"
#include "OverSimMessage.h"

Register_Class(OverSimMessage);

void OverSimMessage::parsimPack(cCommBuffer *b)
{
    cObject::parsimPack(b);

    if (getContextPointer() || getControlInfo())
        throw cRuntimeError(this, "netPack(): cannot pack object with "
                                "contextPointer or controlInfo set");
    if (getParList().size() > 0) {
        b->packFlag(true);
        b->packObject(&getParList());
    } else {
        b->packFlag(false);
    }

    if (b->packFlag(getEncapsulatedPacket() != NULL))
        b->packObject(getEncapsulatedPacket());
}

void OverSimMessage::parsimUnpack(cCommBuffer *b)
{
    int len = 0;
    cNetCommBuffer *netBuffer = dynamic_cast<cNetCommBuffer*>(b);
    if (netBuffer != NULL) {
        len = netBuffer->getRemainingMessageSize();
    }
    cObject::parsimUnpack(b);

    ASSERT(getShareCount() == 0);

    if (b->checkFlag()) {
        cArray *parlistptr = static_cast<cArray*>(b->unpackObject());
        std::cout << "still there: " << *parlistptr << std::endl;
        for (int i=0; i<parlistptr->size(); i++) {
            std::cout << "i: " << i << " " << parlistptr->get(i) << std::endl;
            addObject(static_cast<cObject*>(parlistptr->get(i)->dup()));
        }
        delete parlistptr;
    }

    if (b->checkFlag()) {
        encapsulate((cPacket *) b->unpackObject());
    }

    // set the length of the received message
    // TODO: doesn't contain the length of the string for the object type
    setByteLength(len);
}
