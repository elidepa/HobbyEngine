#include "ParticleSystemComponent.h"

#include <algorithm>
#include <memory>
#include <string>

#include "../Engine/GLApplication.h"
#include "../ResourceCache/ImageLoader.h"
#include "../ResourceCache/ResourceCache.h"
#include "TransformComponent.h"
#include "../Utils/DebugLogger.h"
#include "../Utils/XMLUtils.h"

ParticleSystemComponent::~ParticleSystemComponent()
{
	if (m_particles != nullptr) {
		delete[] m_particles;
		m_particles = nullptr;
	}

	if (m_positionSizeBufferData != nullptr) {
		delete[] m_positionSizeBufferData;
		m_positionSizeBufferData = nullptr;
	}

	if (m_colorBufferData != nullptr) {
		delete[] m_colorBufferData;
		m_colorBufferData = nullptr;
	}

	glDeleteTextures(1, &m_texture);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_positionSizeBuffer);
	glDeleteBuffers(1, &m_colorBuffer);
	glDeleteVertexArrays(1, &m_VAO);
}

bool ParticleSystemComponent::init(tinyxml2::XMLElement* data)
{
	auto particleSystemElem = data->FirstChildElement("ParticleSystem");
	if (!particleSystemElem) {
		LOG_DEBUG("ParticleSystemComponent::init: could not find ParticleSystem element.");
		return false;
	}

	auto initAttrib = particleSystemElem->Attribute("activateAfterInit");
	if (initAttrib && std::string(initAttrib) == std::string("true")) {
		m_activateAfterInit = true;
	}
	else {
		m_activateAfterInit = false;
	}

	auto lifetimeAttrib = particleSystemElem->Attribute("lifetime");
	if (lifetimeAttrib) {
		try
		{
			m_initialSystemLife = std::stoi(lifetimeAttrib);
			m_hasLifetime = true;
		}
		catch (const std::exception&)
		{
			LOG_DEBUG("ParticleSystemComponent::init: could not convert lifetime attribute to int.");
			return false;
		}
	}
	else {
		m_hasLifetime = false;
	}

	auto particlesElem = data->FirstChildElement("Particles");
	if (!particlesElem) {
		LOG_DEBUG("ParticleSystemComponent::init: could not find Particles element.");
		return false;
	}

	if (!XMLUtils::xmlAttribToInt(particlesElem, "maxParticles", m_maxParticles)) {
		return false;
	}

	m_particles = new Particle[m_maxParticles];
	m_positionSizeBufferData = new float[4 * m_maxParticles];
	m_colorBufferData = new float[4 * m_maxParticles];

	if (!XMLUtils::xmlAttribToInt(particlesElem, "initialSpawn", m_initialSpawn)) {
		return false;
	}
	
	if (!XMLUtils::xmlAttribToFloat(particlesElem, "spawnRate", m_spawnRate)) {
		return false;
	}

	if (!XMLUtils::xmlAttribToInt(particlesElem, "life", m_particleLife)) {
		return false;
	}

	if (!XMLUtils::xmlAttribToInt(particlesElem, "lifeRandomness", m_particleLifeRandomness)) {
		m_particleLifeRandomness = 0;
	}

	float r, g, b, a, x, y, z;

	auto colorElem = particlesElem->FirstChildElement("Color");
	if (colorElem && XMLUtils::xmlRGBAAttribsToFloat(colorElem, r, g, b, a)) {
		m_color = glm::vec4(r, g, b, a);
	}
	else {
		m_color = glm::vec4(1.0f);
	}

	auto colorRandomnessElem = particlesElem->FirstChildElement("ColorRandomness");
	if (colorRandomnessElem && XMLUtils::xmlRGBAAttribsToFloat(colorRandomnessElem, r, g, b, a)) {
		m_colorRandomness = glm::vec4(r, g, b, a);
	}
	else {
		m_colorRandomness = glm::vec4(0.0f);
	}

	auto colorChangeElem = particlesElem->FirstChildElement("ColorChange");
	if (colorChangeElem && XMLUtils::xmlRGBAAttribsToFloat(colorChangeElem, r, g, b, a)) {
		m_colorChange = glm::vec4(r, g, b, a);
	}
	else {
		m_colorChange = glm::vec4(0.0f);
	}

	auto velocityElem = particlesElem->FirstChildElement("Velocity");
	if (velocityElem && XMLUtils::xmlXYZAttribsToFloat(velocityElem, x, y, z)) {
		m_initialVelocity = glm::vec3(x, y, z);
	}
	else {
		m_initialVelocity = glm::vec3(0.0f);
	}

	auto velocityRandomnessElem = particlesElem->FirstChildElement("VelocityRandomness");
	if (velocityRandomnessElem && XMLUtils::xmlXYZAttribsToFloat(velocityRandomnessElem, x, y, z)) {
		m_initialVelocityRandomness = glm::vec3(x, y, z);
	}
	else {
		m_initialVelocityRandomness = glm::vec3(0.0f);
	}

	auto accelerationElem = particlesElem->FirstChildElement("Acceleration");
	if (accelerationElem && XMLUtils::xmlXYZAttribsToFloat(accelerationElem, x, y, z)) {
		m_acceleration = glm::vec3(x, y, z);
	}
	else {
		m_acceleration = glm::vec3(0.0f);
	}

	auto accelerationRandomnessElem = particlesElem->FirstChildElement("AccelerationRandomness");
	if (accelerationRandomnessElem && XMLUtils::xmlXYZAttribsToFloat(accelerationRandomnessElem, x, y, z)) {
		m_accelerationRandomness = glm::vec3(x, y, z);
	}
	else {
		m_accelerationRandomness = glm::vec3(0.0f);
	}

	auto offsetElem = particlesElem->FirstChildElement("Offset");
	if (offsetElem && XMLUtils::xmlXYZAttribsToFloat(offsetElem, x, y, z)) {
		m_initialOffset = glm::vec3(x, y, z);
	}
	else {
		m_initialOffset = glm::vec3(0.0f);
	}

	auto offsetRandomnessElem = particlesElem->FirstChildElement("OffsetRandomness");
	if (offsetRandomnessElem && XMLUtils::xmlXYZAttribsToFloat(offsetRandomnessElem, x, y, z)) {
		m_initialOffsetRandomness = glm::vec3(x, y, z);
	}
	else {
		m_initialOffsetRandomness = glm::vec3(0.0f);
	}

	auto sizeElem = particlesElem->FirstChildElement("Size");
	if (sizeElem && XMLUtils::xmlAttribToFloat(sizeElem, "size", x)) {
		m_size = x;
	}
	else {
		m_size = 0.01f;
	}

	auto sizeRandomnessElem = particlesElem->FirstChildElement("SizeRandomness");
	if (sizeRandomnessElem && XMLUtils::xmlAttribToFloat(sizeRandomnessElem, "size", x)) {
		m_sizeRandomness = x;
	}
	else {
		m_sizeRandomness = 0.0025f;
	}

	auto textureElem = data->FirstChildElement("Texture");
	if (!textureElem) {
		LOG_DEBUG("ParticleSystemComponent::init: could not find Texture element.");
		return false;
	}

	auto textureFilename = textureElem->Attribute("file");
	if (!textureFilename) {
		LOG_DEBUG("ParticleSystemComponent::init: could not find file attribute in Texture element.");
		return false;
	}
	auto imageRes = Resource(textureFilename);
	auto imageHandle = Game::instance().resourceCache().getHandle(imageRes);
	if (!imageHandle) {
		LOG_DEBUG("ParticleSystemComponent::init: could not get handle for texture resource.");
		return false;
	}
	auto imageData = std::static_pointer_cast<ImageResProcessedData>(imageHandle->processedData);

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageData->width(), imageData->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imageHandle->buffer);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenVertexArrays(1, &m_VAO);

	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &m_positionSizeBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionSizeBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * m_maxParticles * sizeof(float), NULL, GL_STREAM_DRAW);

	glGenBuffers(1, &m_colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * m_maxParticles * sizeof(float), NULL, GL_STREAM_DRAW);

	return true;
}

