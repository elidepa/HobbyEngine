#version 330 core
layout(location = 0) in vec3 pos;

out vec2 UV;
out vec4 particleColor;

uniform vec3 position;
uniform float size;
uniform vec4 color;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
	vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 vertexPosition_worldspace = position + cameraRight * pos.x * size + cameraUp * pos.y * size;

	UV = pos.xy + vec2(0.5, 0.5);
	particleColor = color;
	mat4 VP = projection * view;
	gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);;
}