#include "ppc.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static inline float constexpr TO_RADIANS(float degree) {
	return degree * (3.14159265358979 / 180.0f);
}

PPC::PPC(int _w, int _h, float hfov)
	: w(_w), h(_h), a{ Vector3f::XBASE }, b{ -Vector3f::YBASE }, C{ 0, 0, 0 }
{
	float halfFov = TO_RADIANS(hfov*0.5);
	c = Vector3f{ -w*0.5f, h*0.5f, -w*0.5f / std::tanf(halfFov) };
}

float PPC::GetHFOV() const {
	// tan(hfov / 2) = (w / 2) / f, where w is image width, f is dist(C, image-plane).
	float f = GetF();
	float halfFov = std::atan(float(w) * 0.5 / f);
	return 2.0 * halfFov * 180.0f / 3.14159265358979f;
}

bool PPC::Project(const Point3f &p, Point3f *projP) const {
	// p = C + (au + bv + c)*w = C + M[uw,vw,w]
	Matrix3f M{ a, b, c };
	Vector3f x = M.inverted() * (p - C);
	if (x[2] <= 0.0) return false;

	*projP = Point3f(x / x[2]);
	projP->z = 1.0 / x[2];
	return true;
}

void PPC::PositionAndOrient(const Point3f & newC, const Point3f & target, const Vector3f & up) {
	// viewDir points outwards, so viewDir is in -z direction.
	Vector3f viewDir = (target - newC);
	viewDir.normalize(); // new z axis
	// rightDir is x axis, and x = y ^ z, or up ^ (-viewDir), or viewDir ^ up.
	Vector3f rightDir = viewDir ^ up;
	rightDir.normalize();

	// upDir is y axis, and y = z ^ x, or (-vD) ^ rD, or rd ^ vD.
	Vector3f upDir = rightDir ^ viewDir;
	upDir.normalize();

	float f = GetF();

	C = newC;
	a = rightDir * a.length();
	b = -upDir * b.length();
	c = f * viewDir - h * 0.5 * b - w * 0.5 * a;

	GetF();
}

float PPC::GetF() const {
	float f = c * GetVD();
	assert(f >= 0);
	return f;
}

// we have cross(a, -b) = -vd, or cross(a, b) = vd.
Vector3f PPC::GetVD() const {
	return (a^b).normalized();
}

void PPC::Pan(float degrees) {
	Matrix3f rot = Matrix3f::rotate(-b, degrees);
	a = rot*a;
	c = rot*c;
}

void PPC::Tilt(float degrees) {
	Matrix3f rot = Matrix3f::rotate(a, degrees);
	b = rot*b; 
	c = rot*c;
}

void PPC::Roll(float degrees) {
	Matrix3f rot = Matrix3f::rotate(GetVD(), degrees);
	a = rot*a;
	b = rot*b;
	c = rot*c;
}

Point3f PPC::GetPoint(float uf, float vf, float z) const {
	return C + (a*uf + b * vf + c) * z / GetF();
}

Point3f PPC::Unproject(const Point3f & pp) const
{
	return C + (a*pp[0] + b*pp[1] + c) / pp[2];
}

glm::mat4 PPC::GetProjectionTrans(float nearZ, float farZ) const {
	float horizonFov = GetHFOV();
	float aspRatio = float(w) / float(h);
	return glm::perspective(glm::radians(horizonFov / aspRatio), aspRatio, nearZ, farZ);
}

glm::mat4 PPC::GetViewTrans() const {
	Vector3f viewDir = GetVD();
	Point3f target = C + viewDir;
	
	glm::vec3 from{ C.x, C.y, C.z };
	glm::vec3 to{ target.x, target.y, target.z };
	glm::vec3 up{ -b.x, -b.y, -b.z };

	return glm::lookAt(from, to, up);
}

void PPC::SaveToTextFile(const char *fname) const {

	std::ofstream ofs(fname, std::ios::trunc);
	ofs << a.x << ' ' << a.y << ' ' << a.z << std::endl;
	ofs << b.x << ' ' << b.y << ' ' << b.z << std::endl;
	ofs << c.x << ' ' << c.y << ' ' << c.z << std::endl;
	ofs << C.x << ' ' << C.y << ' ' << C.z << std::endl;
	//ofs << a << std::endl << b << std::endl << c << std::endl << C << std::endl;
	ofs.close();

}

void PPC::LoadFromTextFile(const char *fname) {

	std::ifstream ifs(fname);
	if (ifs.fail()) {
		std::cerr << "INFO: cannot open file: " << fname << std::endl;
		return;
	}
	ifs >> a.x >> a.y  >> a.z;
	ifs >> b.x >> b.y  >> b.z;
	ifs >> c.x >> c.y  >> c.z;
	ifs >> C.x >> C.y  >> C.z;
	ifs.close();
}