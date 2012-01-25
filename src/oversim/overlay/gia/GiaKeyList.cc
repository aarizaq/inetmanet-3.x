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
 * @file GiaKeyList.cc
 * @author Robert Palmer
 */

#include <vector>

#include "GiaKeyList.h"


void GiaKeyList::addKeyItem(const OverlayKey& item)
{
    if(!contains(item))
        keyList.push_back(item);
}

void GiaKeyList::removeKeyItem(const OverlayKey& item)
{
    std::vector<OverlayKey>::iterator it = keyList.begin();
    if(contains(item))
        keyList.erase(it + getPosition(item));
}

bool GiaKeyList::contains(const OverlayKey& item)
{
    if(getPosition(item) != -1)
        return true;
    return false;
}

int GiaKeyList::getPosition(const OverlayKey& item)
{
    for(uint32_t i = 0; i < keyList.size(); i++)
        if(keyList[i] == item)
            return i;
    return -1;
}

const std::vector<OverlayKey>& GiaKeyList::getVector()
{
    return keyList;
}

uint32_t GiaKeyList::getSize()
{
    return keyList.size();
}

const OverlayKey& GiaKeyList::get
    (uint32_t i)
{
    return keyList[i];
}

std::ostream& operator<<(std::ostream& os, const GiaKeyList& k)
{
    for ( uint32_t i = 0; i<k.keyList.size(); i++ )
        os << k.keyList[i];
    return os;
}
