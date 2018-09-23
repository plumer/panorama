#version 330 core

in vec3 vsout_color;
out vec4 color;

void main() {
	color = vec4(vsout_color, 1.0);
}