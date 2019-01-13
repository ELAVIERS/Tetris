#version 330
uniform sampler2D u_texture;
uniform vec3 u_colour;
uniform bool u_masked;

in vec2 f_uv;

out vec4 colour;

void main() {
	float alpha = (texture(u_texture, f_uv).rgb == vec3(0) && u_masked) ? 0.f : 1.f;
	colour = vec4(u_colour * texture(u_texture, f_uv).r, alpha);
}
