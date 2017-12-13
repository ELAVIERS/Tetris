#version 330
uniform sampler2D S_Texture;

in vec2 F_UV;

out vec4 colour;

void main() {
	colour = vec4(1, 1, 1, texture(S_Texture, F_UV).r);
}
