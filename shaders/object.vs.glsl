#version 330 core

layout (location = 0) in vec3 pos;
//layout (location = 1) in vec3 normal;

uniform mat4 proj, view;

//out vec3 vsout_normal;
smooth out vec3 fragPos;

void main() {
	gl_Position = proj * view * vec4(pos, 1.0);
	//vsout_normal = normal;
	fragPos = pos;
}