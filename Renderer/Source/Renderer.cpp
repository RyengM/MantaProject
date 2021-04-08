#include "Renderer.h"
#include <iostream>

Renderer::Renderer()
{

}

void Renderer::Init()
{
	GLPrepare();
	mCurrentCamera = Camera(glm::vec3(-10.f, 5.f, 0.f), glm::vec3(0.0f, 1.0f, 0.0f));

	//mShadowMap = std::make_unique<ShadowMap>(1024, 1024);
	mShadowMap = std::make_unique<ShadowMap>(2048, 2048);
	mShadowMap->BuildFrameBuffer();

	BuildTextures();
	BuildLights();
	BuildGeometries();
	BuildMaterials();
	BuildShaders();
	BuildRenderItems();
}

void Renderer::BuildLights()
{
	auto dirLight = std::make_unique<Light>();
	dirLight->type = 0;
	dirLight->strength = glm::vec3(1.0f, 1.0f, 1.0f);
	dirLight->position = glm::vec3(0.f, 15.f, 10.f);
	dirLight->focalPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	mLight.push_back(std::move(dirLight));
	//auto pointLight = std::make_unique<Light>();
	//pointLight->type = 1;
	//pointLight->strength = glm::vec3(0.8f, 0.4f, 0.5f);
	//pointLight->position = glm::vec3(1.f, 1.f, 0.f);
	//pointLight->falloffStart = 1.f;
	//pointLight->falloffEnd = 4.f;
	//mLight.push_back(std::move(pointLight));
}

void Renderer::BuildTextures()
{
	std::vector<std::string> texNames =
	{
		"bricksDiffuseMap",
		"bricksNormalMap",
		"tileDiffuseMap",
		"tileNormalMap"
	};

	std::vector<std::string> texFilenames =
	{
		"Texture/bricks2.dds",
		"Texture/bricks2_nmap.dds",
		"Texture/tile.dds",
		"Texture/tile_nmap.dds"
	};

	for (int i = 0; i < (int)texNames.size(); ++i)
	{
		auto texMap = std::make_unique<Texture>();
		texMap->name = texNames[i];
		texMap->path = texFilenames[i];
		auto ret = DDSLoader.Load(texMap->path.c_str());
		if (ret != TinyddsLoader::Result::Success)
		{
			std::cout << "Failed to load.[" << texMap->path << "]\n";
			std::cout << "Result : " << int(ret) << "\n";
		}
		texMap->BuildResource(DDSLoader);

		mTextures[texMap->name] = std::move(texMap);
	}
}

void Renderer::BuildGeometries()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);

	MeshGeometry boxMesh;
	boxMesh.indexCount = (unsigned int)box.Indices32.size();

	MeshGeometry gridMesh;
	gridMesh.indexCount = (unsigned int)grid.Indices32.size();

	MeshGeometry sphereMesh;
	sphereMesh.indexCount = (unsigned int)sphere.Indices32.size();
	
	std::vector<Vertex> boxVertices(box.Vertices.size());
	std::vector<Vertex> gridVertices(grid.Vertices.size());
	std::vector<Vertex> sphereVertices(sphere.Vertices.size());
	
	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		boxVertices[i].pos = box.Vertices[i].Position;
		boxVertices[i].normal = box.Vertices[i].Normal;
		boxVertices[i].tangentU = box.Vertices[i].TangentU;
		boxVertices[i].texC = box.Vertices[i].TexC;
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		gridVertices[i].pos = grid.Vertices[i].Position;
		gridVertices[i].normal = grid.Vertices[i].Normal;
		gridVertices[i].tangentU = grid.Vertices[i].TangentU;
		gridVertices[i].texC = grid.Vertices[i].TexC;
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		sphereVertices[i].pos = sphere.Vertices[i].Position;
		sphereVertices[i].normal = sphere.Vertices[i].Normal;
		sphereVertices[i].tangentU = sphere.Vertices[i].TangentU;
		sphereVertices[i].texC = sphere.Vertices[i].TexC;
	}

	boxMesh.BuildResources(boxVertices, box.Indices32);
	gridMesh.BuildResources(gridVertices, grid.Indices32);
	sphereMesh.BuildResources(sphereVertices, sphere.Indices32);

	mGeometries["box"] = std::move(std::make_unique<MeshGeometry>(boxMesh));
	mGeometries["grid"] = std::move(std::make_unique<MeshGeometry>(gridMesh));
	mGeometries["sphere"] = std::move(std::make_unique<MeshGeometry>(sphereMesh));
}

