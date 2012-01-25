// classes for vectors and matrices
// From D. Yang: C++ and Object-Oriented Numeric Computing,
// Springer-Verlag, 2000
//
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "yang.h"

void error(const char* v) {
  std::cout << v << "\n";
  exit(1);
}

// ***********  definitions of members in class Vtr.

Vtr::Vtr(int n, double* abd) {
  ets = new double [lenth =n];
  for (int i = 0; i < lenth; i++)  ets[i]= *(abd +i);
}

Vtr::Vtr(int n, double a) {
  ets = new double [lenth =n];
  for (int i = 0; i < lenth; i++)  ets[i] = a;
}

Vtr::Vtr(const Vtr & v) {
  ets = new double [lenth = v.lenth];
  for (int i = 0; i < lenth; i++)  ets[i] = v[i];
}





Vtr& Vtr::operator=(const Vtr& v) {
  if (this != &v) {
    if (lenth != v.lenth ) error("Vtr::op=: Error: Bad vector sizes");
    for (int i = 0; i < lenth; i++) ets[i] = v[i];
  }
  return *this;
}

Vtr & Vtr::operator+=(const Vtr& v) {
  if (lenth != v.lenth ) error("bad vector sizes");
  for (int i = 0; i < lenth; i++) ets[i] += v[i];
  return *this;
}

Vtr & Vtr::operator-=(const Vtr& v) {
  if (lenth != v.lenth ) error("bad vtor sizes");
  for (int i = 0; i < lenth; i++) ets[i] -= v[i];
  return *this;
}

Vtr operator+(const Vtr & v) {  // usage: v1 = + v2;
  return v;
}

Vtr operator-(const Vtr& v) {    // usage: v1 = - v2;
  return Vtr(v.lenth) - v;
}

Vtr operator+(const Vtr& v1, const Vtr & v2) {          // v=v1+v2
  if (v1.lenth != v2.lenth ) error("Vtr::op+: Error Bad vtor sizes");
  Vtr sum = v1; // It would cause problem without copy constructor
  sum += v2;
  return sum;
}

Vtr operator-(const Vtr& v1, const Vtr& v2) { // v=v1-v2
//  if (v1.lenth != v2.lenth ) error("bad vtor sizes");
  Vtr sum = v1; // It would cause problem without copy constructor
  sum -= v2;
  return sum;
}

std::ostream&  operator<<(std::ostream& s, const Vtr& v ) {
  for (int i =0; i < v.lenth; i++ ) {
    s << v[i] << "  ";
    if (i%10 == 9) s << "\n";
  }
  return s;
}

double Vtr::twonorm() const{
  double norm = std::abs(ets[0]);
  for (int i = 1; i < lenth; i++) {
    double vi = std::abs(ets[i]);
    if (norm < 100) norm = std::sqrt(norm*norm + vi*vi);
    else {  // to avoid overflow for fn "sqrt" when norm is large
      double tm = vi/norm;
      norm *= std::sqrt(1.0 + tm*tm);
    }
  }
  return norm;
}

double Vtr::maxnorm() const {
  double norm = std::abs(ets[0]);
  for (int i = 1; i < lenth; i++)
    if (norm < std::abs(ets[i])) norm = std::abs(ets[i]);
  return norm;
}

Vtr operator*(const double scalar, const Vtr & v) {
  Vtr tm(v.lenth);
  for (int i = 0; i < v.lenth; i++) tm[i] = scalar*v[i];
  return tm;
}

Vtr operator*(const Vtr & v1, const Vtr & v2) {
  int sz = v1.lenth;
  if (sz != v2.lenth ) error("bad vtor sizes");
  Vtr tm(sz);
  for (int i = 0; i < sz; i++)
      tm[i] = v1[i]*v2[i];
  return tm;
}

Vtr operator/(const Vtr & v, const double scalar) {
  if (scalar == 0) error("division by zero in vector-scalar division");
  return (1.0/scalar)*v;
}

double dot(const Vtr & v1, const Vtr & v2) {
  int sz = v1.lenth;
  if (sz != v2.lenth ) error("bad vtor sizes");
  double tm = v1[0]*v2[0];
  for (int i = 1; i < sz; i++) tm += v1[i]*v2[i];
  return tm;
}

// Matrix matrix

