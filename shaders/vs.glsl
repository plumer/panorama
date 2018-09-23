#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

uniform mat4 proj, view, model;

out vec3 vsout_normal;
smooth out vec2 vsout_texCoord;

void main() {
	gl_Position = proj * view * model * vec4(pos, 1.0);
	vsout_texCoord = texCoord;
}