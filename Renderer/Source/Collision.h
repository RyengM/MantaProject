#pragma once

#include "Utils.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class IntersectType
{
	DISJOINT = 0,
	INTERSECT = 1,
	CONTAIN = 2
};

struct BoundingFrustum
{
	// Frustum plane expression coefficients: a, b, c, d
	// Normal direction: frustum inward, ax + by + cz + d > 0
	glm::vec4 paramLeft;
	glm::vec4 paramRight;
	glm::vec4 paramTop;
	glm::vec4 paramBottom;
	glm::vec4 paramNear;
	glm::vec4 paramFar;

	// From Gil Gribb, Fast Extraction of Viewing Frustum Planes from the WorldView-Projection Matrix, 2001
	// Which extract local frustum from MVP matrix
	void ExtractFrustumFromMatrix(glm::mat4 matrix);
	
	IntersectType ContainAxisAlignedBox(AxisAlignedBoundingBox localAABB, glm::vec3 frustumOrigin);

	// Frustum intersect test with sphere surround the axis aligned box, which is bad for narrow objects
	IntersectType FastIntersectAxisAlignedBoxPlane(glm::vec3 frustumOrigin, glm::vec4 plane, float radius, bool bFarNear);
};