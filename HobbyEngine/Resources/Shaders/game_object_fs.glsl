#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D reflectionMap;
	float shininess;
};

struct Light {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 FragPos;
in vec2 UV;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform vec3 viewPos;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;
uniform samplerCube skybox;
uniform Material material;
uniform Light light;

void main() 
{
	vec3 ambient = light.ambient * texture(material.diffuse, UV).rgb;

	vec3 normal = texture(normalMap, UV).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(TBN * normal);

	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.diffuse, UV).rgb;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.specular, UV).rgb;

	vec3 projCoords = (FragPosLightSpace.xyz / FragPosLightSpace.w) * 0.5 + 0.5;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.001);
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += projCoords.z - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

	if (projCoords.z > 1.0) {
		shadow = 0.0;
	}

	vec3 R = reflect(-viewDir, normal);
	vec3 reflection = texture(skybox, R).rgb * texture(material.reflectionMap, UV).rgb;

	vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular + 0.5 * reflection);
	
	FragColor = vec4(lighting, 1.0);
}