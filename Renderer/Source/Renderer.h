#pragma once

#include "Shader.h"
#include "Utils.h"
#include "Camera.h"
#include "FrameBufferObject.h"
#include "MeshLoader.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <mutex>

class Renderer
{
public:
	Renderer();

	// Prepare OpenGL, set shaders, textures, etc
	void Init();

	// Update necessary data
	void Update();

	// Draw a frame using frame buffer
	void Draw();

	// Process keyboard mouse etc input
	void ProcessInput(GLFWwindow* window);

	// Init GLFW
	void GLPrepare();

	// Finish GLFW
	void GLFinish();

	// Render config
	void BuildFrameBuffers();
	void BuildLights();
	void BuildSmokes();
	void BuildGeometries();
	void BuildShaders();
	void BuildMaterials();
	void BuildTextures();
	void BuildModelMeshes();
	void BuildRenderItems();

	void UpdatePassCb();
	void UpdateObjectCb();
	void UpdateSmoke();
	void UpdateDrawData();

	void DrawOpaque();
	void DrawShadow();
	void DrawLight();
	void DrawSmoke();
	void DrawSkyBox();
	void DrawDebugRT();
	void DrawBoundingBox();

	// Imgui configs
	void SetupImgui();
	void DrawImgui();

	static EInputButton GetPressedButton(GLFWwindow* window);

public:
	bool bUseSmoke = false;
	std::mutex phyxMutex;
	int exit = 0;

	std::unordered_map<std::string, std::unique_ptr<Smoke>> smokes;

private:
	std::unique_ptr<Camera> mCamera;

	TinyddsLoader::DDSFile mDDSLoader;
	MeshLoader mMeshLoader;

	std::unordered_map<std::string, std::unique_ptr<Shader>> mShaders;
	// Built-in geometry data
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	// Model geometry data
	std::unordered_map<std::string, std::unique_ptr<ModelMesh>> mModelMeshes;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::vector<std::unique_ptr<RenderItem>> mRenderItems;
	std::vector<std::unique_ptr<RenderItem>> mDebugRenderItems;
	std::vector<std::unique_ptr<RenderItem>> mBoundingBoxRenderItems;

	std::vector<RenderItem*> mOpaqueItems;
	std::vector<RenderItem*> mLightItems;
	std::vector<RenderItem*> mSmokeItems;

	std::unique_ptr<RenderItem> mSkyItem;

	std::vector<std::unique_ptr<Light>> mLights;

	PassCb mPassCb;

	std::unique_ptr<ShadowMap> mShadowMap;
	std::unique_ptr<GameFrameBufferObject> mSceneFB;
	std::unique_ptr<GameFrameBufferObject> mDebugFB;

	ImGuiContext* mSceneImGuiCtx = nullptr;
	ImGuiContext* mUICtx = nullptr;

	GLFWwindow* mWindow = nullptr;
	const unsigned int screenWidth = 1920;
	const unsigned int screenHeight = 1080;

	// Time
	float lastFrame = 0.f;
	float deltaTime = 0.f;

	// Culling and flags
	bool bShowBoundingBox = false;
	int numObjVisible = 0;
};