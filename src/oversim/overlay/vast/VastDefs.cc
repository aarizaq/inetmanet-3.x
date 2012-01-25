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
 * @file VastDefs.cc
 * @author Helge Backhaus
 */

#include "VastDefs.h"

Site::Site()
{
    type = UNDEF;
    innerEdge[0] = false;
    innerEdge[1] = false;
    innerEdge[2] = false;
    outerEdge = false;
    isAdded = false;
    neighborCount = 0;
    addr = NodeHandle::UNSPECIFIED_NODE;
    tstamp = 0.0;
}

std::ostream& operator<<(std::ostream& Stream, const Site s)
{
    Stream  << "Type: ";
    if(s.type & UNDEF) Stream << "Undefined ";
    if(s.type & THIS) Stream << "This ";
    if(s.type & ENCLOSING) Stream << "Enclosing ";
    if(s.type & NEIGHBOR) Stream << "Inner ";
    if(s.type & BOUNDARY) Stream << "Boundary ";
    if(s.type & NEW) Stream << "Discovered ";
    return Stream << "  IP: " << s.addr.getIp();
}

Edge::Edge()
{
    a = 0.0;
    b = 0.0;
    c = 0.0;
    ep[0] = NULL;
    ep[1] = NULL;
    reg[0] = NULL;
    reg[1] = NULL;
}

Halfedge::Halfedge()
{
    ELleft = NULL;
    ELright = NULL;
    ELedge = NULL;
    ELpm = 0;
    vertex = NULL;
    ystar = 0.0;
    PQnext = NULL;
}

HeapPQ::HeapPQ()
{
    PQhash = NULL;
}

void HeapPQ::PQinitialize(int sqrt_nsites, double ymin, double deltay)
{
    PQcount = 0;
    PQmin = 0;
    PQhashsize = 4 * sqrt_nsites;
    PQhash = new Halfedge[PQhashsize];
    this->ymin = ymin;
    this->deltay = deltay;
}

void HeapPQ::PQreset()
{
    delete[] PQhash;
}

void HeapPQ::PQinsert(Halfedge *he, Site *v, double offset)
{
    Halfedge *last, *next;

    he->vertex = v;
    he->ystar = v->coord.y + offset;
    last = &PQhash[PQbucket(he)];
    while((next = last->PQnext) != NULL && (he->ystar  > next->ystar  || (he->ystar == next->ystar && v->coord.x > next->vertex->coord.x))) {
        last = next;
    }
    he->PQnext = last->PQnext;
    last->PQnext = he;
    PQcount++;
}

void HeapPQ::PQdelete(Halfedge *he)
{
    Halfedge *last;

    if(he->vertex != NULL) {
        last = &PQhash[PQbucket(he)];
        while(last->PQnext != he) last = last->PQnext;

        last->PQnext = he->PQnext;
        PQcount--;
        he->vertex = NULL;
    }
}

int HeapPQ::PQbucket(Halfedge *he)
{
    int bucket;

    bucket = (int)((he->ystar - ymin)/deltay) * PQhashsize;
    if(bucket < 0) bucket = 0;
    if(bucket >= PQhashsize) bucket = PQhashsize-1;
    if(bucket < PQmin) PQmin = bucket;
    return bucket;
}

int HeapPQ::PQempty()
{
    return PQcount==0;
}

Vector2D HeapPQ::PQ_min()
{
    Vector2D answer;

    while(PQhash[PQmin].PQnext == NULL) PQmin++;

    answer.x = PQhash[PQmin].PQnext->vertex->coord.x;
    answer.y = PQhash[PQmin].PQnext->ystar;
    return answer;
}

Halfedge* HeapPQ::PQextractmin()
{
    Halfedge *curr;

    curr = PQhash[PQmin].PQnext;
    PQhash[PQmin].PQnext = curr->PQnext;
    PQcount--;
    return curr;
}

void Geometry::initialize(double deltax, double deltay, Vector2D center, Vector2D old_pos, Vector2D new_pos, double radius)
{
    this->deltay = deltay;
    this->deltax = deltax;
    this->center[0] = center;
    this->center[1] = old_pos;
    this->center[2] = new_pos;
    if((old_pos.x != new_pos.x) || (old_pos.y != new_pos.y)) doDiscovery = true;
    else doDiscovery = false;
    this->sq_radius = radius*radius;
}

void Geometry::reset()
{
    for(std::vector<Site*>::iterator itTemp = SITEVector.begin(); itTemp != SITEVector.end(); ++itTemp) {
        delete *itTemp;
    }
    for(std::vector<Edge*>::iterator itTemp = EDGEVector.begin(); itTemp != EDGEVector.end(); ++itTemp) {
        delete *itTemp;
    }
    SITEVector.clear();
    EDGEVector.clear();
}

