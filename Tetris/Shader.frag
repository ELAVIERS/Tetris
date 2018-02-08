#version 330
uniform sampler2D u_texture;

in vec2 f_uv;

out vec4 colour;

void main() {
	colour = vec4(texture(u_texture, f_uv).rgb, 1);
}