void Renderer::BuildShaders()
{
	// Shadows
	auto shadowShader = std::make_unique<Shader>();
	shadowShader->CreateVS("Shader/Shadow.vert");
	shadowShader->CreatePS("Shader/Shadow.frag");
	shadowShader->Attach();
	shadowShader->Link();
	shadowShader->Use();
	// Main light: directional
	glm::mat4 lightView = glm::lookAt(mLight[0]->position, mLight[0]->focalPoint, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 lightProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 0.1f, 100.f);
	shadowShader->SetMat4("lightProjView", lightProj * lightView);
	mShaders["Shadow"] = std::move(shadowShader);

	// Opaque objects
	auto shapeShader = std::make_unique<Shader>();
	shapeShader->CreateVS("Shader/Shape.vert");
	shapeShader->CreatePS("Shader/Shape.frag");
	shapeShader->Attach();
	shapeShader->Link();
	shapeShader->Use();
	for (int i = 0; i < mLight.size(); ++i)
	{
		shapeShader->SetInt("light[" + std::to_string(i) + "].type", mLight[i]->type);
		shapeShader->SetVec3("light[" + std::to_string(i) + "].pos", mLight[i]->position);
		shapeShader->SetVec3("light[" + std::to_string(i) + "].strength", mLight[i]->strength);
		shapeShader->SetVec3("light[" + std::to_string(i) + "].dir", mLight[i]->focalPoint - mLight[i]->position);
		shapeShader->SetFloat("light[" + std::to_string(i) + "].fallStart", mLight[i]->falloffStart);
		shapeShader->SetFloat("light[" + std::to_string(i) + "].fallEnd", mLight[i]->falloffEnd);
		shapeShader->SetFloat("light[" + std::to_string(i) + "].spotPower", mLight[i]->spotPower);
	}
	shapeShader->SetInt("diffuseMap", 0);
	shapeShader->SetInt("normalMap", 1);
	shapeShader->SetInt("shadowMap", 2);
	shapeShader->SetMat4("lightProjView", lightProj * lightView);
	mShaders["Shape"] = std::move(shapeShader);

	auto lightShader = std::make_unique<Shader>();
	lightShader->CreateVS("Shader/Light.vert");
	lightShader->CreatePS("Shader/Light.frag");
	lightShader->Attach();
	lightShader->Link();
	lightShader->Use();
	mShaders["Light"] = std::move(lightShader);

	// Smoke
	auto smokeShader = std::make_unique<Shader>();
}

void Renderer::BuildMaterials()
{
	auto brick0 = std::make_unique<Material>();
	brick0->name = "brick0";
	brick0->diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	brick0->fresnelR0 = glm::vec3(0.1f, 0.1f, 0.1f);
	brick0->roughness = 0.1f;
	brick0->diffuseID = mTextures["bricksDiffuseMap"]->textureID;
	brick0->normalID = mTextures["bricksNormalMap"]->textureID;

	auto tile = std::make_unique<Material>();
	tile->name = "tile";
	tile->diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	tile->fresnelR0 = glm::vec3(0.1f, 0.1f, 0.1f);
	tile->roughness = 0.1f;
	tile->diffuseID = mTextures["tileDiffuseMap"]->textureID;
	tile->normalID = mTextures["tileNormalMap"]->textureID;

	auto light0 = std::make_unique<Material>();
	light0->name = "light0";
	light0->emmisive = mLight[0]->strength;

	mMaterials["brick0"] = std::move(brick0);
	mMaterials["tile"] = std::move(tile);
	mMaterials["light0"] = std::move(light0);
}