void Geometry::setDebug(bool debugOutput)
{
    this->debugOutput = debugOutput;
}

bool Geometry::intersectCircleSite(Site *s, Vector2D center)
{
    double sq_distance;
    Vector2D temp;
    temp.x = s->coord.x - center.x;
    temp.y = s->coord.y - center.y;
    sq_distance = temp.x*temp.x + temp.y*temp.y;
    return sq_distance < sq_radius ? true : false;
}

bool Geometry::intersectCircleLine(Vector2D start, Vector2D dir, Vector2D center, bool lowerBound, bool upperBound)
{
    Vector2D StartMinusCenter;
    double DirDotStartMinusCenter, DirSq, StartMinusCenterSq, discriminant;
    StartMinusCenter.x = start.x - center.x;
    StartMinusCenter.y = start.y - center.y;
    StartMinusCenterSq = StartMinusCenter.x * StartMinusCenter.x + StartMinusCenter.y * StartMinusCenter.y;

    DirDotStartMinusCenter = dir.x * StartMinusCenter.x + dir.y * StartMinusCenter.y;
    DirSq = dir.x * dir.x + dir.y * dir.y;

    discriminant = DirDotStartMinusCenter * DirDotStartMinusCenter - DirSq * (StartMinusCenterSq - sq_radius);

    if(discriminant <= 0.0f) return false;
    else if(lowerBound) {
        double s = (-DirDotStartMinusCenter - sqrtf(discriminant)) / DirSq;
        if(s < 0.0f) return false;
        else if(upperBound && s > 1.0f) return false;
    }
    return true;
}

void Geometry::processEdge(Edge *e)
{
    bool leftEndpoint_In[3], rightEndpoint_In[3];
    int i, numTest;
    // test the edge just against our own AOI or also against AOI's of a moving neighbor
    numTest = doDiscovery ? 3 : 1;

    for(i = 0; i < numTest; i++) {
        if(e->ep[le]) leftEndpoint_In[i] = intersectCircleSite(e->ep[le], center[i]);
        else leftEndpoint_In[i] = false;
        if(e->ep[re]) rightEndpoint_In[i] = intersectCircleSite(e->ep[re], center[i]);
        else rightEndpoint_In[i] = false;
    }
    for(i = 0; i < numTest; i++) {
        if(leftEndpoint_In[i] || rightEndpoint_In[i]) {
            if(!e->reg[le]->innerEdge[i]) e->reg[le]->innerEdge[i] = true;
            if(!e->reg[re]->innerEdge[i]) e->reg[re]->innerEdge[i] = true;
        }
    }
    if(!leftEndpoint_In[0] || !rightEndpoint_In[0]) {
        if(!e->reg[le]->outerEdge) e->reg[le]->outerEdge = true;
        if(!e->reg[re]->outerEdge) e->reg[re]->outerEdge = true;
    }
    for(i = 0; i < numTest; i++) {
        if(!(leftEndpoint_In[i] || rightEndpoint_In[i])) {
            bool lineTest = false;
            if(e->ep[le] && e->ep[re]) {
                Vector2D t_dir;
                t_dir.x = e->ep[re]->coord.x - e->ep[le]->coord.x;
                t_dir.y = e->ep[re]->coord.y - e->ep[le]->coord.y;
                lineTest = intersectCircleLine(e->ep[le]->coord, t_dir, center[i], true, true);
            }
            if((e->ep[le] && !e->ep[re]) || (!e->ep[le] && e->ep[re])) {
                Vector2D t_dir;
                t_dir.x = e->b;
                t_dir.y = -(e->a);
                if(e->ep[le]) {
                    if(t_dir.x < 0.0f) {
                        t_dir.x = -t_dir.x;
                        t_dir.y = -t_dir.y;
                    }
                    lineTest = intersectCircleLine(e->ep[le]->coord, t_dir, center[i], true, false);
                }
                else {
                    if(t_dir.x >= 0.0f) {
                        t_dir.x = -t_dir.x;
                        t_dir.y = -t_dir.y;
                    }
                    lineTest = intersectCircleLine(e->ep[re]->coord, t_dir, center[i], true, false);
                }
            }
            if(!(e->ep[le] || e->ep[re])) {
                Vector2D t_start, t_dir;
                if(e->b == 0.0f) {
                    t_start.x = e->c / e->a;
                    t_start.y = 0.0f;
                }
                else {
                    t_start.x = 0.0f;
                    t_start.y = e->c / e->b;
                }
                t_dir.x = e->b;
                t_dir.y = -(e->a);
                lineTest = intersectCircleLine(t_start, t_dir, center[i], false, false);
            }

            if(lineTest) {
                if(!e->reg[le]->innerEdge[i]) e->reg[le]->innerEdge[i] = true;
                if(!e->reg[re]->innerEdge[i]) e->reg[re]->innerEdge[i] = true;
            }
        }
    }
    // enhanced enclosing test
    e->reg[re]->enclosingSet.insert(e->reg[le]->addr);
    e->reg[le]->enclosingSet.insert(e->reg[re]->addr);
    
    // test if one of the nodes bisected by the edge is an enclosing neighbor
    if(e->reg[le]->type == THIS) {
        e->reg[re]->type |= ENCLOSING;
        // Debug output
        if(debugOutput) EV << "[Geometry::processEdge()]\n"
                           << "    Site at [" << e->reg[re]->coord.x << ", " << e->reg[re]->coord.y << "] is an enclosing neighbor."
                           << endl;
    }
    if(e->reg[re]->type == THIS) {
        e->reg[le]->type |= ENCLOSING;
        // Debug output
        if(debugOutput) EV << "[Geometry::processEdge()]\n"
                           << "    Site at [" << e->reg[le]->coord.x << ", " << e->reg[le]->coord.y << "] is an enclosing neighbor."
                           << endl;
    }
}

