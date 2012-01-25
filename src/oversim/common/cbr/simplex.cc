/* Numerical Methods, S06
 * Note 8, Function optimization using the downhill Simplex method.
 * Peter Seidler
 * Source Code taken from: http://whome.phys.au.dk/~seidler/numeric/ 		*/

#include "simplex.h"
//#include "CoordNodeFunctions.h"
#include <NeighborCache.h>

#include <cmath>

using namespace std;


/***************************
 * Simplex class defintions.
 ***************************/

Simplex::Simplex(int dimension)
{
    dim = dimension;
    nverts = dim+1;
    verts = new Vec_DP*[nverts];
    for (int i=0; i<nverts; i++)
        verts[i] = new Vec_DP(dim);
}

Simplex::~Simplex()
{
    for (int i=0; i<nverts; i++)
        delete verts[i];
    delete[] verts;
}

int Simplex::high(double* val) const
{
    double test;
    double max = functionObject->f(*verts[0]);
    int idx = 0;
    for (int i=1; i<nverts; i++) {
        test = functionObject->f(*verts[i]);
        if (test > max) {
            max = test;
            idx = i;
        }
    }
    if (0 != val)
        *val = max;
    return idx;
}

int Simplex::low(double* val) const
{
    double test;
    double min = functionObject->f(*verts[0]);;
    int idx = 0;
    for (int i=1; i<nverts; i++) {
        test = functionObject->f(*verts[i]);
        if (test < min) {
            min = test;
            idx = i;
        }
    }
    if (0 != val)
        *val = min;
    return idx;
}

void Simplex::centroid(Vec_DP& vec) const
{
    Vec_DP ce(dim);
    int hi = high();

    for (int i=0; i<nverts; i++)
        if (i != hi)
            ce += *verts[i];

    vec = ce / (nverts-1);
}

// Size is defined to be sum of distances from centroid to
// all points in simplex.
double Simplex::size() const
{
    Vec_DP ce(dim);
    centroid(ce);
    double dp, size = 0;

    for (int i=0; i<nverts; i++) {
        dp = dot(*verts[i]-ce, *verts[i]-ce);
        size += sqrt(dp);
    }
    return size;
}

int Simplex::reflect()
{
    int hi = high();
    Vec_DP ce(dim);

    centroid(ce);
    *verts[hi] = ce + (ce - *verts[hi]);
    return hi;
}

int Simplex::reflect_exp()
{
    int hi = high();
    Vec_DP ce(dim);

    centroid(ce);
    *verts[hi] = ce + 2*(ce - *verts[hi]);
    return hi;
}

int Simplex::contract()
{
    int hi = high();
    Vec_DP ce(dim);

    centroid(ce);
    *verts[hi] = ce + 0.5*(*verts[hi] - ce);
    return hi;
}

void Simplex::reduce()
{
    int lo = low();

    for (int i = 0; i<nverts; i++) {
        if (i != lo) {
            *verts[i] = 0.5 * (*verts[i] + *verts[lo]);
        }
    }
}