Mtx::Mtx(int n, int m, double** dbp) {         // construct from double pointer
  nrows = n;
  ncols = m;
  ets = new double* [nrows];
  for (int i =  0; i < nrows; i++) {
    ets[i] = new double [ncols];
    for (int j = 0; j < ncols; j++) ets[i][j] = dbp[i][j];
  }
}

Mtx::Mtx(int n, int m, double a) {              // construct from a double
  ets = new double* [nrows = n];
  for (int i =  0; i< nrows; i++) {
    ets[i] = new double [ncols = m];
    for (int j = 0; j < ncols; j++) ets[i][j] = a;
  }
}

Mtx::Mtx(const Mtx & mat) {                      // copy constructor
  ets = new double* [nrows = mat.nrows];
  for (int i =  0; i< nrows; i++) {
    ets[i] = new double [ncols = mat.ncols];
    for (int j = 0; j < ncols; j++) ets[i][j] = mat[i][j];
  }
}

Mtx::~Mtx(){                                    // destructor
  for (int i = 0; i< nrows; i++) delete[]  ets[i];
  delete[] ets;
}

Mtx& Mtx::operator=(const Mtx& mat) {           // copy assignment
  if (this != &mat) {
    if (nrows != mat.nrows || ncols != mat.ncols) error("bad matrix sizes");
    for (int i = 0; i < nrows; i++)
      for (int j = 0; j < ncols; j++) ets[i][j]  = mat[i][j];
  }
  return *this;
}

Mtx& Mtx::operator+=(const Mtx&  mat) {         // add-assign
  if (nrows != mat.nrows || ncols != mat.ncols) error("bad matrix sizes");
  for (int i = 0; i < nrows; i++)
    for (int j = 0; j < ncols; j++) ets[i][j] += mat[i][j];
  return *this;
}

Mtx& Mtx::operator-=(const Mtx&  mat) {         // subtract-assign
  if (nrows != mat.nrows || ncols != mat.ncols) error("bad matrix sizes");
  for (int i = 0; i < nrows; i++)
    for (int j = 0; j < ncols; j++) ets[i][j] -= mat[i][j];
  return *this;
}

Mtx& Mtx::operator+() {                         // usage: mat1 = + mat2;
  return *this;
}

Mtx operator-(const Mtx &  mat) {               // usage: mat1 = - mat2;
  return Mtx(mat.nrows,mat.ncols) - mat;
}

Mtx Mtx::operator+(const Mtx & mat) {           // usage: m = m1 + m2
 Mtx sum = *this;                               // user-defined copy constructor
 sum += mat;                                    // is important here
 return sum;                                    // otherwise m1 would be changed
}

Vtr Mtx::operator*(const Vtr& v) const {        // matrix-vector multiply
  if (ncols != v.size())
	  error("Mtx::op*(Vtr): Error: Mat. and vec. sizes do not match.");
  Vtr tm(nrows);
  for (int i = 0; i < nrows; i++)
    for (int j = 0; j < ncols; j++) tm[i] += ets[i][j]*v[j];
  return tm;
}

Vtr operator*(const Vtr& v, const Mtx& mat)
{
	if (v.lenth != mat.nrows)
		error("op*(Vtr, Mtx): Error: Mat. and vec. size do no match.");
	Vtr res(mat.ncols, 0.0);
	for (int i=0; i<mat.ncols; i++)
		for (int j=0; j<v.lenth; j++)
			res[i] = v.ets[j] * mat.ets[j][i];
	return res;
}


Mtx operator-(const Mtx& m1, const Mtx&  m2) {  // matrix subtract
 if(m1.nrows !=m2.nrows || m1.ncols !=m2.ncols)
   error("bad matrix sizes");
 Mtx sum = m1;
 sum -= m2;
 return sum;
}

// Added by Peter Seidler
void Mtx::getcol(int n, Vtr& vec) const
{
	if (vec.size() != nrows)
		error("Mtx::getcol(): Bad vector size.");
	for (int i = 0; i < nrows; i++)
		vec[i] = ets[i][n];
}

void Mtx::setcol(int n, const Vtr& vec)
{
	if (vec.size() != nrows)
		error("Mtx::getcol(): Bad vector size.");
	for (int i = 0; i < nrows; i++)
		ets[i][n] = vec[i];
}

void Mtx::clear()
{
	for (int i = 0; i < nrows; i++)
		for (int j = 0; j < ncols; j++)
			ets[i][j] = 0;
}

