#include "Renderer.h"

#include <algorithm>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tinyxml2/tinyxml2.h>

#include "../Engine/GLApplication.h"
#include "../GameObjects/ParticleSystemComponent.h"
#include "../GameObjects/TransformComponent.h"
#include "../GameObjects/RenderComponent.h"
#include "../ResourceCache/ResourceCache.h"
#include "../UI/TextElement.h"
#include "../UI/UIElement.h"
#include "../Utils/DebugLogger.h"

#ifdef LOG_LEVEL_DEBUG
#define CHECK_GL_ERR() {\
	GLenum err;\
	while ((err = glGetError()) != GL_NO_ERROR) {\
		DebugLogger::log("OpenGL error: " +  std::to_string(err));\
	}\
}
#else
#define CHECK_GL_ERR()
#endif // LOG_LEVEL_DEBUG

uint32_t createProgram(tinyxml2::XMLElement* vertexShaderElement, tinyxml2::XMLElement* fragmentShaderElement) {
	auto vertexShaderFile = vertexShaderElement->Attribute("file");
	auto fragmentShaderFile = fragmentShaderElement->Attribute("file");

	if (!vertexShaderFile || !fragmentShaderFile) {
		LOG_DEBUG("Renderer::init: could not get vertex or fragment shader file attribute");
		return -1;
	}

	Resource vertexShaderResource(vertexShaderFile);
	Resource fragmentShaderResource(fragmentShaderFile);

	auto vertexShaderHandle = Game::instance().resourceCache().getHandle(vertexShaderResource);
	auto fragmentShaderHandle = Game::instance().resourceCache().getHandle(fragmentShaderResource);

	if (!vertexShaderHandle || !fragmentShaderHandle) {
		LOG_DEBUG("Renderer::init: could not get vertex or fragment shader resource handles");
		return -1;
	}

	uint32_t vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertexSource = (const char*)vertexShaderHandle->buffer;
	glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	glCompileShader(vertexShader);
	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
#ifdef LOG_LEVEL_DEBUG
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		DebugLogger::log("Renderer::init: could not compile vertex shader:\n" + std::string(infoLog));
#endif // LOG_LEVEL_DEBUG
		return -1;
	}

	uint32_t fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragmentSource = (const char*)fragmentShaderHandle->buffer;
	glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
#ifdef LOG_LEVEL_DEBUG
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		DebugLogger::log("Renderer::init: could not compile fragment shader:\n" + std::string(infoLog));
#endif // LOG_LEVEL_DEBUG
		return -1;
	}

	uint32_t program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
#ifdef LOG_LEVEL_DEBUG
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		DebugLogger::log("Renderer::init: could not link shader program:\n" + std::string(infoLog));
#endif // LOG_LEVEL_DEBUG
		return -1;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

Renderer::~Renderer()
{
	glDeleteProgram(m_program);
	glDeleteProgram(m_skyboxProgram);
	glDeleteProgram(m_shadowDepthMapProgram);
	glDeleteProgram(m_particleProgram);
	glDeleteProgram(m_uiProgram);

	glDeleteFramebuffers(1, &m_shadowDepthMapFBO);
	glDeleteTextures(1, &m_shadowDepthMap);

#ifdef RENDER_DEBUG
	glDeleteProgram(m_shadowDepthMapDebugProgram);

	glDeleteBuffers(1, &m_debugQuadVAO);
	glDeleteBuffers(1, &m_debugQuadVBO);
#endif // RENDER_DEBUG

}

