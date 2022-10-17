#version 330 core
out vec4 FragColor;

in float Height; // from -16 to 48
in vec3 Normal;

vec3 c_smoothstep(vec3 c1, vec3 c2, float curh, float minh, float maxh) {
	float t = (curh - minh) / (maxh - minh);
	float v1 = t*t;
	float v2 = 1.0 - (1.0 - t) * (1.0 - t);
	return mix(c1, c2, mix(v1, v2, t));
}

void main()
{
	vec3 color;
	vec3 ppeak = vec3(213.0, 213.0, 213.0) / 255;
	vec3 peak = vec3(168.0,198.0,249.0) / 255;
	vec3 dirt = vec3(79.0, 68.0, 42.0) / 255;
	vec3 mtn = vec3(100.0, 100.0, 100.0) / 255;

	vec3 grass = vec3(72.0, 111.0, 56.0) / 255;
	vec3 sand = vec3(208.0,191.0,146.0) / 255;
	vec3 water = vec3(0.0, 117.0, 119.0) / 255;
	vec3 dwater = vec3(35.0, 55.0, 110.0) / 255;

	if (Height >= 70) {
		color = c_smoothstep(peak, ppeak, Height, 70.0, 80.0);
	} else if (Height >= 50.0) {
		color = c_smoothstep(mtn, peak, Height, 50.0, 70.0);
	} else if (Height >= 23.5) {
		color = c_smoothstep(dirt, mtn, Height, 23.5, 50.0);
	} else if (Height >= 3.5) {
		color = c_smoothstep(grass, dirt, Height, 3.5, 23.5);
	} else if (Height >= 0.0) {
		color = c_smoothstep(sand, grass, Height, 0, 3.5);
	} else if (Height >= -4.0) {
		color = c_smoothstep(water, sand, Height, -4, 0);
	} else {
		color = c_smoothstep(dwater, water, Height, -16, -4);
	}
	FragColor = vec4(color, 1.0);
}