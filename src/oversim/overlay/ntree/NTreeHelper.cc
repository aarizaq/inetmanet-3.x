#include "NTreeHelper.h"

NTreeScope::NTreeScope()
{
    size = -1;
}

NTreeScope::NTreeScope(const Vector2D& _origin, const double _size) :
    origin(_origin), size(_size)
{
    // Boundary checking?
}

void NTreeScope::resize(const Vector2D& _origin, const double _size)
{
    origin = _origin;
    size = _size;
}

bool NTreeScope::contains(const Vector2D& point) const
{
    if( !isValid() ) return false;
    return origin.xyMaxDistance(point)*2.0 <= size;
}

NTreeScope NTreeScope::getSubScope( unsigned int quadrant ) const
{
    if( !isValid() ) return NTreeScope();

    Vector2D newOrigin = origin;
    double newSize = size/2.0;
    if( quadrant < 2 ) {
        // right half
        newOrigin.x += newSize / 2.0;
    } else {
        newOrigin.x -= newSize / 2.0;
    }
    if( quadrant == 0 || quadrant == 3 ) {
        // upper half
        newOrigin.y += newSize / 2.0;
    } else {
        newOrigin.y -= newSize / 2.0;
    }
    return NTreeScope( newOrigin, newSize );
}

bool operator<(const NTreeScope& a, const NTreeScope& b)
{
    // for sorting only. This results in the biggest scope comming first
    if( a.size == b.size ) {
        return a.origin < b.origin;
    }
    return a.size > b.size;
}

bool operator==(const NTreeScope& a, const NTreeScope& b)
{
    return a.origin == b.origin && a.size == b.size;
}

std::ostream& operator<<(std::ostream& Stream, const NTreeScope& scope)
{
    Stream << "[" << scope.origin << " - " << scope.size << "]";
    return Stream;
}

NTreeGroup::NTreeGroup(const NTreeScope& _scope) :
    scope(_scope)
{
    dividePending = false;
}

NTreeGroup::NTreeGroup(const Vector2D& _origin, const double _size) :
    scope(_origin,_size)
{
    dividePending = false;
}

bool NTreeGroup::isInScope(const Vector2D& point) const
{
    return scope.contains(point);
}

bool operator<(const NTreeGroup& a, const NTreeGroup& b)
{
    return a.scope < b.scope;
}

bool operator==(const NTreeGroup& a, const NTreeGroup& b)
{
    return a.scope == b.scope;
}

std::ostream& operator<<(std::ostream& Stream, const NTreeGroup& group)
{
    Stream << group.scope << " Leader: " << group.leader;
    for( std::set<NodeHandle>::iterator it = group.members.begin(); it != group.members.end(); ++it ){
        Stream << "\n" << it->getIp();
    }
    return Stream;
}


NTreeNode::NTreeNode(const NTreeScope& _scope) :
    scope(_scope)
{
    for( unsigned int i = 0; i < 4; ++i ){
        aggChildCount[i] = 0;
    }
    group = 0;
    parentIsRoot = false;
}

NTreeNode::NTreeNode(const Vector2D& _origin, const double _size) :
    scope(_origin,_size)
{
    for( unsigned int i = 0; i < 4; ++i ){
        aggChildCount[i] = 0;
    }
    group = 0;
    parentIsRoot = false;
}

bool NTreeNode::isInScope(const Vector2D& point) const
{
    return scope.contains(point);
}


const NodeHandle& NTreeNode::getChildForPos( const Vector2D& pos ) const
{
    if (!isInScope( pos ) ) return NodeHandle::UNSPECIFIED_NODE;
    return children[ scope.origin.getQuadrant(pos) ];
}

bool operator==(const NTreeNode& a, const NTreeNode& b)
{
    return a.scope== b.scope;
}

bool operator<(const NTreeNode& a, const NTreeNode& b)
{
    return a.scope < b.scope;
}

std::ostream& operator<<(std::ostream& Stream, const NTreeNode& node)
{
    Stream << node.scope << "\nParent: " << node.parent.getIp();
    if( node.group ) {
        Stream << "\nNode is leaf";
    } else {
        for( unsigned int i = 0; i < 4; ++i ){
            Stream << "\nChild " << i << ": " << node.children[i];
        }
    }
    return Stream;
}

NTreePingContext::NTreePingContext(const NTreeScope& _scope, unsigned int _quadrant) :
        nodeScope(_scope), quadrant(_quadrant)
{
}

