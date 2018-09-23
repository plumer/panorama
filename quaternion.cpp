#include "quaternion.h"
#include <cmath>
#include <cassert>
#include <iostream>

Quaternion::Quaternion()
	:xyz{ Vector3f::XBASE }, w{ 0.0f } {}

inline Quaternion::Quaternion(const Vector3f & v, float w)
	: xyz{ v }, w{ w } {}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HELPERS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline constexpr float TO_RADIAN(float degree) {
	return degree * (3.1415926535897932384626f / 180.0f);
}

static inline constexpr float TO_DEGREE(float radian) {
	return radian * (180.0f / 3.1415926535897932384626f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<< HELPERS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Quaternion
Quaternion::operator+(const Quaternion &rhs) const {
	return Quaternion{ xyz + rhs.xyz, w + rhs.w };
}

Quaternion
Quaternion::operator-(const Quaternion &rhs) const {
	return Quaternion{ xyz - rhs.xyz, w - rhs.w };
}

Quaternion
Quaternion::operator*(const Quaternion &rhs) const {
	float w_ = w*rhs.w - xyz*rhs.xyz;
	Vector3f xyz_ = xyz^rhs.xyz + rhs.xyz*w + xyz*rhs.w;
	return Quaternion{ xyz_, w_ };
}

Quaternion
Quaternion::operator/(float x) const {
	assert(x != 0.0);
	float inv = 1.0f / x;
	return Quaternion{ xyz*inv, w*inv };
}

Quaternion &
Quaternion::operator+=(const Quaternion &rhs) {
	xyz += rhs.xyz; w += rhs.w;
	return *this;
}

Quaternion &
Quaternion::operator-=(const Quaternion &rhs) {
	xyz -= rhs.xyz; w -= rhs.w;
	return *this;
}

Quaternion &
Quaternion::operator*=(const Quaternion &rhs) {
	float w_ = w*rhs.w - xyz*rhs.xyz;
	Vector3f xyz_ = xyz^rhs.xyz + rhs.xyz*w + xyz*rhs.w;
	this->xyz = xyz_; this->w = w_;
	return *this;
}

Quaternion
Quaternion::conjugate() const {
	return Quaternion{ -xyz, w };
}

float Quaternion::lengthSquared() const {
	// length squared is defined as norm2 = q.conj * q.
	// in this case, norm2.xyz = 0 (cross prod = 0) and norm2.w = w*w-xyz*(-xyz) = w*w+xyz*xyz;
	return xyz*xyz + w*w;
}

float Quaternion::length() const {
	return std::sqrt(lengthSquared());
}

Quaternion Quaternion::reciprocal() const {
	float norm = this->length();
	if (norm != 1.0f) {
		return Quaternion{ -xyz / norm, w / norm };
	}
	else
		return Quaternion{ -xyz, w };
}

void Quaternion::normalize() {
	float l = this->length();
	if (l != 1.0f) {
		xyz /= l;
		w /= l;
	}
}

Quaternion Quaternion::normalized() const {
	float l = this->length();
	if (l != 1.0f)
		return Quaternion{ xyz / l, w / l };
	else
		return *this;
}

Quaternion
Quaternion::rotater(const Vector3f &axis, float degree) {
	Vector3f a = axis.normalized();
	float sinT = std::sin(TO_RADIAN(degree/2));
	float cosT = std::cos(TO_RADIAN(degree/2));
	a *= sinT;
	return Quaternion{ a, cosT };
}

Matrix3f Quaternion::toRotMatrix() const
{
	// TODO: what if the quaternion is not unit-length?
	float x = xyz.x, y = xyz.y, z = xyz.z;
	Vector3f col1{ 1 - 2 * (y*y + z*z), 2 * (x*y - z*w), 2 * (x*z + y*w) };
	Vector3f col2{ 2 * (x*y + z*w), 1 - 2 * (x*x + z*z), 2 * (y*z - x*w) };
	Vector3f col3{ 2 * (z*x - y*w), 2 * (y*z + x*w), 1 - 2 * (x*x + y*y) };
	return Matrix3f{ col1, col2, col3 };
}

Quaternion Quaternion::slerp(Quaternion q0, Quaternion q1, float t)
{
	q0.normalize();
	q1.normalize();
	float cosT = q0.dot(q1);
	if (cosT < 0) { cosT = -cosT; q1 = -q1; }
	if (cosT > 0.9995f) {
		return q0 * (1 - t) + q1 * t;
	}
	else {
		float theta = std::acos(cosT);
		Quaternion qPerp = q1 - q0*cosT;
		qPerp.normalize();
		return q0*std::cos(theta*t) + qPerp * std::sin(theta*t);
	}
}

void Quaternion::fromRotMatrix(const Matrix3f &m) {
	Matrix3f id = m*m.transposed();
	Matrix3f diff = id - Matrix3f::IDENTITY;
	if (diff.frobeniusNorm() < 0.001) {
		// trace(m) = 3-2(y2 + z2 + x2 + z2 + x2 + y2) = 3-4(x^2+y^2+z^2);
		// xyz*xyz = 0.25*(3-trace)
		const Vector3f &c0 = m.col(0), &c1 = m.col(1), &c2 = m.col(2);
		float vnorm = 0.5f*std::sqrt(3 - c0[0] - c1[1] - c2[2]);
		float zw = 0.25f*(c1[0] - c0[1]);
		float yw = 0.25f*(c0[2] - c2[0]);
		float xw = 0.25f*(c2[1] - c1[2]);
		Vector3f xyz_{ xw, yw, zw };
		// {xw, yw, zw} = {x, y, z} * w.
		float w_ = xyz_.length() / vnorm;
		if (std::isnan(w_)) {
			this->xyz = xyz_;
			this->w = 1.0f;
		} else {
			xyz_ /= w_;
			this->xyz = xyz_;
			this->w = w_;
			this->normalize();
		}
	}
	else
		std::cout << "not a unitary matrix\n";
}

std::ostream & operator << (std::ostream &os, const Quaternion &q) {
	std::ios::sync_with_stdio();
	printf("[%5.2fi %5.2fj %5.2fk + %5.2f]", q.xyz.x, q.xyz.y, q.xyz.z, q.w);
	return os;
}