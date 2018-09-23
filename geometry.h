#pragma once
#include <iosfwd>

class Point3f;

class Vector3f {
public:
	float x, y, z;
	Vector3f();
	Vector3f(float, float, float);
	explicit Vector3f(const Point3f &);
	float & operator [] (int i)       { return (&x)[i]; }
	float   operator [] (int i) const { return (&x)[i]; }

	Vector3f operator + (const Vector3f & rhs) const;
	Vector3f operator - (const Vector3f & rhs) const;
	Vector3f operator * (float s) const;
	Vector3f operator / (float s) const;

	Vector3f & operator += (const Vector3f & rhs);
	Vector3f & operator -= (const Vector3f & rhs);
	Vector3f & operator *= (float s);
	Vector3f & operator /= (float s);

	// Dot-product and cross-product
	float    operator * (const Vector3f & rhs) const;
	Vector3f operator ^ (const Vector3f & rhs) const;
	float    dot(const Vector3f & rhs) const { return (*this)*rhs; }
	Vector3f cross(const Vector3f & rhs) const { return (*this) ^ rhs; }

	// return vector in opposite direction
	Vector3f operator - () const;

	float length() const;
	float lengthSquared() const { return x*x + y*y + z*z; }
	void normalize();
	Vector3f normalized() const;


	static const Vector3f XBASE;
	static const Vector3f YBASE;
	static const Vector3f ZBASE;
	static const Vector3f &XDIR;
	static const Vector3f &YDIR;
	static const Vector3f &ZDIR;
};

Vector3f operator * (float s, const Vector3f & v);


// class Point3f

class Point3f {
public:
	float x, y, z;
	Point3f();
	Point3f(float, float, float);
	explicit Point3f(const Vector3f &);

	float & operator [] (int i) { return (&x)[i]; }
	float   operator [] (int i) const { return (&x)[i]; }

	// point +/+= vec, point -/-= vec
	Point3f operator + (const Vector3f & rhs) const;
	Point3f operator - (const Vector3f & rhs) const;

	Point3f & operator += (const Vector3f & rhs);
	Point3f & operator -= (const Vector3f & rhs);

	// point - point
	Vector3f operator - (const Point3f & rhs) const;

	// static methods
	static Point3f origin() { return Point3f(); }
	static const Point3f ORIGIN;
};

Point3f operator + (const Vector3f & v, const Point3f & p);

class Matrix3f {
public:
	Vector3f columns[3];

	Matrix3f();
	Matrix3f(const Vector3f &c0, const Vector3f &c1, const Vector3f &c2);

	// I don't think M+N, M-N will be used a lot...
	Matrix3f operator + (const Matrix3f & rhs) const;
	Matrix3f operator - (const Matrix3f & rhs) const;
	Matrix3f operator * (const Matrix3f & rhs) const;

	// multiplication is in the order of (*this) * (rhs).

	Vector3f operator * (const Vector3f & v) const;

	Matrix3f & operator += (const Matrix3f & rhs);
	Matrix3f & operator -= (const Matrix3f & rhs);
	Matrix3f & operator *= (const Matrix3f & rhs);

	// one can only get a copy of a row of matrix
	Vector3f   row(int x) const;

	// however a reference to a column is accessible.
	Vector3f & col(int x) { return columns[x]; }
	const Vector3f & col(int x) const { return columns[x]; }

	void transpose();
	Matrix3f transposed() const;
	void invert();
	Matrix3f inverted() const;

	float frobeniusNorm() const;

	static Matrix3f rotateX(float degree);
	static Matrix3f rotateY(float degree);
	static Matrix3f rotateZ(float degree);
	static Matrix3f rotate(const Vector3f & axis, float degree);

	static Matrix3f lookAt(const Point3f & origin, const Point3f & target, const Vector3f & up);
	static Matrix3f lookAt(const Point3f & origin, const Vector3f & viewDir, const Vector3f & up);

	static const Matrix3f IDENTITY;
};

std::ostream & operator << (std::ostream &, const Point3f & p);
std::ostream & operator << (std::ostream &, const Vector3f & v);
std::ostream & operator << (std::ostream &, const Matrix3f & m);