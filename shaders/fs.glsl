#version 330 core

in vec2 vsout_texCoord;
out vec4 color;

uniform sampler2D tex0;
uniform float HDRmax;

void main() {
	// we want to show the color in [0, HDRmax],
	// so every color should be scaled by 1/HDRmax.

	color = texture(tex0, vsout_texCoord) / HDRmax;
}
