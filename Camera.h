#pragma once
static const DirectX::XMFLOAT3 WorldUpVector = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
static const DirectX::XMFLOAT3 CameraStartPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
class Camera
{
public:
	Camera(const DirectX::XMFLOAT3& position, const uint32_t width, const uint32_t height) noexcept;
	~Camera() noexcept = default;
	void RecalculateViewProjectionMatrix() noexcept;
	void Update(const float deltaTime) noexcept;
	[[nodiscard]] constexpr DirectX::XMFLOAT4X4& GetVPMatrix() noexcept { return m_ViewProjectionMatrix; }
	[[nodiscard]] float GetElement1PMatrix() noexcept { return m_ProjectionMatrix._11; }
	[[nodiscard]] float GetElement2PMatrix() noexcept { return m_ProjectionMatrix._22; }
	[[nodiscard]] constexpr DirectX::XMFLOAT3& GetPosition() noexcept { return m_Position; }
	void OnMouseMove() noexcept;
	void SetCameraSpeed(float speed) noexcept;

	bool GetRayTraceBool() const noexcept { return m_RayTrace; }
private:
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_UpVector;
	DirectX::XMFLOAT3 m_FrontVector;
	DirectX::XMFLOAT3 m_RightVector;

	DirectX::XMFLOAT4X4 m_ViewMatrix;
	DirectX::XMFLOAT4X4 m_ProjectionMatrix;
	DirectX::XMFLOAT4X4 m_ViewProjectionMatrix;

	float m_Yaw;
	float m_Pitch;
	float m_CameraSpeed;
	float m_TiltSensitivity;

	bool m_RayTrace = true;
	bool m_RayTraceReleased = true;
};