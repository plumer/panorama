#version 330 core

//in vec3 vsout_color;
out vec4 color;
smooth in vec3 fragPos;

uniform sampler2D tex0;

uniform vec3 C;
uniform mat3 abc;


void main() {

	// project fragPos onto the image plane.
	// C + (au+bv+c)w = fragPos, C + (au+bv+c) = imagePos;
	// (uw,vw,w) = inverse(abc)*(fragPos-C)
	// (u,v) = (uw,vw)/w.
	// texCoord = (u/width, v / height)
	vec3 uvw = inverse(abc)*(fragPos-C);
	uvw /= uvw.z;

	// in PPC, image plane has the origin at top-left corner,
	// while in openGL, origin is at bottom-left corner
	// so we need this 1.0-y.
	vec2 uv = vec2(uvw.x / 1024.0, 1.0-uvw.y / 768.0);
	vec3 texColor = vec3(texture(tex0, uv));

	color = vec4(vec3(0.2, 0.5, 0.7)*0.6+texColor*0.4, 1.0);
}