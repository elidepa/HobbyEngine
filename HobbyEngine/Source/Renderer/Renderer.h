#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "Camera.h"
#include "../Engine/Scene.h"
#include "../GameObjects/GameObject.h"
#include "Skybox.h"
#include "../UI/TextElement.h"

class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	bool init(uint32_t screenWidth, uint32_t screenHeight);

	bool renderScene(Scene& scene);

	void updateScreenSize(uint32_t screenWidth, uint32_t screenHeight);

private:
	bool renderGameObjects(Scene& scene);
	bool renderGameObject(GameObject& gameObject, Scene& scene);
	bool renderSkybox(Scene& scene);
	bool renderShadowDepthMap(Camera& camera, Scene& scene);
	bool renderShadowDepthMapGO(GameObject& gameObject);
	bool renderParticleSystems(Scene& scene);
	bool renderParticleSystem(GameObject& gameObject, Camera& camera);
	bool renderUIElements(Scene& scene);
	bool renderTextElement(std::shared_ptr<TextElement> elem);

	uint32_t m_program;
	uint32_t m_skyboxProgram;
	uint32_t m_shadowDepthMapProgram;
	uint32_t m_particleProgram;
	uint32_t m_uiProgram;

	uint32_t m_shadowDepthMapFBO;
	uint32_t m_shadowDepthMap;

	uint32_t m_screenWidth;
	uint32_t m_screenHeight;

	bool m_particlesInstanced = false;

#ifdef RENDER_DEBUG
	bool renderShadowMapDebug();

	uint32_t m_shadowDepthMapDebugProgram;

	uint32_t m_debugQuadVAO;
	uint32_t m_debugQuadVBO;
#endif // RENDERER_DEBUG
};

#endif // !RENDERER_H
