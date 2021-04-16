#include "Camera.h"

void Camera::ProcessKeyboard(ECameraMovement direction, float deltaTime)
{
	if (active)
	{
		float velocity = movementSpeed * deltaTime;
		if (direction == ECameraMovement::FORWARD)
			position += front * velocity;
		if (direction == ECameraMovement::BACKWARD)
			position -= front * velocity;
		if (direction == ECameraMovement::LEFT)
			position -= right * velocity;
		if (direction == ECameraMovement::RIGHT)
			position += right * velocity;

		UpdateView();
	}
}

void Camera::ProcessMouseMovement(float x, float y, EInputButton input)
{
	glm::vec2 pos{x, y};

	if (input == EInputButton::RIGHT)
	{
		active = true;
		glm::vec2 delta = (pos - cursorLastPos) * mouseSensitivity;

		float sign = up.y < 0 ? -1.0f : 1.0f;
		yaw += sign * delta.x;
		pitch -= delta.y;
	}
	else if (input == EInputButton::NONE)
		active = false;

	cursorLastPos = pos;

	UpdateCameraVectors();
	UpdateView();
}

void Camera::UpdateCameraVectors()
{
	// calculate the new front vector
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);
	// also re-calculate the right and up vector
	right = glm::normalize(glm::cross(front, worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, front));
}