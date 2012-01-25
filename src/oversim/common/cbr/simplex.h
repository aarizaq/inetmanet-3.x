/* Numerical Methods, S06
 * Note 8, Function optimization using the downhill simplex method.
 * Peter Seidler		*/

#ifndef _SIMPLEX_H_
#define _SIMPLEX_H_

#include "yang.h"


// forward declarations
class CoordCalcFunction;



class Simplex
{
private:
    int nverts;		// Number of vertices.
    Vec_DP** verts;		// Array of pointers to vertices.
    int dim;		// Dimension of points.

public:
    CoordCalcFunction *functionObject;

    Simplex(int dimension);
    ~Simplex();

    Vec_DP& operator[](int i) const {return *verts[i];}

    // Returns different characteristic properties.
    int high(double* val = 0) const;
    int low(double* val = 0) const;
    void centroid(Vec_DP& vec) const;
    double size() const;

    // Operations that can be performed on simplex.
    int reflect();			// Returns index of vertex reflected.
    int reflect_exp();		// Returns index of vertex reflected.
    int contract();			// Returns index of vertex contracted.
    void reduce();
};

/* Finds minimum of f using the downhill simplex method.
 * init: Initial guess on minimum.
 * res:  Point of calculated minimum.
 * accf: Desired accuracy: Max diff. between function values at highest
 *       and lowest point of simplex. On return the variable
 *       is updated with the actual difference.
 * accx: Desired accuracy: Max size of simplex. On return updated with
 *       actual size.
 * nmax: Max no. of iterations. On return updated with actual no.
 *       of iterations.
 * Return value: Function value at minimum.		*/


#endif // _SIMPLEX_H
