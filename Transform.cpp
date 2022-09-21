#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
	position(0,0,0),
	pitchYawRoll(0,0,0),
	scale(1,1,1)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
	matrixDirty = false;
}

// Offsetters also called Transformers

void Transform::MoveAbsolute(float x, float y, float z)
{
	XMVECTOR start = XMLoadFloat3(&position);
	XMVECTOR offset = XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&position, XMVectorAdd(start, offset));
	matrixDirty = true;
}

void Transform::Rotate(float p, float y, float r)
{
	XMVECTOR start = XMLoadFloat3(&pitchYawRoll);
	XMVECTOR offset = XMVectorSet(p, y, r, 0);
	XMStoreFloat3(&pitchYawRoll, XMVectorAdd(start, offset));
	matrixDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	XMVECTOR start = XMLoadFloat3(&scale);
	XMVECTOR offset = XMVectorSet(x, y, z, 0);
	XMStoreFloat3(&scale, XMVectorMultiply(start, offset));
	matrixDirty = true;
}

// Setters

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	matrixDirty = true;
}

void Transform::SetPitchYawRoll(float p, float y, float r)
{
	pitchYawRoll.x = p;
	pitchYawRoll.y = y;
	pitchYawRoll.z = r;
	matrixDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	matrixDirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return pitchYawRoll;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if (matrixDirty)
	{
		// This is good...
		/* XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z); */
		// ...but this is a little bit faster- but it won't be required
		XMMATRIX trans = XMMatrixTranslationFromVector(XMLoadFloat3(&position));

		// Reminder - the rollpitchyaw in the function name is the Order they do the rotations in- not the order of expected inputs! (It wants pitch, yaw, roll.)
		XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
	
		XMMATRIX sc = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

		// Combine them
		XMMATRIX worldMat = sc * rot * trans; // S * R * T // is the way to be
	
		// Store as a storage type
		XMStoreFloat4x4(&worldMatrix, worldMat);

		// Store the inverse transpose
		XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(worldMat)));

		// Lower the flag - our work here is done! (for now)
		matrixDirty = false;
	}

	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	return worldInverseTranspose;
}