Edge* Geometry::bisect(Site *s1, Site *s2)
{
    double dx, dy, adx, ady;
    Edge *newedge;

    newedge = new Edge;

    newedge->reg[0] = s1;
    newedge->reg[1] = s2;
    newedge->ep[0] = NULL;
    newedge->ep[1] = NULL;

    dx = s2->coord.x - s1->coord.x;
    dy = s2->coord.y - s1->coord.y;
    adx = dx > 0 ? dx : -dx;
    ady = dy > 0 ? dy : -dy;
    newedge->c = s1->coord.x * dx + s1->coord.y * dy + (dx*dx + dy*dy)*0.5;
    if(adx>ady) {
        newedge->a = 1.0;
        newedge->b = dy/dx;
        newedge->c /= dx;
    }
    else {
        newedge->b = 1.0;
        newedge->a = dx/dy;
        newedge->c /= dy;
    }

    EDGEVector.push_back(newedge);
    return newedge;
}

Site* Geometry::intersect(Halfedge *el1, Halfedge *el2)
{
    Edge *e1, *e2, *e;
    Halfedge *el;
    double d, xint, yint;
    int right_of_site;
    Site *v;

    e1 = el1->ELedge;
    e2 = el2->ELedge;
    if(e1 == NULL || e2 == NULL) return NULL;
    if(e1->reg[1] == e2->reg[1]) return NULL;

    d = e1->a * e2->b - e1->b * e2->a;
    if(-1.0e-10 < d && d < 1.0e-10) return NULL;

    xint = (e1->c * e2->b - e2->c * e1->b)/d;
    yint = (e2->c * e1->a - e1->c * e2->a)/d;

    if((e1->reg[1]->coord.y < e2->reg[1]->coord.y) || (e1->reg[1]->coord.y == e2->reg[1]->coord.y && e1->reg[1]->coord.x < e2->reg[1]->coord.x)) {
        el = el1;
        e = e1;
    }
    else {
        el = el2;
        e = e2;
    }

    right_of_site = xint >= e->reg[1]->coord.x;
    if((right_of_site && el->ELpm == le) || (!right_of_site && el->ELpm == re)) return NULL;

    v = new Site;

    v->coord.x = xint;
    v->coord.y = yint;

    SITEVector.push_back(v);
    return v;
}

void Geometry::endpoint(Edge *e, int lr, Site *s)
{
    e->ep[lr] = s;
    if(e->ep[re-lr] == NULL) return;
    processEdge(e);
}

double Geometry::dist(Site *s, Site *t)
{
    double dx, dy;
    dx = s->coord.x - t->coord.x;
    dy = s->coord.y - t->coord.y;
    return sqrt(dx*dx + dy*dy);
}

EdgeList::EdgeList()
{
    ELhash = NULL;
}

void EdgeList::initialize(int sqrt_nsites, double xmin, double deltax, Site *bottomsite)
{
    int i;

    HEcount = 0;
    ELhashsize = 2 * sqrt_nsites;

    HEmemmap = new Halfedge*[ELhashsize];

    ELhash = new Halfedge*[ELhashsize];
    for(i=0; i<ELhashsize; i++) ELhash[i] = NULL;
    ELleftend = HEcreate(NULL, 0);
    ELrightend = HEcreate(NULL, 0);
    ELleftend->ELleft = NULL;
    ELleftend->ELright = ELrightend;
    ELrightend->ELleft = ELleftend;
    ELrightend->ELright = NULL;
    ELhash[0] = ELleftend;
    ELhash[ELhashsize-1] = ELrightend;
    this->xmin = xmin;
    this->deltax = deltax;
    this->bottomsite = bottomsite;
}

void EdgeList::reset()
{
    delete[] ELhash;
    // free all allocated memory
    for(int i=0; i<HEcount; i++) delete HEmemmap[i];
    delete[] HEmemmap;
}

