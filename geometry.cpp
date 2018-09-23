#include "geometry.h"
#include <cmath>
//#include <iostream>
#include <iomanip>
#include <cassert>

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>> HELPERS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static inline constexpr float TO_RADIAN(float degree) {
	return degree * (3.1415926535897932384626 / 180.0);
}

static inline constexpr float TO_DEGREE(float radian) {
	return radian * (180.0 / 3.1415926535897932384626);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<< HELPERS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Vector3f::Vector3f() : x(0), y(0), z(0) {}
Vector3f::Vector3f(float _x, float _y, float _z) :
	x{ _x }, y{ _y }, z{ _z } {}
Vector3f::Vector3f(const Point3f &p) :
	x{ p.x }, y{ p.y }, z{ p.z } {}

Vector3f Vector3f::operator+(const Vector3f & rhs) const
{
	return Vector3f{ x + rhs.x, y + rhs.y, z + rhs.z };
}

Vector3f Vector3f::operator-(const Vector3f & rhs) const {
	return Vector3f{x - rhs.x, y - rhs.y, z - rhs.z};
}

Vector3f Vector3f::operator*(float s) const {
	return Vector3f{ x*s, y*s, z*s };
}

Vector3f Vector3f::operator/(float s) const {
	float inv = 1.0f / s;
	return Vector3f{ x*inv, y*inv, z*inv };
}


Vector3f & Vector3f::operator += (const Vector3f & rhs) {
	x += rhs.x; y += rhs.y; z += rhs.z;
	return *this;
}

Vector3f & Vector3f::operator -= (const Vector3f & rhs) {
	x -= rhs.x; y -= rhs.y; z -= rhs.z;
	return *this;
}

Vector3f & Vector3f::operator*=(float s) {
	x *= s; y *= s; z *= s;
	return *this;
}

Vector3f & Vector3f::operator/=(float s) {
	float inv = 1.0 / s;
	x *= inv; y *= inv; z *= inv;
	return *this;
}

// dot product
float Vector3f::operator*(const Vector3f & rhs) const {
	return x * rhs.x + y * rhs.y + z * rhs.z;
}

Vector3f Vector3f::operator^(const Vector3f & rhs) const
{
	/* | x | y | z |
	   | x'| y'| z'|
	   | i | j | k |
	*/
	float nx = y * rhs.z - z * rhs.y;
	float ny = z * rhs.x - x * rhs.z;
	float nz = x * rhs.y - y * rhs.x;
	return Vector3f{ nx, ny, nz };
}

Vector3f Vector3f::operator - () const {
	return Vector3f{ -x, -y, -z };
}

float Vector3f::length() const {
	return std::sqrt(x * x + y * y + z * z);
}

void Vector3f::normalize() {
	float l = this->length();
	if (l >= 0.0001f) {
		float inv = 1.0 / l;
		x *= inv; y *= inv; z *= inv;
	}
}

Vector3f Vector3f::normalized() const {
	float l = this->length();
	if (l < 0.0001f) return *this;
	else return (*this) / l;
}

// static const for Vector
const Vector3f Vector3f::XBASE{ 1.0, 0.0, 0.0 };
const Vector3f Vector3f::YBASE{ 0.0, 1.0, 0.0 };
const Vector3f Vector3f::ZBASE{ 0.0, 0.0, 1.0 };
const Vector3f &Vector3f::XDIR = XBASE;
const Vector3f &Vector3f::YDIR = YBASE;
const Vector3f &Vector3f::ZDIR = ZBASE;

// scalar * v
Vector3f operator*(float s, const Vector3f & v) {
	return v * s;
}


// >>>>>>>>>>>>>>>>>>>>>>>>>> Point3f >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

Point3f::Point3f() : x(0), y(0), z(0) {}

Point3f::Point3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

Point3f::Point3f(const Vector3f &v) : x(v.x), y(v.y), z(v.z) {}

Point3f Point3f::operator+(const Vector3f & rhs) const {
	return Point3f{ x + rhs.x, y + rhs.y, z + rhs.z };
}

Point3f Point3f::operator-(const Vector3f & rhs) const {
	return Point3f{ x - rhs.x, y - rhs.y, z - rhs.z };
}

Point3f & Point3f::operator += (const Vector3f & v) {
	x += v.x; y += v.y; z += v.z;
	return *this;
}

Point3f & Point3f::operator -= (const Vector3f & v) {
	x -= v.x; y -= v.y; z -= v.z;
	return *this;
}

Vector3f Point3f::operator-(const Point3f & rhs) const {
	return Vector3f{x - rhs.x, y - rhs.y, z - rhs.z};
}

const Point3f Point3f::ORIGIN{ 0.0f, 0.0f, 0.0f };

// <<<<<<<<<<<<<<<<<<<<<<<<<<< Point3f <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// v + p

Point3f operator + (const Vector3f & v, const Point3f & p) {
	return p + v;
}


// >>>>>>>>>>>>>>>>>>>>>>>>>> Natrix3f class >>>>>>>>>>>>>>>>>>>>>>>>>

Matrix3f::Matrix3f() {
	columns[0].x = columns[1].y = columns[2].z = 1.0f;
}

Matrix3f::Matrix3f(const Vector3f &a, const Vector3f &b, const Vector3f &c) {
	columns[0] = a;
	columns[1] = b;
	columns[2] = c;
}

Matrix3f Matrix3f::operator + (const Matrix3f & rhs) const {
	return Matrix3f{
		columns[0] + rhs.columns[0],
		columns[1] + rhs.columns[1],
		columns[2] + rhs.columns[2]
	};
}

Matrix3f Matrix3f::operator - (const Matrix3f & rhs) const {
	return Matrix3f{
		columns[0] - rhs.columns[0],
		columns[1] - rhs.columns[1],
		columns[2] - rhs.columns[2]
	};
}

Matrix3f Matrix3f::operator * (const Matrix3f &rhs) const {
	return Matrix3f{
		(*this)*rhs.columns[0],
		(*this)*rhs.columns[1],
		(*this)*rhs.columns[2]
	};
}



Matrix3f & Matrix3f::operator += (const Matrix3f & rhs) {
	columns[0] += rhs.columns[0];
	columns[1] += rhs.columns[1];
	columns[2] += rhs.columns[2];
	return *this;
}

Matrix3f & Matrix3f::operator -= (const Matrix3f & rhs) {
	columns[0] -= rhs.columns[0];
	columns[1] -= rhs.columns[1];
	columns[2] -= rhs.columns[2];
	return *this;
}

Matrix3f & Matrix3f::operator *= (const Matrix3f & rhs) {
	Matrix3f temp = (*this) * rhs;
	(*this) = temp;
	return *this;
}

Vector3f Matrix3f::operator*(const Vector3f & v) const {
	return columns[0] * v.x + columns[1] * v.y + columns[2] * v.z;
}

Vector3f Matrix3f::row(int x) const {
	return Vector3f{ columns[0][x], columns[1][x], columns[2][x] };
}

void Matrix3f::transpose() {
#define SWAP_CODE9527(a, b) \
	do {decltype(a) t = b; b = a; a = t;} while (0)
	SWAP_CODE9527(columns[0].y, columns[1].x);
	SWAP_CODE9527(columns[0].z, columns[2].x);
	SWAP_CODE9527(columns[1].z, columns[2].y);
#undef SWAP_CODE9527
}

Matrix3f Matrix3f::transposed() const
{
	return Matrix3f{
		Vector3f{columns[0].x, columns[1].x, columns[2].x},
		Vector3f{ columns[0].y, columns[1].y, columns[2].y },
		Vector3f{ columns[0].z, columns[1].z, columns[2].z }
	};
}

void Matrix3f::invert() {
	// I copied this code from Dr. Popescu
	const Vector3f a = row(0), b = row(1), c = row(2);
	Vector3f _a = b ^ c; _a /= (a * _a);
	Vector3f _b = c ^ a; _b /= (b * _b);
	Vector3f _c = a ^ b; _c /= (c * _c);
	columns[0] = _a; columns[1] = _b; columns[2] = _c;
}

Matrix3f Matrix3f::inverted() const
{
	const Vector3f a = row(0), b = row(1), c = row(2);
	Vector3f _a = b ^ c; _a /= (a * _a);
	Vector3f _b = c ^ a; _b /= (b * _b);
	Vector3f _c = a ^ b; _c /= (c * _c);

	return Matrix3f{_a, _b, _c};
}

// static methods
Matrix3f Matrix3f::rotateX(float degree) {
	float radian = TO_RADIAN(degree);
	float cosAngle = std::cosf(radian);
	float sinAngle = std::sinf(radian);

	// c & -s \\ s & c
	return Matrix3f{
		Vector3f::XBASE,
		Vector3f{0, cosAngle, sinAngle},
		Vector3f{0, -sinAngle, cosAngle}
	};
}

Matrix3f Matrix3f::rotateY(float degree) {
	float radian = TO_RADIAN(degree);
	float cosAngle = std::cosf(radian);
	float sinAngle = std::sinf(radian);

	// c & -s \\ s & c
	return Matrix3f{
		Vector3f{cosAngle, 0.0, -sinAngle},
		Vector3f::YBASE,
		Vector3f{ sinAngle, 0.0, cosAngle }
	};
}

Matrix3f Matrix3f::rotateZ(float degree) {
	float radian = TO_RADIAN(degree);
	float cosAngle = std::cosf(radian);
	float sinAngle = std::sinf(radian);

	// c & -s \\ s & c
	return Matrix3f{
		Vector3f{ cosAngle, sinAngle, 0 },
		Vector3f{ -sinAngle, cosAngle, 0 },
		Vector3f::ZBASE
	};
}

Matrix3f Matrix3f::rotate(const Vector3f & axis, float degree) {
	Vector3f a = axis.normalized();
	float sinT = std::sin(TO_RADIAN(degree));
	float cosT = std::cos(TO_RADIAN(degree));
	return Matrix3f{
		Vector3f{a.x*a.x + cosT*(1 - a.x*a.x), a.x*a.y*(1 - cosT) - sinT*a.z, a.x*a.z*(1 - cosT) + sinT*a.y},
		Vector3f{a.x*a.y*(1 - cosT) + sinT*a.z, a.y*a.y + cosT*(1 - a.y*a.y), a.y*a.z*(1 - cosT) - sinT*a.x},
		Vector3f{a.x*a.z*(1 - cosT) - sinT*a.y, a.y*a.z*(1 - cosT) + sinT*a.x, a.z*a.z + cosT*(1 - a.z*a.z)}
	};
}

Matrix3f Matrix3f::lookAt(const Point3f & origin, const Point3f & target, const Vector3f & up) {
	// origin + dir = target
	return Matrix3f::lookAt(origin, target - origin, up);
}

Matrix3f Matrix3f::lookAt(const Point3f & origin, const Vector3f & viewDir, const Vector3f & up) {
	Vector3f z = viewDir.normalized();
	Vector3f x = up^z;
	x.normalize(); // x = cross(y, z)
	Vector3f y = z ^ x;
	y.normalize(); // y = cross(z, x)
	//static int DO_NOT_USE_THIS_CODE = 0;
	//assert(DO_NOT_USE_THIS_CODE);
	return Matrix3f{ x, y, z };
}

float Matrix3f::frobeniusNorm() const
{
	return std::sqrt(
		columns[0].lengthSquared() + columns[1].lengthSquared() + columns[2].lengthSquared());
}


const Matrix3f Matrix3f::IDENTITY{ Vector3f::XBASE, Vector3f::YBASE, Vector3f::ZBASE };

// <<<<<<<<<<<<<<<<<<<<<<<<<< Matrix3f class <<<<<<<<<<<<<<<<<<<<<<<<<




std::ostream & operator<<(std::ostream &os, const Point3f & p)
{
	os << '[' << p.x << ' ' << p.y << ' ' << p.z << ']';
	return os;
}

std::ostream & operator<<(std::ostream &os, const Vector3f & v)
{
	os << '[' << v.x << ' ' << v.y << ' ' << v.z << ']';
	return os;
}

std::ostream & operator<<(std::ostream &os, const Matrix3f & m)
{
	//const Vector3f &a = m.columns[0], &b = m.columns[1], &c = m.columns[2];
	std::ios::sync_with_stdio(true);
	printf("\n| %5.2f %5.2f %5.2f |\n", m.columns[0].x, m.columns[1].x, m.columns[2].x);
	printf("| %5.2f %5.2f %5.2f |\n", m.columns[0].y, m.columns[1].y, m.columns[2].y);
	printf("| %5.2f %5.2f %5.2f |\n", m.columns[0].z, m.columns[1].z, m.columns[2].z);
	return os;
}