#pragma once
// "Please only include everything only once!"

#include <DirectXMath.h>

class Transform
{
public:
	Transform();

	// Offseters - these change the existing data
	void MoveAbsolute(float x, float y, float z);
	void MoveRelative(float x, float y, float z);
	void Rotate(float p, float y, float r);
	void Scale(float x, float y, float z);

	// Setters - these overwrite the exisiting data
	void SetPosition(float x, float y, float z);
	void SetPitchYawRoll(float p, float y, float r);
	void SetScale(float x, float y, float z);

	// Getters - these return the existing data
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetForward();

	// Get the world matrix
	// Describes where our model is in 3d space
	DirectX::XMFLOAT4X4 GetWorldMatrix();

	// Get the inverse transpose of the world matrix
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

private:
	// Raw Transformation Data
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll; // NOT a quaternion, just three different rotation values
	DirectX::XMFLOAT3 scale;

	// Relative Transformation Data
	DirectX::XMFLOAT3 up;
	DirectX::XMFLOAT3 right;
	DirectX::XMFLOAT3 forward;

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTranspose;

	bool matrixDirty;
	bool vectorsDirty;

	void UpdateVectors();
};

