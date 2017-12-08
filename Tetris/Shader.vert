#version 330
in vec2 V_Position;
in vec2 V_UV;

out vec2 F_UV;

void main() {
	F_UV = V_UV;
	gl_Position = vec4(V_Position, 0, 1);
}
