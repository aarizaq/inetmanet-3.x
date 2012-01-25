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
 * @file QuonDefs.h
 * @author Helge Backhaus
 * @author Stephan Krause
 */

#ifndef __QUONDEFS_H_
#define __QUONDEFS_H_

#include <Vector2D.h>
#include <NodeHandle.h>
#include <limits.h>
#include <float.h>
#include <set>
#include <map>

#define QINFINITY DBL_MAX

/// Common Quon Definitions

// protocol states
enum QState {
    QUNINITIALIZED, QJOINING, QREADY
};

// node types
enum QNeighborType {
    QUNDEFINED, QTHIS, QNEIGHBOR, QBINDING
};

// purge types
enum QPurgeType {
    QKEEPSOFT, QPURGESOFT
};

// update types
enum QUpdateType{
    QDIRECT, QFOREIGN
};

//typedef std::set<OverlayKey> QKeySet;
typedef std::set<Vector2D> QPositionSet;
typedef std::map<OverlayKey, Vector2D> QDeleteMap;

#endif
