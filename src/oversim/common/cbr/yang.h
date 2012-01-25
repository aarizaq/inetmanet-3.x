// classes for vectors and matrices
// From D. Yang: C++ and Object-Oriented Numeric Computing,
// Springer-Verlag, 2000
// 
#ifndef _YANG_H_
#define _YANG_H_


#include <iostream>


class Mtx;

class Vtr {       
  int lenth;               // number of entries in the vector
  double* ets;             // entries of the vector
public: 
  Vtr(int, double*);       // costruct a vector with given entries
  Vtr(int, double = 0);    // a vector with all entries same
  Vtr(const Vtr&);         // copy constructor
  ~Vtr(){ delete[] ets; }  // destructor is defined inline

  Vtr& operator=(const Vtr&);                        // copy assignment 
  Vtr& operator+=(const Vtr &);                      // v += v2
  Vtr& operator-=(const Vtr &);                      // v -= v2
  double& operator[](int i) const { return ets[i]; } // eg v[i] = 10;
  double maxnorm() const;                            // maximum norm
  double twonorm() const;                            // L-2 norm
  int size() const { return lenth; }                 // return length of vector

  friend Vtr operator+(const Vtr&);                  // unary +, v = + v2
  friend Vtr operator-(const Vtr&);                  // unary -, v = - v2
  friend Vtr operator+(const Vtr&, const Vtr&);      // binary +, v = v1+v2
  friend Vtr operator-(const Vtr&, const Vtr&);      // binary -, v = v1-v2
  friend double dot(const Vtr&, const Vtr&);         // dot product
  friend Vtr operator*(const double, const Vtr&);    // vec-scalar x 
  friend Vtr operator*(const Vtr&, const double);    // vec-scalar x
  friend Vtr operator*(const Vtr&, const Vtr&);      // component-wise multiply
  friend Vtr operator*(const Vtr&, const Mtx&);      // vec-matrix x
  friend Vtr operator/(const Vtr&, const double);
  friend std::ostream& operator<<(std::ostream&, const Vtr&); // output operator
};

inline Vtr operator*(const Vtr & v, const double scalar) {
  return scalar*v; 
}

class Mtx {
private: 
  int nrows;                                   // number of rows in the matrix 
  int ncols;                                   // number of columns in  matrix 
  double** ets;                                // entries of the matrix
public: 
  Mtx(int n, int m, double**);                 // construct an n by m matrix
  Mtx(int n, int m, double d = 0);             // all entries equal d
  Mtx(const Mtx &);                            // copy constructor
  ~Mtx();                                      // destructor

  Mtx& operator=(const Mtx&);                  // overload =
  Mtx& operator+=(const Mtx&);                 // overload +=
  Mtx& operator-=(const Mtx&);                 // overload -=
  Vtr operator*(const Vtr&) const;             // matrix vector multiply
  double* operator[](int i) const { return ets[i]; }      // subscripting, row i
  
  // Implement plus as a member fcn. minus could also be so implemented.
  // But I choose to implement minus as a friend just to compare the difference 
  Mtx& operator+();                            // unary +, eg, mat1 = + mat2
  Mtx operator+(const Mtx&);                   // binary +, eg, m = m1 + m2
  friend Mtx operator-(const Mtx&);            // unary -, eg, m1 = -m2
  friend Mtx operator-(const Mtx&,const Mtx&); // binary -, eg, m = m1 - m2

  // Added by Peter Seidler...
  int rows() const {return nrows;}
  int cols() const {return ncols;}
  void getcol(int i, Vtr& vec) const;
  void setcol(int i, const Vtr& vec);
  void clear();
  int transpose(Mtx& dest) const;
  Mtx operator*(const Mtx&) const; 
  friend Vtr operator*(const Vtr&, const Mtx&);    // vec-matrix x
  const double& operator()(int i, int j) const {return ets[i][j];}
  double& operator()(int i, int j) {return ets[i][j];}
  friend std::ostream& operator<<(std::ostream&, const Mtx&);	

  int QRdecomp(Mtx& Q, Mtx& R);
  int QRdecomp_slow(Mtx& Q, Mtx& R);
};

typedef Vtr Vec_DP;
typedef Mtx Mat_DP;

#endif // _YANG_H_
