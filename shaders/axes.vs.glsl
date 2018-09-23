#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

out vec3 vsout_color;

uniform mat4 proj, view;

void main() {
	vsout_color = color;
	gl_Position = proj * view * vec4(pos, 1.0);
}