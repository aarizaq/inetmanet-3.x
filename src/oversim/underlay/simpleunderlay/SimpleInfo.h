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
 * @file SimpleInfo.h
 * @author Helge Backhaus
 */

#ifndef __SIMPLEINFO_H__
#define __SIMPLEINFO_H__

#include <omnetpp.h>
#include <PeerInfo.h>
#include <SimpleNodeEntry.h>

class SimpleInfo : public PeerInfo
{
public:
    /**
     * constructor
     */
    SimpleInfo(uint32_t type, int moduleId, cObject** context);
    ~SimpleInfo();

    /**
     * setter and getter
     */
    void setEntry(SimpleNodeEntry* entry) { delete this->entry; this->entry = entry; };
    SimpleNodeEntry* getEntry() { return entry; };

protected:
    void dummy(); /**< dummy-function to make SimpleInfo polymorphic */

    SimpleNodeEntry* entry;
};

#endif
