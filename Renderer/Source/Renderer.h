#pragma once

#include "Shader.h"
#include "Utils.h"
#include "Camera.h"
#include "ShadowMap.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <mutex>

class Renderer
{
public:
	Renderer();

	// Prepare OpenGL, set shaders, textures, etc
	void Init();

	// Draw a frame using double frame buffer
	void Draw();

	// Process keyboard mouse etc input
	void ProcessInput(GLFWwindow* window);

	// Init GLFW
	void GLPrepare();

	// Finish GLFW
	void GLFinish();

	void BuildLights();
	void BuildSmokes();
	void BuildGeometries();
	void BuildShaders();
	void BuildMaterials();
	void BuildTextures();
	void BuildRenderItems();

	void UpdatePassCb();
	// There is no need to update static objects
	void UpdateObjectCb();
	void UpdateSmoke();

	void DrawOpaque();
	void DrawShadow();
	void DrawLight();
	void DrawSmoke();

	// Callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

public:
	std::mutex phyxMutex;
	int exit = 0;

	std::unordered_map<std::string, std::unique_ptr<Smoke>> smokes;

private:
	TinyddsLoader::DDSFile DDSLoader;

	std::unordered_map<std::string, std::unique_ptr<Shader>> mShaders;
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::vector<std::unique_ptr<RenderItem>> mRenderItems;

	std::vector<RenderItem*> mOpaqueItems;
	std::vector<RenderItem*> mLightItems;
	std::vector<RenderItem*> mSmokeItems;

	std::vector<std::unique_ptr<Light>> mLights;

	PassCb mPassCb;

	std::unique_ptr<ShadowMap> mShadowMap;

	//std::unordered_map<std::string, std::unique_ptr<Camera>> mCameras;

	GLFWwindow* window = nullptr;
	const unsigned int screenWidth = 1900;
	const unsigned int screenHeight = 1080;
};

// =========================================================================
// Static camera and related parameters for callback 
static Camera mCurrentCamera;
// Mouse position at last frame
static float lastX;
static float lastY;
static bool firstMouse = true;
// Timing
static float deltaTime = 0.f;	// time between current frame and last frame
static float lastFrame = 0.f;
