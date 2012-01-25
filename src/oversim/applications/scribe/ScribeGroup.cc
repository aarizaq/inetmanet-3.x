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
 * @file ScribeGroup.cc
 * @author Stephan Krause
 */



#include "ScribeGroup.h"


ScribeGroup::ScribeGroup( OverlayKey id ) : groupId(id)
{
    parent = NodeHandle::UNSPECIFIED_NODE;
    rendezvousPoint = NodeHandle::UNSPECIFIED_NODE;
    subscription = false;
    amISource = false;
    parentTimer = NULL;
    heartbeatTimer = NULL;
}

ScribeGroup::~ScribeGroup( )
{
    children.clear();
}

bool ScribeGroup::isForwarder() const
{
    return !children.empty();
}

std::pair<std::set<NodeHandle>::iterator, bool> ScribeGroup::addChild( const NodeHandle& node )
{
    return children.insert(node);
}

void ScribeGroup::removeChild( const NodeHandle& node )
{
    children.erase(node);
}


std::set<NodeHandle>::iterator ScribeGroup::getChildrenBegin()
{
    return children.begin();
}

std::set<NodeHandle>::iterator ScribeGroup::getChildrenEnd()
{
    return children.end();
}

