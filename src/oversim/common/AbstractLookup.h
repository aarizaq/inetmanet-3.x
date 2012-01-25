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
 * @file AbstractLookup.h
 * @author Sebastian Mies
 */

#ifndef __ABSTRACT_LOOKUP_H
#define __ABSTRACT_LOOKUP_H

#include <NodeVector.h>

class OverlayKey;
class LookupListener;

/**
 * This class declares an abstract iterative lookup.
 *
 * @author Sebastian Mies
 */
class AbstractLookup
{
public:
    /**
     * Virtual destructor
     */
    virtual ~AbstractLookup();

    /**
     * Lookup siblings for a key
     *
     * @param key The key to lookup
     * @param numSiblings Number of siblings to lookup
     * @param hopCountMax Maximum hop count
     * @param retries Number of retries if lookup fails
     * @param listener Listener to inform, when the lookup is done
     */
    virtual void lookup(const OverlayKey& key, int numSiblings = 1,
                        int hopCountMax = 0, int retries = 0,
                        LookupListener* listener = NULL) = 0;

    /**
     * Returns the result of the lookup
     *
     * @return The result node vector.
     */
    virtual const NodeVector& getResult() const = 0;

    /**
     * Returns true, if the lookup was successful.
     *
     * @return true, if the lookup was successful.
     */
    virtual bool isValid() const = 0;

    /**
     * Aborts a running lookup.
     *
     * This method aborts a running lookup without calling the
     * listener and delete the lookup object.
     */
    virtual void abortLookup() = 0;

    /**
     * Returns the total number of hops for all lookup paths.
     *
     * @return The accumulated number of hops.
     */
    virtual uint32_t getAccumulatedHops() const = 0;

};

#endif
