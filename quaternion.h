#pragma once
#include "geometry.h"

class Quaternion
{
public:
	Quaternion();
	Quaternion(const Vector3f &v, float w);

	static Quaternion rotater(const Vector3f &axis, float degree);

	// internal arithmetic operation
	Quaternion operator + (const Quaternion &rhs) const;
	Quaternion operator - (const Quaternion &rhs) const;
	Quaternion operator * (const Quaternion &rhs) const;
	Quaternion operator / (float) const;	// what does this mean?

	Quaternion operator * (float s) const {
		return Quaternion{ xyz*s, w*s };
	}

	Quaternion operator - () const {
		return Quaternion{ -xyz, -w };
	}

	Quaternion & operator += (const Quaternion &rhs);
	Quaternion & operator -= (const Quaternion &rhs);
	Quaternion & operator *= (const Quaternion &rhs);

	// length, conjugate and reciprocal
	Quaternion conjugate() const;
	void normalize();
	Quaternion normalized() const;
	float length() const;
	float lengthSquared() const;
	Quaternion reciprocal() const;

	// operation on other geometric objects
	Point3f rotate(const Point3f &p);
	void fromRotMatrix(const Matrix3f &m);
	Matrix3f toRotMatrix() const;

	float dot(const Quaternion &other) const {
		return xyz * other.xyz + w*other.w;
	}

	static Quaternion slerp(Quaternion q0, Quaternion q1, float t);

	Vector3f xyz;
	float w;
};

std::ostream & operator << (std::ostream & os, const Quaternion &q);