bool Renderer::init(uint32_t screenWidth, uint32_t screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	glViewport(0, 0, m_screenWidth, m_screenHeight);

	Resource configResource("RendererConfig.xml");
	auto configHandle = Game::instance().resourceCache().getHandle(configResource);

	if (!configHandle) {
		LOG_DEBUG("Renderer::init: could not get renderer config");
		return false;
	}

	tinyxml2::XMLError err;
	tinyxml2::XMLDocument doc;
	if ((err = doc.Parse(configHandle->buffer)) != tinyxml2::XML_SUCCESS) {
		LOG_DEBUG("Renderer::init: Could not parse XML, error: " + err + std::string("\n") + configHandle->buffer);
		return false;
	}
	auto root = doc.FirstChildElement();

	auto particleVertexShaderElement = root->FirstChildElement("ParticleVertexShader");
	auto particleFragmentShaderElement = root->FirstChildElement("ParticleFragmentShader");

	if (!particleVertexShaderElement || !particleFragmentShaderElement) {
		LOG_DEBUG("Renderer::init: could not find particle vertex or fragment shader elements");
		return false;
	}
	m_particleProgram = createProgram(particleVertexShaderElement, particleFragmentShaderElement);
	if (m_particleProgram == -1) {
		return false;
	}

	if (particleVertexShaderElement->Attribute("instanced")) {
		m_particlesInstanced = true;
	}

	auto vertexShaderElement = root->FirstChildElement("VertexShader");
	auto fragmentShaderElement = root->FirstChildElement("FragmentShader");

	if (!vertexShaderElement || !fragmentShaderElement) {
		LOG_DEBUG("Renderer::init: could not find vertex or fragment shader elements");
		return false;
	}

	m_program = createProgram(vertexShaderElement, fragmentShaderElement);
	if (m_program == -1) {
		return false;
	}

	// Skybox shader

	auto skyboxVertexShaderElement = root->FirstChildElement("SkyboxVertexShader");
	auto skyboxFragmentShaderElement = root->FirstChildElement("SkyboxFragmentShader");

	if (skyboxVertexShaderElement && skyboxFragmentShaderElement) {
		m_skyboxProgram = createProgram(skyboxVertexShaderElement, skyboxFragmentShaderElement);
		if (m_skyboxProgram == -1) {
			return false;
		}
	}

	// Shadow depth map shader and FBOs

	glGenFramebuffers(1, &m_shadowDepthMapFBO);

	glGenTextures(1, &m_shadowDepthMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowDepthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowDepthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	auto shadowMapVertexShaderElement = root->FirstChildElement("ShadowMapVertexShader");
	auto shadowMapFragmentShaderElement = root->FirstChildElement("ShadowMapFragmentShader");

	if (!shadowMapVertexShaderElement || !shadowMapFragmentShaderElement) {
		LOG_DEBUG("Renderer::init: could not find shadow map vertex or fragment shader elements");
		return false;
	}
	m_shadowDepthMapProgram = createProgram(shadowMapVertexShaderElement, shadowMapFragmentShaderElement);
	if (m_shadowDepthMapProgram == -1) {
		return false;
	}

	auto uiVertexShaderElement = root->FirstChildElement("UIVertexShader");
	auto uiFragmentShaderElement = root->FirstChildElement("UIFragmentShader");

	if (!uiVertexShaderElement || !uiFragmentShaderElement) {
		LOG_DEBUG("Renderer::init: could not find UI vertex or fragment shader elements");
		return false;
	}
	m_uiProgram = createProgram(uiVertexShaderElement, uiFragmentShaderElement);
	if (m_uiProgram == -1) {
		return false;
	}

#ifdef RENDER_DEBUG
	auto shadowMapDebugVertexShaderElement = root->FirstChildElement("ShadowMapDebugVertexShader");
	auto shadowMapDebugFragmentShaderElement = root->FirstChildElement("ShadowMapDebugFragmentShader");

	if (!shadowMapDebugVertexShaderElement || !shadowMapDebugFragmentShaderElement) {
		LOG_DEBUG("Renderer::init: could not find shadow map vertex or fragment shader elements");
		return false;
	}
	m_shadowDepthMapDebugProgram = createProgram(shadowMapDebugVertexShaderElement, shadowMapDebugFragmentShaderElement);
	if (m_shadowDepthMapDebugProgram == -1) {
		return false;
	}

	float quadVertices[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	glGenVertexArrays(1, &m_debugQuadVAO);
	glGenBuffers(1, &m_debugQuadVBO);
	glBindVertexArray(m_debugQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_debugQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
#endif // RENDER_DEBUG

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	return true;
}

bool Renderer::renderScene(Scene& scene)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	Camera camera = scene.camera();

	// First pass: shadow depth map
	// Switch to correct framebuffer
	glViewport(0, 0, 2048, 2048);
	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowDepthMapFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);
	
	renderShadowDepthMap(camera, scene);

	// Switch back to default rendering config
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glCullFace(GL_BACK);
	glViewport(0, 0, m_screenWidth, m_screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Second pass: skybox
	if (scene.skybox() && m_skyboxProgram != -1) {
		renderSkybox(scene);
	}

	// Third pass: game objects
	renderGameObjects(scene);

	// Fourth pass: particle systems
	renderParticleSystems(scene);

	// Fifth pass: UI elements
	renderUIElements(scene);

#ifdef RENDER_DEBUG
	// Debug pass: shadow map
	renderShadowMapDebug();
#endif // RENDER_DEBUG

	Game::instance().swapBuffers();
	return true;
}

void Renderer::updateScreenSize(uint32_t screenWidth, uint32_t screenHeight)
{
	m_screenHeight = screenHeight;
	m_screenWidth = screenWidth;
	glViewport(0, 0, m_screenWidth, m_screenHeight);
}

bool Renderer::renderGameObjects(Scene& scene)
{
	glUseProgram(m_program);

	glm::mat4 view = scene.camera().viewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(m_program, "view"), 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(m_program, "viewPos"), 1, glm::value_ptr(scene.camera().position));
	glUniformMatrix4fv(glGetUniformLocation(m_program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(scene.lightSpaceMatrix()));

	glUniform1i(glGetUniformLocation(m_program, "skybox"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox()->texture());

	for (auto it = scene.gameObjects().begin(); it != scene.gameObjects().end(); ++it) {
		auto go = *it;
		if (go->findComponent<RenderComponent>("RenderComponent").lock()) {
			renderGameObject(*go, scene);
		}
	}
	return true;
}

bool Renderer::renderGameObject(GameObject& gameObject, Scene& scene)
{
	auto transformComponent = gameObject.findComponent<TransformComponent>("TransformComponent").lock();
	auto renderComponent = gameObject.findComponent<RenderComponent>("RenderComponent").lock();

	glm::mat4 model = transformComponent->getTransformMatrix();
	glUniformMatrix4fv(glGetUniformLocation(m_program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
	glUniformMatrix3fv(glGetUniformLocation(m_program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// Setup material
	glUniform1i(glGetUniformLocation(m_program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(m_program, "material.specular"), 1);
	glUniform1i(glGetUniformLocation(m_program, "material.reflectionMap"), 5);
	glUniform1f(glGetUniformLocation(m_program, "material.shininess"), renderComponent->material().shininess);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderComponent->material().diffuseMap);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderComponent->material().specularMap);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, renderComponent->material().reflectionMap);

	// Setup normal map

	glUniform1i(glGetUniformLocation(m_program, "normalMap"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderComponent->normalMap());

	// Setup shadow map

	glUniform1i(glGetUniformLocation(m_program, "shadowMap"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepthMap);

	// Setup reflection map

	// Setup lighting
	auto lighting = scene.lighting();
	glUniform3fv(glGetUniformLocation(m_program, "light.direction"), 1, glm::value_ptr(lighting.direction));
	glUniform3fv(glGetUniformLocation(m_program, "light.ambient"), 1, glm::value_ptr(lighting.ambient));
	glUniform3fv(glGetUniformLocation(m_program, "light.diffuse"), 1, glm::value_ptr(lighting.diffuse));
	glUniform3fv(glGetUniformLocation(m_program, "light.specular"), 1, glm::value_ptr(lighting.specular));
	
	// Setup VAO and model data
	glBindVertexArray(renderComponent->vao());

	glBindBuffer(GL_ARRAY_BUFFER, renderComponent->vbo());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, renderComponent->uvs());
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, renderComponent->normals());
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, renderComponent->tangents());
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, renderComponent->bitangents());
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderComponent->ebo());

	// Render
	glDrawElements(GL_TRIANGLES, renderComponent->nIndices(), GL_UNSIGNED_SHORT, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(renderComponent->vao());

	return true;
}

bool Renderer::renderSkybox(Scene& scene)
{
	glUseProgram(m_skyboxProgram);

	glDepthMask(GL_FALSE);

	glm::mat4 view = glm::mat4(glm::mat3(scene.camera().viewMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(m_skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(m_skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform1i(glGetUniformLocation(m_skyboxProgram, "skybox"), 0);

	glBindVertexArray(scene.skybox()->vao());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox()->texture());

	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);
	return true;
}

bool Renderer::renderShadowDepthMap(Camera& camera, Scene& scene)
{
	glUseProgram(m_shadowDepthMapProgram);

	glUniformMatrix4fv(glGetUniformLocation(m_shadowDepthMapProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(scene.lightSpaceMatrix()));

	for (auto it = scene.gameObjects().begin(); it != scene.gameObjects().end(); ++it) {
		auto go = *it;
		if (go->findComponent<RenderComponent>("RenderComponent").lock()) {
			renderShadowDepthMapGO(*go);
		}
	}
	return true;
}

bool Renderer::renderShadowDepthMapGO(GameObject& gameObject)
{
	auto transformComponent = gameObject.findComponent<TransformComponent>("TransformComponent").lock();
	auto renderComponent = gameObject.findComponent<RenderComponent>("RenderComponent").lock();

	glm::mat4 model = transformComponent->getTransformMatrix();
	glUniformMatrix4fv(glGetUniformLocation(m_shadowDepthMapProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(renderComponent->vao());

	glBindBuffer(GL_ARRAY_BUFFER, renderComponent->vbo());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderComponent->ebo());

	glDrawElements(GL_TRIANGLES, renderComponent->nIndices(), GL_UNSIGNED_SHORT, (void*)0);

	glDisableVertexAttribArray(0);

	return true;
}

bool Renderer::renderParticleSystems(Scene& scene)
{
	glUseProgram(m_particleProgram);

	glm::mat4 view = scene.camera().viewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(m_particleProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)m_screenWidth / (float)m_screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(m_particleProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glUniform3fv(glGetUniformLocation(m_particleProgram, "cameraUp"), 1, glm::value_ptr(cameraUp));

	glm::vec3 cameraRight = glm::normalize(glm::cross(scene.camera().front(), cameraUp));
	glUniform3fv(glGetUniformLocation(m_particleProgram, "cameraRight"), 1, glm::value_ptr(cameraUp));

	for (auto it = scene.gameObjects().begin(); it != scene.gameObjects().end(); ++it) {
		auto go = *it;
		if (go->findComponent<ParticleSystemComponent>("ParticleSystemComponent").lock()) {
			renderParticleSystem(*go, scene.camera());
		}
	}
	return true;
}

bool Renderer::renderParticleSystem(GameObject& gameObject, Camera& camera)
{
	auto particleSystem = gameObject.findComponent<ParticleSystemComponent>("ParticleSystemComponent").lock();

	glUniform1i(glGetUniformLocation(m_particleProgram, "particleTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particleSystem->texture());

	if (m_particlesInstanced) {
		glBindVertexArray(particleSystem->vao());

		glBindBuffer(GL_ARRAY_BUFFER, particleSystem->vbo());
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, particleSystem->positionSizeBuffer());
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, particleSystem->colorBuffer());
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(2);

		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particleSystem->bufferSize());
	}
	else {
		Particle* particles = particleSystem->particles();

		for (int i = 0; i < particleSystem->maxParticles(); ++i) {
			Particle& p = particles[i];

			if (p.life > 0) {
				glUniform3fv(glGetUniformLocation(m_particleProgram, "position"), 1, glm::value_ptr(p.position));
				glUniform1f(glGetUniformLocation(m_particleProgram, "size"), p.size);
				glUniform4fv(glGetUniformLocation(m_particleProgram, "color"), 1, glm::value_ptr(p.color));

				glBindVertexArray(particleSystem->vao());

				glBindBuffer(GL_ARRAY_BUFFER, particleSystem->vbo());
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				glEnableVertexAttribArray(0);

				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
		}
	}

	return true;
}

bool Renderer::renderUIElements(Scene& scene)
{
	glUseProgram(m_uiProgram);

	glm::mat4 projection = glm::ortho(0.0f, (float)m_screenWidth, 0.0f, (float)m_screenHeight);
	glUniformMatrix4fv(glGetUniformLocation(m_uiProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	auto uiElements = scene.uiElements();
	for (auto it = uiElements.begin(); it != uiElements.end(); ++it) {
		std::shared_ptr<UIElement> pElement = *it;
		if (std::string(pElement->elementType()) == "TextElement") {
			std::shared_ptr<TextElement> pTextElem = std::static_pointer_cast<TextElement>(pElement);
			renderTextElement(pTextElem);
		}
		else {
			LOG_DEBUG("Renderer::renderUIElements: couldn't render UI element - couldn't recognize element type.");
		}
	}

	return true;
}

bool Renderer::renderTextElement(std::shared_ptr<TextElement> elem)
{
	glUniform4fv(glGetUniformLocation(m_uiProgram, "textColor"), 1, glm::value_ptr(elem->color()));
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(m_uiProgram, "glyphTexture"), 0);
	glBindVertexArray(elem->vao());

	auto font = elem->font();
	float x = elem->position().x;
	float y = elem->position().y;
	float scale = elem->textScale();

	std::string text = elem->text();
	for (auto c = text.begin(); c != text.end(); ++c) {
		std::shared_ptr<Character> ch = font->glyphs()[*c];

		float xpos = x + ch->bearing.x * scale;
		float ypos = y - (ch->size.y - ch->bearing.y) * scale;
		float w = ch->size.x * scale;
		float h = ch->size.y * scale;

		float vertices[] = {
			xpos, ypos + h, 0.0, 0.0,
			xpos, ypos, 0.0, 1.0,
			xpos + w, ypos, 1.0, 1.0,
			xpos, ypos + h, 0.0, 0.0,
			xpos + w, ypos, 1.0, 1.0,
			xpos + w, ypos + h, 1.0, 0.0
		};

		glBindTexture(GL_TEXTURE_2D, ch->textureID);
		glBindBuffer(GL_ARRAY_BUFFER, elem->vbo());
		glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * 4 * sizeof(float), vertices);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch->advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

#ifdef RENDER_DEBUG
bool Renderer::renderShadowMapDebug()
{
	glViewport(0, 0, 400, 300);
	glUseProgram(m_shadowDepthMapDebugProgram);

	glUniform1i(glGetUniformLocation(m_shadowDepthMapDebugProgram, "shadowMap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepthMap);

	glUniform1f(glGetUniformLocation(m_shadowDepthMapDebugProgram, "near_plane"), -10.0f);
	glUniform1f(glGetUniformLocation(m_shadowDepthMapDebugProgram, "far_plane"), 20.0f);

	glBindVertexArray(m_debugQuadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	return true;
}
#endif // RENDER_DEBUG