int Mtx::transpose(Mat_DP& dest) const
{
	if ((nrows != dest.cols()) || (ncols != dest.rows())) {
		std::cerr << "Mtx::transpose(): Error: Matrix dim."
		          << std::endl;
		return -1;
	}

	for (int i=0; i<nrows; i++)
		for (int j=0; j<ncols; j++)
			dest(j, i) = ets[i][j];
	return 0;
}

std::ostream& operator<<(std::ostream& s, const Mtx& mat)
{
	for (int i = 0; i < mat.rows(); i++) {
		s << "| ";
		for(int j = 0; j < mat.cols(); j++) {
			s.setf(std::ios_base::fixed, std::ios_base::floatfield);
			s.precision(4);
			s.width(8);
			s << mat.ets[i][j];
		}
		s << " |" << std::endl;
	}
	return s;
}

Mtx Mtx::operator*(const Mtx& mat) const
{
	Mtx tmp(nrows, mat.ncols);

	if (ncols != mat.nrows)
		error("Mtx::op*=: Bad matrix sizes.");

	for (int i = 0; i < nrows; i++)
		for (int j = 0; j < mat.ncols; j++)
			for (int k = 0; k < ncols; k++)
				tmp(i,j) += ets[i][k] * mat.ets[k][j];

	return tmp;
}

int Mtx::QRdecomp(Mtx& Q, Mtx& R)
{
	if ((Q.nrows != nrows) || (Q.ncols != ncols) ||
	    (R.nrows != ncols) || (R.ncols != ncols))   {
		std::cerr << "Mtx::QRdecomp(): Error: Bad matrix size.";
		return -1;
	}

	double** tmp;
	double norm, dot;

	// tmp = A transpose
	int tmprows = ncols;
	int tmpcols = nrows;
	tmp = new double* [tmprows];
	for (int i=0; i<tmprows; i++) {
		tmp[i] = new double [tmpcols];
		for (int j=0; j<tmpcols; j++)
			tmp[i][j] = ets[j][i];
	}
	Mat_DP QT(tmprows,tmpcols);

	R.clear();

	for (int i=0; i<tmprows; i++) {
		norm = 0;
		for (int k=0; k<tmpcols; k++)
			norm += tmp[i][k]*tmp[i][k];
		norm = std::sqrt(norm);
		R.ets[i][i] = norm;
		for (int k=0; k<tmpcols; k++)
			QT.ets[i][k] = tmp[i][k] / norm;

		for (int j=i+1; j<tmprows; j++) {
			dot = 0;
			for (int k=0; k<tmpcols; k++)
				dot += QT.ets[i][k]*tmp[j][k];
			R.ets[i][j] = dot;
			for (int k=0; k<tmpcols; k++)
				tmp[j][k] = tmp[j][k] - dot*QT.ets[i][k];
		}
	}

	QT.transpose(Q);

	for (int i=0; i<tmprows; i++)
		delete[] tmp[i];
	return 0;
}

int Mtx::QRdecomp_slow(Mtx& Q, Mtx& R)
{
	if ((Q.nrows != nrows) || (Q.ncols != ncols) ||
	    (R.nrows != ncols) || (R.ncols != ncols))   {
		std::cerr << "Mtx::QRdecomp(): Error: Bad matrix size.";
		return -1;
	}

	double** tmp;
	double norm, dot;

	tmp = new double* [nrows];
	for (int i=0; i<nrows; i++) {
		tmp[i] = new double [ncols];
		for (int j=0; j<ncols; j++)
			tmp[i][j] = ets[i][j];
	}

	R.clear();

	for (int i=0; i<ncols; i++) {
		norm = 0;
		for (int k=0; k<nrows; k++)
			norm += tmp[k][i]*tmp[k][i];
		norm = std::sqrt(norm);
		R.ets[i][i] = norm;
		for (int k=0; k<nrows; k++)
			Q.ets[k][i] = tmp[k][i] / norm;

		for (int j=i+1; j<ncols; j++) {
			dot = 0;
			for (int k=0; k<nrows; k++)
				dot += Q.ets[k][i]*tmp[k][j];
			R.ets[i][j] = dot;
			for (int k=0; k<nrows; k++)
				tmp[k][j] = tmp[k][j] - dot*Q.ets[k][i];
		}
	}

	for (int i=0; i<nrows; i++)
		delete[] tmp[i];
	return 0;
}


