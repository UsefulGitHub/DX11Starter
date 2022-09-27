#include "Camera.h"

using namespace DirectX;

Camera::Camera(float x, float y, float z, float aspectRatio, float fieldOfView)
    : transform()
{
    transform.SetPosition(x, y, z);

    this->fieldOfView = fieldOfView;

    // Z positive goes INTO the screen
    // Z positive goes INTO the screen with Left Hand
    UpdateProjectionMatrix(aspectRatio);
    UpdateViewMatrix();
}

Camera::~Camera()
{
}

void Camera::Update(float deltaTime)
{
    Input& input = Input::GetInstance();

    if (input.KeyDown('W'))
    {
        transform.MoveRelative(0, 0, 1 * deltaTime);
    }

    if (input.KeyDown('S'))
    {
        transform.MoveRelative(0, 0, -1 * deltaTime);
    }
    
    if (input.KeyDown('A'))
    {
        transform.MoveRelative(-1 * deltaTime, 0, 0);
    }
    
    if (input.KeyDown('D'))
    {
        transform.MoveRelative(1 * deltaTime, 0, 0);
    }

    if (input.KeyDown(VK_SPACE)) {
        transform.MoveAbsolute(0, 1 * deltaTime, 0);
    }

    if (input.KeyDown('X')) {
        transform.MoveAbsolute(0, -1 * deltaTime, 0);
    }

    // Update the view matrix because ewe have probably changed the transform
    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    XMFLOAT3 pos = transform.GetPosition();
    XMFLOAT3 forward = transform.GetForward();
    // Z positive goes INTO the screen with Left Hand
    XMMATRIX view = XMMatrixLookToLH(
        XMLoadFloat3(&pos),
        XMLoadFloat3(&forward),
        XMVectorSet(0, 1, 0, 0)
    );
    XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    // Z positive goes INTO the screen with Left Hand
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        fieldOfView,
        aspectRatio,
        0.01f,
        100.0f
    );
    XMStoreFloat4x4(&projMatrix, proj);
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
    return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
    return projMatrix;
}