void ParticleSystemComponent::postInit()
{
	if (m_activateAfterInit) {
		activate();
	}
}

void ParticleSystemComponent::update(int deltaTime)
{
	if (!m_hasActiveParticles) {
		return;
	}

	for (int i = 0; i < m_maxParticles; ++i) {
		m_positionSizeBufferData[4 * i + 0] = 0;
		m_positionSizeBufferData[4 * i + 1] = 0;
		m_positionSizeBufferData[4 * i + 2] = 0;
		m_positionSizeBufferData[4 * i + 3] = 0;
		m_colorBufferData[4 * i + 0] = 0;
		m_colorBufferData[4 * i + 1] = 0;
		m_colorBufferData[4 * i + 2] = 0;
		m_colorBufferData[4 * i + 3] = 0;
	}

	m_gpuBufferSize = 0;

	for (int i = 0; i < m_maxParticles; ++i) {
		if (m_particles[i].life > 0) {
			updateParticle(m_particles[i], deltaTime);
		}
	}

	if (m_isActive) {
		m_spawnTimer += m_spawnRate * deltaTime;
		while (m_spawnTimer > 1.0f) {
			spawnParticle();
			m_spawnTimer -= 1.0f;
		}
	}

	Camera& camera = m_owner->scene->camera();

	std::sort(m_particles, m_particles + m_maxParticles,
		[&camera](const Particle& p1, const Particle& p2) -> bool {
			return glm::length(p1.position - camera.position) > glm::length(p2.position - camera.position);
		});

	int activeParticles = 0;
	for (int i = 0; i < m_maxParticles; ++i) {
		if (m_particles[i].life > 0) {
			activeParticles++;
			Particle& p = m_particles[i];

			m_positionSizeBufferData[4 * i + 0] = p.position.x;
			m_positionSizeBufferData[4 * i + 1] = p.position.y;
			m_positionSizeBufferData[4 * i + 2] = p.position.z;
			m_positionSizeBufferData[4 * i + 3] = p.size;

			m_colorBufferData[4 * i + 0] = p.color.r;
			m_colorBufferData[4 * i + 1] = p.color.g;
			m_colorBufferData[4 * i + 2] = p.color.b;
			m_colorBufferData[4 * i + 3] = p.color.a;

			m_gpuBufferSize++;
		}
	}

	if (activeParticles == 0 && !m_isActive) {
		m_hasActiveParticles = false;
		return;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_positionSizeBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * m_maxParticles * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * m_gpuBufferSize * sizeof(float), m_positionSizeBufferData);

	glBindBuffer(GL_ARRAY_BUFFER, m_colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, 4 * m_maxParticles * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * m_gpuBufferSize * sizeof(float), m_colorBufferData);

	if (m_hasLifetime) {
		m_life -= deltaTime;
		if (m_life <= 0) {
			stop();
		}
	}
}

