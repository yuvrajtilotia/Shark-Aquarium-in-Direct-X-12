#include "pch.h"
#include "Camera.h"
#include "Keyboard.h"
#include "Mouse.h"

Camera::Camera(const DirectX::XMFLOAT3& position, const uint32_t width, const uint32_t height) noexcept
	: m_Position{position},
	  m_UpVector{ WorldUpVector },
	  m_RightVector{ DirectX::XMFLOAT3{1.0f, 0.0f, 0.0f} },
	  m_Yaw{ 90.0f },
	  m_Pitch{ 0.0f },
	  m_CameraSpeed{ 40.0f },
	  m_TiltSensitivity{ 0.25f }
{
	DirectX::XMVECTOR positionAsXMVector = DirectX::XMLoadFloat3(&position);

	DirectX::XMStoreFloat3(&m_FrontVector, DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&CameraStartPosition), positionAsXMVector)));

	DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(positionAsXMVector, DirectX::XMLoadFloat3(&m_FrontVector));

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(positionAsXMVector, lookAt, DirectX::XMLoadFloat3(&m_UpVector));

	float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

	DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), aspectRatio, 0.1f, 10000.0f);

	DirectX::XMStoreFloat4x4(&m_ViewProjectionMatrix, viewMatrix * projectionMatrix);
	DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);
	DirectX::XMStoreFloat4x4(&m_ProjectionMatrix, projectionMatrix);
}

void Camera::RecalculateViewProjectionMatrix() noexcept
{
	DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&m_Position), DirectX::XMLoadFloat3(&m_FrontVector));
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&m_Position),
		lookAt,
		DirectX::XMLoadFloat3(&m_UpVector));

	DirectX::XMStoreFloat4x4(&m_ViewMatrix, viewMatrix);
	DirectX::XMStoreFloat4x4(&m_ViewProjectionMatrix, viewMatrix * DirectX::XMLoadFloat4x4(&m_ProjectionMatrix));
}

void Camera::Update(const float deltaTime) noexcept
{
	if (Mouse::IsRightButtonPressed())
	{
		DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_Position);
		if (Keyboard::IsKeyDown(KEY::W))
		{
			DirectX::XMVECTOR frontVector = DirectX::XMLoadFloat3(&m_FrontVector);
			frontVector = DirectX::XMVectorScale(frontVector, m_CameraSpeed * deltaTime);
			position = DirectX::XMVectorAdd(position, frontVector);
		}
		else if (Keyboard::IsKeyDown(KEY::S))
		{
			DirectX::XMVECTOR frontVector = DirectX::XMLoadFloat3(&m_FrontVector);
			frontVector = DirectX::XMVectorScale(frontVector, m_CameraSpeed * deltaTime);
			position = DirectX::XMVectorSubtract(position, frontVector);
		}
		if (Keyboard::IsKeyDown(KEY::D))
		{
			DirectX::XMVECTOR rightVector = DirectX::XMLoadFloat3(&m_RightVector);
			rightVector = DirectX::XMVectorScale(rightVector, m_CameraSpeed * deltaTime);
			position = DirectX::XMVectorAdd(position, rightVector);
		}
		else if (Keyboard::IsKeyDown(KEY::A))
		{
			DirectX::XMVECTOR rightVector = DirectX::XMLoadFloat3(&m_RightVector);
			rightVector = DirectX::XMVectorScale(rightVector, m_CameraSpeed * deltaTime);
			position = DirectX::XMVectorSubtract(position, rightVector);
		}
		if (Keyboard::IsKeyDown(KEY::Q))
		{
			DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&m_UpVector);
			upVector = DirectX::XMVectorScale(upVector, m_CameraSpeed * deltaTime);
			position = DirectX::XMVectorSubtract(position, upVector);
		}
		else if (Keyboard::IsKeyDown(KEY::E))
		{
			DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&m_UpVector);
			upVector = DirectX::XMVectorScale(upVector, m_CameraSpeed * deltaTime);
			position = DirectX::XMVectorAdd(position, upVector);
		}
		DirectX::XMStoreFloat3(&m_Position, position);
		RecalculateViewProjectionMatrix();

		OnMouseMove();
	}
	if (Keyboard::IsKeyDown(KEY::R) && m_RayTraceReleased)
	{
		m_RayTraceReleased = false;
		m_RayTrace = !m_RayTrace;
	}
	else if (!Keyboard::IsKeyDown(KEY::R) && !m_RayTraceReleased)
	{
		m_RayTraceReleased = true;
	}
}

void Camera::OnMouseMove() noexcept
{
	auto [xOffset, yOffset] = Mouse::GetMovementDelta();
	m_Yaw -= static_cast<float>(xOffset) * m_TiltSensitivity;
	m_Pitch -= static_cast<float>(yOffset) * m_TiltSensitivity;

	if (m_Pitch > 89.0f)
		m_Pitch = 89.0f;
	else if (m_Pitch < -89.0f)
		m_Pitch = -89.0f;

	float yawRadians = DirectX::XMConvertToRadians(m_Yaw);
	float pitchRadians = DirectX::XMConvertToRadians(m_Pitch);
	float forwardX = std::cos(yawRadians) * std::cos(pitchRadians);
	float forwardY = std::sin(pitchRadians);
	float forwardZ = std::sin(yawRadians) * std::cos(pitchRadians);

	DirectX::XMVECTOR forwardVector = { forwardX, forwardY, forwardZ, 0.0f };
	forwardVector = DirectX::XMVector3Normalize(forwardVector);
	DirectX::XMStoreFloat3(&m_FrontVector, forwardVector);

	DirectX::XMVECTOR rightVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&WorldUpVector), forwardVector));
	DirectX::XMVECTOR upVector = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(forwardVector, rightVector));
	DirectX::XMStoreFloat3(&m_RightVector, rightVector);
	DirectX::XMStoreFloat3(&m_UpVector, upVector);

	RecalculateViewProjectionMatrix();
}

void Camera::SetCameraSpeed(float speed) noexcept
{
	m_CameraSpeed = speed;
}