Halfedge* EdgeList::HEcreate(Edge *e, int pm)
{
    Halfedge *answer = new Halfedge;
    answer->ELedge = e;
    answer->ELpm = pm;

    HEmemmap[HEcount++] = answer;
    if(HEcount%ELhashsize == 0) {
        Halfedge **Temp = new Halfedge*[HEcount + ELhashsize];
        for(int i=0; i<HEcount; i++) Temp[i] = HEmemmap[i];
        delete[] HEmemmap;
        HEmemmap = Temp;
    }
    return answer;
}

void EdgeList::ELinsert(Halfedge *lb, Halfedge *new_he)
{
    new_he->ELleft = lb;
    new_he->ELright = lb->ELright;
    (lb->ELright)->ELleft = new_he;
    lb->ELright = new_he;
}

/* Get entry from hash table, pruning any deleted nodes */
Halfedge* EdgeList::ELgethash(int b)
{
    Halfedge *he;

    if(b < 0 || b >= ELhashsize) return NULL;
    he = ELhash[b];
    if(he == NULL || he->ELedge != (Edge*)DELETED) return he;

    /* Hash table points to deleted half edge. */
    ELhash[b] = NULL;
    return NULL;
}

/* returns 1 if p is to right of halfedge e */
int EdgeList::right_of(Halfedge *el, Vector2D *p)
{
    Edge *e;
    Site *topsite;
    int right_of_site, above, fast;
    double dxp, dyp, dxs, t1, t2, t3, yl;

    e = el->ELedge;
    topsite = e->reg[1];
    right_of_site = p->x > topsite->coord.x;
    if(right_of_site && el->ELpm == le) return 1;
    if(!right_of_site && el->ELpm == re) return 0;

    if(e->a == 1.0) {
        dyp = p->y - topsite->coord.y;
        dxp = p->x - topsite->coord.x;
        fast = 0;
        if((!right_of_site && (e->b < 0.0)) || (right_of_site && (e->b >= 0.0))) {
            above = dyp >= e->b * dxp;
            fast = above;
        }
        else {
            above = p->x + p->y * e->b > e->c;
            if(e->b < 0.0) above = !above;
            if(!above) fast = 1;
        }
        if(!fast) {
            dxs = topsite->coord.x - (e->reg[0])->coord.x;
            above = e->b * (dxp*dxp - dyp*dyp) < dxs * dyp * (1.0 + 2.0*dxp/dxs + e->b*e->b);
            if(e->b < 0.0) above = !above;
        }
    }
    else {
        yl = e->c - e->a * p->x;
        t1 = p->y - yl;
        t2 = p->x - topsite->coord.x;
        t3 = yl - topsite->coord.y;
        above = t1*t1 > t2*t2 + t3*t3;
    }
    return el->ELpm == le ? above : !above;
}

Halfedge* EdgeList::ELleftbnd(Vector2D *p)
{
    int i, bucket;
    Halfedge *he;

    /* Use hash table to get close to desired halfedge */
    bucket = (int)((p->x - xmin) / deltax) * ELhashsize;
    if(bucket < 0) bucket =0;
    if(bucket >= ELhashsize) bucket = ELhashsize - 1;
    he = ELgethash(bucket);
    if(he == NULL) {
        for(i=1; 1; i++) {
            if((he = ELgethash(bucket-i)) != NULL) break;
            if((he = ELgethash(bucket+i)) != NULL) break;
        }
    totalsearch++;
    }
    ntry++;
    /* Now search linear list of halfedges for the corect one */
    if(he == ELleftend  || (he != ELrightend && right_of(he, p))) {
        do {he = he->ELright;} while(he != ELrightend && right_of(he, p));
        he = he->ELleft;
    }
    else do {he = he->ELleft;} while(he != ELleftend && !right_of(he, p));

    /* Update hash table and reference counts */
    if(bucket > 0 && bucket < ELhashsize-1) {
        ELhash[bucket] = he;
    }
    return he;
}

void EdgeList::ELdelete(Halfedge *he)
{
    (he->ELleft)->ELright = he->ELright;
    (he->ELright)->ELleft = he->ELleft;
    he->ELedge = (Edge*)DELETED;
}

Halfedge* EdgeList::ELright(Halfedge *he)
{
    return he->ELright;
}

Halfedge* EdgeList::ELleft(Halfedge *he)
{
    return he->ELleft;
}


Site* EdgeList::leftreg(Halfedge *he)
{
    if(he->ELedge == NULL) return(bottomsite);
    return he->ELpm == le ? he->ELedge->reg[le] : he->ELedge->reg[re];
}

Site* EdgeList::rightreg(Halfedge *he)
{
    if(he->ELedge == NULL) return(bottomsite);
    return he->ELpm == le ? he->ELedge->reg[re] : he->ELedge->reg[le];
}
