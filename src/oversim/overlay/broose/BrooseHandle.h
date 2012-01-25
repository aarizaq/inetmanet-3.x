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
 * @file BrooseHandle.h
 * @author Jochen Schenk
 */

#ifndef __BROOSEHANDLE_H_
#define __BROOSEHANDLE_H_

#include <NodeHandle.h>

class BrooseHandle : public NodeHandle
{
 public: //fields
  int failedResponses;
  simtime_t rtt;
  simtime_t lastSeen;

 public://construction
    BrooseHandle();
    BrooseHandle( OverlayKey initKey, IPvXAddress initIP, int initPort);
    BrooseHandle( const NodeHandle& node );
    BrooseHandle( const TransportAddress& node, const OverlayKey& destKey );

 private:
    static const BrooseHandle* _unspecifiedNode;
 public:
	static const BrooseHandle& unspecifiedNode()
	{
	    if (!_unspecifiedNode)
	        _unspecifiedNode = new BrooseHandle();
	    return *_unspecifiedNode;
	}

    bool operator==(const BrooseHandle& rhs) const;
    bool operator!=(const BrooseHandle& rhs) const;
    BrooseHandle& operator=(const BrooseHandle& rhs);

    friend std::ostream& operator<<(std::ostream& os, const BrooseHandle& n);
};

#endif
