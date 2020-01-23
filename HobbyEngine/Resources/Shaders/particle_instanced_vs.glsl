#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 posSize;
layout(location = 2) in vec4 color;

out vec2 UV;
out vec4 particleColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
	vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 position = posSize.xyz;
	float size = posSize.w;
	vec3 vertexPosition_worldspace = position + cameraRight * vertex.x * size + cameraUp * vertex.y * size;

	UV = vertex.xy + vec2(0.5, 0.5);
	particleColor = color;
	mat4 VP = projection * view;
	gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);;
}