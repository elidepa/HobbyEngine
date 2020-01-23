#ifndef PARTICLE_SYSTEM_COMPONENT_H
#define PARTICLE_SYSTEM_COMPONENT_H

#include <vector>

#include <glm/glm.hpp>
#include <tinyxml2/tinyxml2.h>

#include "GameObject.h"

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec4 color;
	float size;
	int life;
};

class ParticleSystemComponent : public IGOComponent
{
public:
	ParticleSystemComponent() = default;
	~ParticleSystemComponent();

	virtual bool init(tinyxml2::XMLElement* data);
	virtual void postInit() override;
	virtual ComponentId componentId() const { return COMPONENT_ID; }

	virtual void update(int deltaTime);

	void activate();
	void stop();

	uint32_t texture() { return m_texture; }
	uint32_t vao() { return m_VAO; }
	uint32_t vbo() { return m_VBO; }
	uint32_t positionSizeBuffer() { return m_positionSizeBuffer; }
	uint32_t colorBuffer() { return m_colorBuffer; }
	uint32_t bufferSize() { return m_gpuBufferSize; }
	int maxParticles() { return m_maxParticles; }

	Particle* particles() { return m_particles; }

private:
	void updateParticle(Particle& particle, int deltaTime);
	void spawnParticle();

	const ComponentId COMPONENT_ID = "ParticleSystemComponent";

	// TODO: should this be something else than a raw array?
	Particle* m_particles;

	uint32_t m_texture;
	uint32_t m_VAO;
	uint32_t m_VBO;
	uint32_t m_positionSizeBuffer;
	uint32_t m_colorBuffer;
	float* m_positionSizeBufferData;
	float* m_colorBufferData;
	uint32_t m_gpuBufferSize;

	int m_maxParticles;
	int m_initialSpawn;
	float m_spawnRate;
	float m_spawnTimer;

	bool m_hasLifetime;
	int m_initialSystemLife;
	int m_life;
	bool m_activateAfterInit;
	bool m_isActive;
	bool m_hasActiveParticles;

	// TODO: add different random functions for initial parameter calculations
	glm::vec4 m_color;
	glm::vec4 m_colorRandomness;
	glm::vec4 m_colorChange;
	glm::vec3 m_initialVelocity;
	glm::vec3 m_initialVelocityRandomness;
	glm::vec3 m_acceleration;
	glm::vec3 m_accelerationRandomness;
	glm::vec3 m_initialOffset;
	glm::vec3 m_initialOffsetRandomness;
	float m_size;
	float m_sizeRandomness;
	int m_particleLife;
	int m_particleLifeRandomness;
};

IGOComponent* createParticleSystemComponent();

#endif // !PARTICLE_SYSTEM_COMPONENT_H
