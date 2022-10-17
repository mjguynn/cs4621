#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

out float Height;
out vec3 Norm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	Height = aPos.y;
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	Norm = aNorm; 
}