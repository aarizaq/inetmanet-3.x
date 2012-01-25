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
 * @file VastDefs.h
 * @author Helge Backhaus
 */

#ifndef __DEFS_H_
#define __DEFS_H_

#include <NodeHandle.h>
#include <Vector2D.h>
#include <math.h>
#include <map>
#include <set>
#include <list>
#include <vector>

// constants for node types
#define UNDEF       0
#define THIS        1
#define ENCLOSING   2
#define NEIGHBOR    4
#define BOUNDARY    8
#define NEW        16

#define le 0
#define re 1

#define DELETED -2

/// Vast Definitions
/**
 Some structures needed for building a voronoi diagram and maintaining the overlays neighbors.
 */

typedef std::set<Vector2D> PositionSet;
typedef std::set<NodeHandle> EnclosingSet;
typedef std::list<NodeHandle> StockList;

class Site
{
    public:
        Site();
        Vector2D        coord;
        unsigned char   type;
        bool            innerEdge[3], outerEdge, isAdded;
        NodeHandle      addr;
        simtime_t       tstamp;
        int             neighborCount;
        EnclosingSet    enclosingSet;       // enhanced enclosing test
        EnclosingSet    oldEnclosingSet;    // enhanced enclosing test
        friend std::ostream& operator<<(std::ostream& Stream, const Site s);
};

typedef std::map<NodeHandle, Site*> SiteMap;

class Edge
{
    public:
        Edge();
        double  a, b, c;
        Site    *ep[2];
        Site    *reg[2];
};

class Halfedge
{
    public:
        Halfedge();
        Halfedge    *ELleft, *ELright;
        Edge        *ELedge;
        char        ELpm;
        Site        *vertex;
        double      ystar;
        Halfedge    *PQnext;
};

/// HeapPQ class
/**
 Maintains a hash table in order to find halfedges in n*log(n) time.
 */

class HeapPQ
{
    public:
        HeapPQ();
        void PQinitialize(int sqrt_nsites, double ymin, double deltay);
        void PQreset();
        void PQinsert(Halfedge *he, Site *v, double offset);
        void PQdelete(Halfedge *he);
        int PQbucket(Halfedge *he);
        int PQempty();
        Vector2D PQ_min();
        Halfedge* PQextractmin();

    protected:
        int PQcount, PQmin, PQhashsize;
        double ymin, deltay;
        Halfedge *PQhash;
};

/// Geometry class
/**
 Provides basic line inter- / bisecting and processing functions, needed to build the voronoi and determine neighborhood relationships.
 */

class Geometry
{
    public:
        void initialize(double deltax, double deltay, Vector2D center, Vector2D old_pos, Vector2D new_pos, double radius);
        void reset();
        void setDebug(bool debugOutput);
        Edge* bisect(Site *s1, Site *s2);
        Site* intersect(Halfedge *el1, Halfedge *el2);
        void endpoint(Edge *e, int lr, Site *s);
        void processEdge(Edge *e);
        double dist(Site *s, Site *t);

    protected:
        std::vector<Site*> SITEVector;
        std::vector<Edge*> EDGEVector;
        //int SITEcount, EDGEcount;

        double deltax, deltay, sq_radius;
        Vector2D center[3];
        bool debugOutput, doDiscovery;
        bool intersectCircleLine(Vector2D start, Vector2D dir, Vector2D center, bool lowerBound, bool upperBound);
        bool intersectCircleSite(Site *s, Vector2D center);
};

/// EdgeList class
/**
 Maintains the edges found while building the voronoi diagram.
*/

class EdgeList
{
    public:
        /// Standard constructor.
        EdgeList();
        /// Initializes the list before building the diagram.
        /**
        \@param sqrt_nsites Squareroot of the total number of sites.
        \@param xmin Min x coordinate of all sites.
        \@param deltax xmin+deltax is max x coordinate of all sites.
        \@param bottomsite A pointer to the bottom site of the sites list.
        */
        void initialize(int sqrt_nsites, double xmin, double deltax, Site *bottomsite);
        /// Resets the list for further use.
        void reset();
        /// Creates a halfedge from a given edge.
        /**
        \@param e A pointer to an edge.
        \@param pm Determins wether the halfedge represents the left or right "side" of the given edge (le/re).
        \@return Returns the created halfedge.
        */
        Halfedge* HEcreate(Edge *e, int pm);
        /// Inserts a new halfedge to the list.
        /**
        \@param lb lower bound for this edge.
        \@param new_he A new halfedge to be added to the list.
        */
        void ELinsert(Halfedge *lb, Halfedge *new_he);
        /// Get an entry from the list by number.
        /**
        \@param b An integer.
        \@return Returns halfedge number b.
        */
        Halfedge* ELgethash(int b);
        /// Get an entry from the list by point.
        /**
        \@param p A pointer to a point.
        \@return Returns halfedge nearest to p.
        */
        Halfedge* ELleftbnd(Vector2D *p);
        /// Delete an entry from the list.
        /**
        \@param he Halfedge to be removed.
        */
        void ELdelete(Halfedge *he);
        /// Get right neighbor of an edge.
        /**
        \@param he A halfedge.
        \@return Returns right neighbor of halfedge he.
        */
        Halfedge* ELright(Halfedge *he);
        /// Get left neighbor of an edge.
        /**
        \@param he A halfedge.
        \@return Returns left neighbor of halfedge he.
        */
        Halfedge* ELleft(Halfedge *he);
        /// Get site left of an edge.
        /**
        \@param he A halfedge.
        \@return Returns site left of halfedge he.
        */
        Site* leftreg(Halfedge *he);
        /// Get site right of an edge.
        /**
        \@param he A halfedge.
        \@return Returns site right of halfedge he.
        */
        Site* rightreg(Halfedge *he);
        /// Determines if a point is right of an halfedge.
        /**
        \@param he A halfedge.
        \@param p A point.
        \@return Returns 1 if point p is right of halfedge el, 0 otherwise.
        */
        int right_of(Halfedge *el, Vector2D *p);

        Halfedge *ELleftend, *ELrightend;

    protected:
        int ELhashsize, totalsearch, ntry, HEcount;
        double xmin, deltax;
        Halfedge **ELhash;
        Halfedge **HEmemmap;
        Site *bottomsite;
};

#endif
