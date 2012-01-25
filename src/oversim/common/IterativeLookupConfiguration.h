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
 * @file IterativeLookupConfiguration.h
 * @author Sebastian Mies, Ingmar Baumgart
 */

#ifndef __ITERATIVE_LOOKUP_CONFIGURATION_H
#define __ITERATIVE_LOOKUP_CONFIGURATION_H

/**
 * This class holds the lookup configuration.
 *
 * @author Sebastian Mies, Ingmar Baumgart
 */
class IterativeLookupConfiguration
{
public:
    int redundantNodes;   /**< number of next hops in each step */
    int parallelPaths; /**< number of parallel paths */
    int parallelRpcs;  /**< number of nodes to ask in parallel */
    bool strictParallelRpcs; /**< limited the number of concurrent RPCS to parameter parallelRpcs */
    bool useAllParallelResponses; /**< merge all parallel responses from earlier steps */
    bool newRpcOnEveryTimeout; /**< send a new RPC immediately after an RPC timeouts */
    bool newRpcOnEveryResponse; /**< send a new RPC after every response, even if there was no progress */
    bool finishOnFirstUnchanged; /**< finish lookup, if the last pending RPC returned without progress */
    bool verifySiblings; /**< true, if siblings need to be authenticated with a ping */
    bool majoritySiblings; /**< true, if sibling candidates are selected by a majority decision if using parallel paths */
    bool merge; /**< true, if parallel RPCs results should be merged */
    bool failedNodeRpcs; /**< communicate failed nodes */
    bool visitOnlyOnce; /**< if true, the same node is never asked twice during a single lookup */
    bool acceptLateSiblings; /**< if true, a FindNodeResponse with sibling flag set is always accepted, even if it is from a previous lookup step */
};

#endif
