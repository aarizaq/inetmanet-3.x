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
 * @file NTreeHelper.h
 * @author Stephan Krause
 */

#ifndef __NTREEHELPER_H_
#define __NTREEHELPER_H_

#include <NodeHandle.h>
#include <Vector2D.h>

class NTreeScope
{
    public:
        NTreeScope();
        NTreeScope(const Vector2D& _origin, double _size);
        void resize(const Vector2D& _origin, double _size);
        bool contains(const Vector2D&) const;
        NTreeScope getSubScope( unsigned int quadrant ) const;

        bool isValid() const { return size >= 0; }
        Vector2D origin;
        double size;
        
        friend bool operator==(const NTreeScope& a, const NTreeScope& b);
        friend bool operator<(const NTreeScope& a, const NTreeScope& b);
        friend std::ostream& operator<<(std::ostream& Stream, const NTreeScope& scope);
};


class NTreeGroup
{
    public:
        NodeHandle leader;
        std::set<NodeHandle> members;
        NTreeScope scope;
        bool dividePending;

        bool isInScope(const Vector2D& p) const;
        NTreeGroup(const NTreeScope& _scope);
        NTreeGroup(const Vector2D& _origin, double _size);
        
        friend bool operator==(const NTreeGroup& a, const NTreeGroup& b);
        friend bool operator<(const NTreeGroup& a, const NTreeGroup& b);
        friend std::ostream& operator<<(std::ostream& Stream, const NTreeGroup& group);
};

class NTreeNode
{
    public:
        NTreeScope scope;
        NodeHandle parent;
        NodeHandle children[4];
        // On leafs, group points to the corresponding group. NULL else.
        NTreeGroup* group;
        // for computing when to collapse trees
        unsigned int aggChildCount[4];
        // for backup on node failure
        NodeHandle siblings[4];
        std::set<NodeHandle> childChildren[4];
        simtime_t lastPing;
        bool parentIsRoot;

        bool isInScope(const Vector2D& p) const;
        const NodeHandle& getChildForPos( const Vector2D& pos ) const;
        NTreeNode(const NTreeScope& _scope);
        NTreeNode(const Vector2D& _origin, double _size);
        
        friend bool operator==(const NTreeNode& a, const NTreeNode& b);
        friend bool operator<(const NTreeNode& a, const NTreeNode& b);
        friend std::ostream& operator<<(std::ostream& Stream, const NTreeNode& node);
};

class NTreeGroupDivideContext
{
    public:
        NodeHandle newChild[4];
        NTreeScope nodeScope;
};

class NTreeGroupDivideContextPtr : public cPolymorphic
{
    public:
        NTreeGroupDivideContext* ptr;
};

class NTreePingContext : public cPolymorphic
{
    public:
        NTreePingContext(const NTreeScope& _scope, unsigned int _quadrant);
        NTreeScope nodeScope;
        unsigned int quadrant;
};

#endif
