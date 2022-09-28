#include "Camera.h"

using namespace DirectX;

Camera::Camera(float x, float y, float z, float aspectRatio, float fieldOfView, float movementSpeed = 1, float mouseLookSpeed = 1)
    : transform()
{
    transform.SetPosition(x, y, z);

    this->fieldOfView = fieldOfView;
    this->aspectRatio = aspectRatio;

    this->movementSpeed = movementSpeed;
    this->mouseLookSpeed = mouseLookSpeed;

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
        transform.MoveRelative(0, 0, movementSpeed * deltaTime);
    }

    if (input.KeyDown('S'))
    {
        transform.MoveRelative(0, 0, -1 * movementSpeed * deltaTime);
    }
    
    if (input.KeyDown('A'))
    {
        transform.MoveRelative(-1 * movementSpeed * deltaTime, 0, 0);
    }
    
    if (input.KeyDown('D'))
    {
        transform.MoveRelative(movementSpeed * deltaTime, 0, 0);
    }

    if (input.KeyDown(VK_SPACE)) {
        transform.MoveAbsolute(0, movementSpeed * deltaTime, 0);
    }

    if (input.KeyDown('X')) {
        transform.MoveAbsolute(0, movementSpeed * deltaTime, 0);
    }

    // Mouse input!
    if (input.MouseLeftDown())
    {
        int cursorMoveX = input.GetMouseXDelta();
        int cursorMoveY = input.GetMouseYDelta();

        // X-axis rotation yaws around the Y-axis
        transform.Rotate(0, cursorMoveX * mouseLookSpeed * deltaTime, 0);

        // Y-axis rotation pitches around the X-axis
        transform.Rotate(cursorMoveY * mouseLookSpeed * deltaTime, 0, 0);

        // Get X rotation for clamp
        DirectX::XMFLOAT3 newRotation = transform.GetPitchYawRoll();
        if (newRotation.x >= DirectX::XM_PIDIV2)
        {
            transform.SetPitchYawRoll(DirectX::XM_PIDIV2, newRotation.y, newRotation.z);
        }
        if (newRotation.x <= DirectX::XM_PIDIV2 * -1)
        {
            transform.SetPitchYawRoll(DirectX::XM_PIDIV2 * -1, newRotation.y, newRotation.z);
        }
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

Transform Camera::GetTransform()
{
    return transform;
}

float* Camera::GetFOV()
{
    return &fieldOfView;
}

float* Camera::GetAspectRatio()
{
    return &aspectRatio;
}
