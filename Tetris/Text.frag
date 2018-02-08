#version 330
uniform sampler2D u_texture;
uniform vec3 u_colour;

in vec2 f_uv;

out vec4 colour;

void main() {
	float alpha = texture(u_texture, f_uv).r;
	colour = vec4(alpha * u_colour.r, alpha * u_colour.g, alpha * u_colour.b, alpha == 0 ? 0 : alpha);
}
