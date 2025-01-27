#pragma once

#include "Transform.h"
#include "Input.h"
#include <DirectXMath.h>

class Camera
{
public:
	Camera(float x, float y, float z, float aspectRatio, float fieldOfView, float movementSpeed, float mouseLookSpeed);
	~Camera();

	// Update methods
	void Update(float deltaTime);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	Transform GetTransform();
	float* GetFOV();
	float* GetAspectRatio();

private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	Transform transform;

	float fieldOfView;
	float aspectRatio;

	float movementSpeed;
	float mouseLookSpeed;
};

