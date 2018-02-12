#version 330
uniform mat3 u_projection;
uniform mat3 u_transform;
uniform vec2 u_uvoffset;

in vec2 v_position;
in vec2 v_uv;

out vec2 f_uv;

void main() {
	f_uv = u_uvoffset + v_uv;
	gl_Position = vec4(u_projection * u_transform * vec3(v_position, 1), 1);
}
