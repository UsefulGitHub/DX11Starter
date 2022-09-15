#pragma once
// "Please only include everything only once!"

#include <DirectXMath.h>

class Transform
{
public:
	Transform();

	// Offseters - these change the existing data
	void MoveAbsolute(float x, float y, float z);
	// let's do void MoveRelative later
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

	// Get the world matrix
	// Describes where our model is in 3d space
	DirectX::XMFLOAT4X4 GetWorldMatrix();

private:
	// Raw Transformation Data
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll; // NOT a quaternion, just three different rotation values
	DirectX::XMFLOAT3 scale;

	DirectX::XMFLOAT4X4 worldMatrix;

	bool matrixDirty;
};

