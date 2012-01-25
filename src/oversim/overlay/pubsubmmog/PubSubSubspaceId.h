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
 * @file PubSubSubspaceId.h
 * @author Stephan Krause
 */


#ifndef __PUBSUBSUBSPACEID_H_
#define __PUBSUBSUBSPACEID_H_

#include <ostream>

class PubSubSubspaceId
{
    protected:
        int spaceId;
        int maxY;

    public:
        /**
         * Creates a new PubSubSubspace
         *
         * @param id The group ID of the new group
         * @param _maxY The number of subspaces per row
         */
        PubSubSubspaceId( int id, int _maxY ) : spaceId(id), maxY(_maxY) {};
        PubSubSubspaceId( int x, int y, int _maxY ) : spaceId( x*_maxY + y), maxY(_maxY) {};
        ~PubSubSubspaceId( ) {};

        int getId() const { return spaceId; }
        int getX() const { return (int) spaceId / maxY; }
        int getY() const { return (int) spaceId % maxY; }

        bool operator< (const PubSubSubspaceId x) const { return spaceId < x.spaceId; }
        bool operator== (const PubSubSubspaceId x) const { return spaceId == x.spaceId; }
        
        friend std::ostream& operator<< (std::ostream&, const PubSubSubspaceId&);
};

#endif
