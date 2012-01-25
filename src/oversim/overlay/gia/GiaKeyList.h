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
 * @file GiaKeyList.h
 * @author Robert Palmer
 */
#ifndef __GIAKEYLIST_H_
#define __GIAKEYLIST_H_

#include <OverlayKey.h>

/**
 * This class is for managing (search)keys
 */
class GiaKeyList
{
  public:
    /**
     * Add key-item to keyList
     * @param item to add
     */
    void addKeyItem(const OverlayKey& item);

    /**
     * Removes key-item from keyList
     * @param item to remove
     */
    void removeKeyItem(const OverlayKey& item);

    /**
     * @param item to check
     * @return true, if keylist contains item
     */
    bool contains(const OverlayKey& item);

    /**
     * @return vector of key
     */
    const std::vector<OverlayKey>& getVector();

    /**
     * @return size of keyList-vector
     */
    uint32_t getSize();

    /**
     * @return element at position i
     */
    const OverlayKey& get(uint32_t i);

    friend std::ostream& operator<<(std::ostream& os, const GiaKeyList& k);

  protected:
    std::vector<OverlayKey> keyList; /**< contains all search keys */

    /**
     * @param item to get position
     * @return position of item in keylist, -1 if no item found
     */
    int getPosition(const OverlayKey& item);
};

#endif
