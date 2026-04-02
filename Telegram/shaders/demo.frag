#version 450

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Params {
	vec2 viewport;
	float time;
	float _pad;
};

void main() {
	vec2 uv = v_texcoord;
	vec2 p = uv * 4.0 - 2.0;

	float a = time * 0.8;
	float c = cos(a);
	float s = sin(a);
	p = mat2(c, -s, s, c) * p;

	float d = length(p);
	float wave1 = sin(p.x * 3.0 + time * 2.0) * 0.5 + 0.5;
	float wave2 = sin(p.y * 2.5 - time * 1.5) * 0.5 + 0.5;
	float wave3 = sin(d * 5.0 - time * 3.0) * 0.5 + 0.5;
	float ring = smoothstep(0.3, 0.0, abs(d - 1.0 + 0.3 * sin(time)));

	float r = wave1 * 0.4 + wave3 * 0.3 + ring * 0.6;
	float g = wave2 * 0.3 + wave3 * 0.4 + ring * 0.2;
	float b = wave1 * 0.2 + wave2 * 0.5 + ring * 0.8;

	vec3 col = vec3(r, g, b);
	col = pow(col, vec3(0.85));

	fragColor = vec4(col, 1.0);
}