void ParticleSystemComponent::activate()
{
	m_isActive = true;
	m_hasActiveParticles = true;

	if (m_hasLifetime) {
		m_life = m_initialSystemLife;
	}

	for (int i = 0; i < m_initialSpawn; ++i) {
		spawnParticle();
	}
}

void ParticleSystemComponent::stop()
{
	m_isActive = false;

	if (m_hasLifetime) {
		m_life = -1;
	}
}

void ParticleSystemComponent::updateParticle(Particle& particle, int deltaTime)
{
	particle.life -= deltaTime;
	if (particle.life < 0) {
		return;
	}

	particle.velocity += particle.acceleration * (deltaTime / 1000.0f);
	particle.position += particle.velocity * (deltaTime / 1000.0f);

	particle.color += m_colorChange * (deltaTime / 1000.0f);
}

void ParticleSystemComponent::spawnParticle()
{
	int newParticleIdx = 0;
	int minLife = m_particles[0].life;

	for (int i = 0; i < m_maxParticles; ++i) {
		if (m_particles[i].life < 0) {
			newParticleIdx = i;
			break;
		}
		if (m_particles[i].life < minLife) {
			newParticleIdx = i;
			minLife = m_particles[i].life;
		}
	}

	Particle& p = m_particles[newParticleIdx];

	p.life = m_particleLife + ((rand() % 100) / 100.0f * m_particleLifeRandomness);

	auto transformComponent = m_owner->findComponent<TransformComponent>("TransformComponent").lock();
	float x = m_initialOffset.x + ((rand() % 100) / 100.0f * m_initialOffsetRandomness.x) - m_initialOffsetRandomness.x / 2.0f;
	float y = m_initialOffset.y + ((rand() % 100) / 100.0f * m_initialOffsetRandomness.y) - m_initialOffsetRandomness.x / 2.0f;
	float z = m_initialOffset.z + ((rand() % 100) / 100.0f * m_initialOffsetRandomness.z) - m_initialOffsetRandomness.x / 2.0f;
	p.position = glm::vec3(transformComponent->getTransformMatrix() * glm::vec4(x, y, z, 1.0f));

	x = m_acceleration.x + ((rand() % 100) / 100.0f * m_accelerationRandomness.x) - m_accelerationRandomness.x / 2.0f;
	y = m_acceleration.y + ((rand() % 100) / 100.0f * m_accelerationRandomness.y) - m_accelerationRandomness.x / 2.0f;
	z = m_acceleration.z + ((rand() % 100) / 100.0f * m_accelerationRandomness.z) - m_accelerationRandomness.x / 2.0f;
	p.acceleration = glm::vec3(x, y, z);

	x = m_initialVelocity.x + ((rand() % 100) / 100.0f * m_initialVelocityRandomness.x) - m_initialVelocityRandomness.x / 2.0f;
	y = m_initialVelocity.y + ((rand() % 100) / 100.0f * m_initialVelocityRandomness.y) - m_initialVelocityRandomness.y / 2.0f;
	z = m_initialVelocity.z + ((rand() % 100) / 100.0f * m_initialVelocityRandomness.z) - m_initialVelocityRandomness.z / 2.0f;
	p.velocity = glm::vec3(x, y, z);

	float r = m_color.r + ((rand() % 100) / 100.0f * m_colorRandomness.r) - m_colorRandomness.r / 2.0f;
	float g = m_color.g + ((rand() % 100) / 100.0f * m_colorRandomness.g) - m_colorRandomness.g / 2.0f;
	float b = m_color.b + ((rand() % 100) / 100.0f * m_colorRandomness.b) - m_colorRandomness.b / 2.0f;
	float a = m_color.a + ((rand() % 100) / 100.0f * m_colorRandomness.a) - m_colorRandomness.a / 2.0f;
	p.color = glm::vec4(r, g, b, a);

	p.size = m_size + ((rand() % 100) / 100.0f * m_sizeRandomness) - m_sizeRandomness / 2.0f;
}

IGOComponent* createParticleSystemComponent()
{
	return new ParticleSystemComponent;
}
