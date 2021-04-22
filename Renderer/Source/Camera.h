#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class ECameraMovement {
	FORWARD = 0,
	BACKWARD = 1,
	LEFT = 2,
	RIGHT = 3
};

enum class EInputButton
{
	LEFT = 0,
	RIGHT = 1,
	MIDDLE = 2,
	NONE = 3
};

// an abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	Camera(glm::vec3 position, float fov, float aspect, float nearPlane, float farPlane) : position(position), fov(fov), aspect(aspect), nearPlane(nearPlane), farPlane(farPlane)
	{
		SetAspect();
		UpdateView();
		UpdateCameraVectors();
	}

	inline void SetAspect()
	{
		projMatrix = glm::perspective(fov, aspect, nearPlane, farPlane);
	}

	inline void UpdateView()
	{
		viewMatrix = glm::lookAt(position, position + front, up);
		viewProjMatrix = projMatrix * viewMatrix;
	}

	inline glm::mat4 GetViewMatrix()
	{
		return viewMatrix;
	}

	inline glm::mat4 GetProjMatrix()
	{
		return projMatrix;
	}

	inline glm::mat4 GetViewProjMatrix()
	{
		return viewProjMatrix;
	}

	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(ECameraMovement direction, float deltaTime);

	// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float x, float y, EInputButton input);

private:
	// calculates the front vector from the Camera's (updated) Euler Angles
	void UpdateCameraVectors();

public:
	glm::mat4 viewMatrix = glm::mat4(1.f);
	glm::mat4 projMatrix = glm::mat4(1.f);
	glm::mat4 viewProjMatrix = glm::mat4(1.f);
	// camera Attributes
	glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 front = glm::vec3(1.f, 0.f, 0.f);
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 right = glm::vec3(0.f, 0.f, 1.f);
	glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
	// euler Angles
	float yaw = 0.f;
	float pitch = 0.f;
	// camera options
	float movementSpeed = 2.5f;
	float mouseSensitivity = 0.2f;
	float fov = 45.f;
	float aspect = 16.f / 9.f;
	float nearPlane = 0.1;
	float farPlane = 1000.f;
	// cursor
	glm::vec2 cursorLastPos = glm::vec2(0.f, 0.f);
	// right mouse is activated
	bool active = false;
};