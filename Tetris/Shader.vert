#version 330
uniform mat3 u_projection;
uniform vec2 u_offset;

in vec2 v_position;
in vec2 v_uv;

out vec2 f_uv;

void main() {
	f_uv = v_uv;
	gl_Position = vec4(u_projection * vec3(v_position + u_offset, 1), 1);
}
