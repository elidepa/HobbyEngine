#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec2 UV;
out mat3 TBN;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform mat3 normalMatrix;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	UV = vec2(aUV.x, aUV.y);

	vec3 T = normalize(vec3(normalMatrix * aTangent));
	vec3 B = normalize(vec3(normalMatrix * aBitangent));
	vec3 N = normalize(vec3(normalMatrix * aNormal));
	TBN = mat3(T, B, N);

	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

	gl_Position = projection * view * vec4(FragPos, 1.0);
}