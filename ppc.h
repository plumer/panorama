#pragma once

#include "geometry.h"
#include <glm/fwd.hpp>
class FrameBuffer;


// the PPC class uses right-handed system. x goes right, y goes up, z goes outside the monitor.
class PPC {
public:
	// Vector a: pixel vector along right direction
	// Vector b: pixel vector along up direction
	Vector3f a, b, c;
	Point3f C;
	int w, h; // in number of pixels

	// >>>>>>>>>>>>>>>>>>> Set camera intrinsics and extrinsics >>>>>>>>>>>>>>>>>>

	// Initializes w and h, hfov is horizontal field of view in degrees
	PPC(int _w, int _h, float hfov);
	void Translate(const Vector3f & tv) {
		C += tv;
	};
	
	// perform "lookAt" positioning.
	void PositionAndOrient(const Point3f & newC, const Point3f & target, const Vector3f & up);
	
	// rotate the camera around
	// - up direction
	void Pan(float degrees);
	// - right direction
	void Tilt(float theta);
	// - view direction
	void Roll(float theta);
	
	// interpolate the two cameras (ppc0 and ppc1) by _fracf_.
	// What about using spherical linear interpolation?
	void SetInterpolated(PPC *ppc0, PPC *ppc1, float fracf);
	
	// <<<<<<<<<<<<<<<<<<<< Set camera intrinsics and extrinsics <<<<<<<<<<<<<<<<<<

	float GetHFOV() const;
	// Project arbitrary P onto the image plane, returned in projP,
	// as [u, v, 1/w]. [u,v] is the image coordinate in #pixels, w is distance to the point.
	// return false if _P_ is behind the camera.
	bool Project(const Point3f & P, Point3f *projP) const;
	

	// get focus length: dist(C, image-plane)
	float GetF() const;

	// view direction
	Vector3f GetVD() const;


	// Draw the camera (like Blender does)
	// maybe, just return a single VAO, or a list of vertices.
	//void Visualize(float vf, PPC *vppc, FrameBuffer *fb);

	// Compute the point that appears on the image plane in coordinate [u,v], with a given distance z.
	Point3f GetPoint(float uf, float vf, float z) const;

	// Inverse operation against _Project(p, &pp)_ method.
	Point3f Unproject(const Point3f & pp) const;

	
	//void SaveToTextFile(char *fname);
	//void LoadFromTextFile(char *fname);

	// Do necessary computation and set glFrustum.
	// We can return a glm::mat4 instead.
	//void SetIntrinsicsHW(float nearZ, float farZ) const;
	// Do necessary computation and set gluLookAt().
	// We can also return a glm::mat4 instead.
	//void SetExtrinsicsHW() const;

	glm::mat4 GetProjectionTrans(float nearZ, float farZ) const;
	glm::mat4 GetViewTrans() const;

	//float CorrespondenceError(V3 uv0, PPC *ppc1, V3 uv1);

	// >>>> I/O >>>>
	void SaveToTextFile(const char * filename) const;
	void LoadFromTextFile(const char * filename);

};