void Renderer::BuildRenderItems()
{
	auto boxItem = std::make_unique<RenderItem>();
	boxItem->world = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 2.f, 0.f));
	boxItem->mat = mMaterials["brick0"].get();
	boxItem->geo = mGeometries["box"].get();
	boxItem->textureScale = 1;
	mOpaqueItems.push_back(boxItem.get());
	mRenderItems.push_back(std::move(boxItem));

	auto sphereItem = std::make_unique<RenderItem>();
	sphereItem->world = glm::translate(glm::mat4(1.f), glm::vec3(3.f, 2.f, 0.f));
	sphereItem->mat = mMaterials["brick0"].get();
	sphereItem->geo = mGeometries["sphere"].get();
	sphereItem->textureScale = 1;
	mOpaqueItems.push_back(sphereItem.get());
	mRenderItems.push_back(std::move(sphereItem));

	auto gridItem = std::make_unique<RenderItem>();
	gridItem->world = glm::mat4(1.f);
	gridItem->mat = mMaterials["tile"].get();
	gridItem->geo = mGeometries["grid"].get();
	gridItem->textureScale = 5;
	mOpaqueItems.push_back(gridItem.get());
	mRenderItems.push_back(std::move(gridItem));

	auto smokeItem = std::make_unique<SmokeItem>();
	smokeItem->world = glm::translate(glm::mat4(1.f), glm::vec3(2.f, 0.f, 4.f));
	smokeItem->mat = mMaterials["brick0"].get();
	smokeItem->geo = mGeometries["box"].get();
	smokeItem->textureScale = 1;
	mSmokeItem = smokeItem.get();
	mRenderItems.push_back(std::move(smokeItem));

	auto lightItem = std::make_unique<RenderItem>();
	lightItem->world = glm::translate(glm::mat4(1.f), mLight[0]->position);
	lightItem->world = glm::scale(lightItem->world, glm::vec3(0.3f));
	lightItem->mat = mMaterials["light0"].get();
	lightItem->geo = mGeometries["box"].get();
	mLightItems.push_back(lightItem.get());
	mRenderItems.push_back(std::move(lightItem));
}

void Renderer::UpdatePassCb()
{
	mPassCb.view = mCurrentCamera.GetViewMatrix();
	mPassCb.proj = glm::perspective(glm::radians(mCurrentCamera.fov), (float)screenWidth / (float)screenHeight, mCurrentCamera.nearPlane, mCurrentCamera.farPlane);
	mPassCb.eyePos = mCurrentCamera.position;
	mPassCb.lightNum = mLight.size();
}

void Renderer::Draw()
{
	while (!glfwWindowShouldClose(window))
	{
		// Process input
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		ProcessInput(window);

		// Update info
		UpdatePassCb();

		// Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, screenWidth, screenHeight);

		glEnable(GL_DEPTH_TEST);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		DrawLight();
		DrawShadow();
		DrawOpaque();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	GLFinish();
}

void Renderer::DrawLight()
{
	auto light = mShaders["Light"].get();
	light->Use();
	light->SetMat4("passCb.view", mPassCb.view);
	light->SetMat4("passCb.proj", mPassCb.proj);
	for (auto item : mLightItems)
	{
		light->SetMat4("model", item->world);
		light->SetVec3("emmisive", item->mat->emmisive);
		item->Draw(light);
	}
}

void Renderer::DrawShadow()
{
	glViewport(0, 0, mShadowMap->GetWidth(), mShadowMap->GetHeight());
	auto shadow = mShaders["Shadow"].get();
	shadow->Use();
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMap->GetFrameBufferID());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (auto item : mOpaqueItems)
	{
		shadow->SetMat4("model", item->world);
		item->Draw(shadow);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
}

void Renderer::DrawOpaque()
{
	auto shape = mShaders["Shape"].get();
	shape->Use();
	shape->SetMat4("passCb.view", mPassCb.view);
	shape->SetMat4("passCb.proj", mPassCb.proj);
	shape->SetVec3("passCb.eyePos", mPassCb.eyePos);
	shape->SetVec4("passCb.ambientLight", mPassCb.ambientLight);
	shape->SetInt("passCb.lightNum", mPassCb.lightNum);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, mShadowMap.get()->GetFrameBufferID());
	for (auto item : mOpaqueItems)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, item->mat->diffuseID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, item->mat->normalID);
		shape->SetMat4("model", item->world);
		shape->SetInt("textureScale", item->textureScale);
		shape->SetVec4("material.diffuse", item->mat->diffuse);
		shape->SetVec3("material.fresnelR0", item->mat->fresnelR0);
		shape->SetFloat("material.roughness", item->mat->roughness);
		item->Draw(shape);
	}
}

void Renderer::GLPrepare()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	window = glfwCreateWindow(screenWidth, screenHeight, "MantaProject", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	typedef void* (*FUNC)(GLFWwindow*, int, int);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	// init static camera for callback
	lastX = screenWidth / 2.f;
	lastY = screenHeight / 2.f;
}

void Renderer::GLFinish()
{
	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
}

void Renderer::ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		exit = 1;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		mCurrentCamera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		mCurrentCamera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		mCurrentCamera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		mCurrentCamera.ProcessKeyboard(RIGHT, deltaTime);
}

void Renderer::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void Renderer::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	// reversed since y-coordinates go from bottom to top
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	mCurrentCamera.ProcessMouseMovement(xoffset, yoffset);
}

void Renderer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mCurrentCamera.ProcessMouseScroll(yoffset);
}