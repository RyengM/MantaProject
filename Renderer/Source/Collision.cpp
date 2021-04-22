#include "Collision.h"

void BoundingFrustum::ExtractFrustumFromMatrix(glm::mat4 matrix)
{
	// Note that matrix is col major in OpenGL
	paramLeft.x = matrix[0][3] + matrix[0][0];
	paramLeft.y = matrix[1][3] + matrix[1][0];
	paramLeft.z = matrix[2][3] + matrix[2][0];
	paramLeft.w = matrix[3][3] + matrix[3][0];

	paramRight.x = matrix[0][3] - matrix[0][0];
	paramRight.y = matrix[1][3] - matrix[1][0];
	paramRight.z = matrix[2][3] - matrix[2][0];
	paramRight.w = matrix[3][3] - matrix[3][0];

	paramTop.x = matrix[0][3] - matrix[0][1];
	paramTop.y = matrix[1][3] - matrix[1][1];
	paramTop.z = matrix[2][3] - matrix[2][1];
	paramTop.w = matrix[3][3] - matrix[3][1];

	paramBottom.x = matrix[0][3] + matrix[0][1];
	paramBottom.y = matrix[1][3] + matrix[1][1];
	paramBottom.z = matrix[2][3] + matrix[2][1];
	paramBottom.w = matrix[3][3] + matrix[3][1];

	paramNear.x = matrix[0][3] + matrix[0][2];
	paramNear.y = matrix[1][3] + matrix[1][2];
	paramNear.z = matrix[2][3] + matrix[2][2];
	paramNear.w = matrix[3][3] + matrix[3][2];

	paramFar.x = matrix[0][3] - matrix[0][2];
	paramFar.y = matrix[1][3] - matrix[1][2];
	paramFar.z = matrix[2][3] - matrix[2][2];
	paramFar.w = matrix[3][3] - matrix[3][2];

	// Normalize plane, so we can extract normal as glm::vec3(plane.a, plane.b, plane.c) and distance as plane.a * pt.x + plane.b * pt.y + plane.c * pt.z + plane.d
	paramLeft /= sqrt(paramLeft.x * paramLeft.x + paramLeft.y * paramLeft.y + paramLeft.z * paramLeft.z);
	paramRight /= sqrt(paramRight.x * paramRight.x + paramRight.y * paramRight.y + paramRight.z * paramRight.z);
	paramTop /= sqrt(paramTop.x * paramTop.x + paramTop.y * paramTop.y + paramTop.z * paramTop.z);
	paramBottom /= sqrt(paramBottom.x * paramBottom.x + paramBottom.y * paramBottom.y + paramBottom.z * paramBottom.z);
	paramNear /= sqrt(paramNear.x * paramNear.x + paramNear.y * paramNear.y + paramNear.z * paramNear.z);
	paramFar /= sqrt(paramFar.x * paramFar.x + paramFar.y * paramFar.y + paramFar.z * paramFar.z);
}

IntersectType BoundingFrustum::FastIntersectAxisAlignedBoxPlane(glm::vec3 frustumOrigin, glm::vec4 plane, float radius, bool bFarNear)
{
	float distance = plane.w;

	if (abs(distance) < radius) return IntersectType::INTERSECT;
	if (distance > 0) return IntersectType::CONTAIN;
	return IntersectType::DISJOINT;
}

IntersectType BoundingFrustum::ContainAxisAlignedBox(AxisAlignedBoundingBox localAABB, glm::vec3 frustumOrigin)
{
	float radius = glm::length(localAABB.entents);

	IntersectType left = FastIntersectAxisAlignedBoxPlane(frustumOrigin, paramLeft, radius, false);
	IntersectType right = FastIntersectAxisAlignedBoxPlane(frustumOrigin, paramRight, radius, false);
	IntersectType top = FastIntersectAxisAlignedBoxPlane(frustumOrigin, paramTop, radius, false);
	IntersectType bottom = FastIntersectAxisAlignedBoxPlane(frustumOrigin, paramBottom, radius, false);
	IntersectType near = FastIntersectAxisAlignedBoxPlane(frustumOrigin, paramNear, radius, true);
	IntersectType far = FastIntersectAxisAlignedBoxPlane(frustumOrigin, paramFar, radius, true);

	if (left == IntersectType::DISJOINT || right == IntersectType::DISJOINT || top == IntersectType::DISJOINT || bottom == IntersectType::DISJOINT || near == IntersectType::DISJOINT || far == IntersectType::DISJOINT)
		return IntersectType::DISJOINT;
	if (left == IntersectType::CONTAIN && right == IntersectType::CONTAIN && top == IntersectType::CONTAIN && bottom == IntersectType::CONTAIN && near == IntersectType::CONTAIN && far == IntersectType::CONTAIN)
		return IntersectType::CONTAIN;
	return IntersectType::INTERSECT;
}

