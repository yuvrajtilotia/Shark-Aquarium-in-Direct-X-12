#pragma once
#include "Model.h"
#include "MemoryManager.h"

enum UpdateType
{
	NONE = 0,
	SPIN,
	RESIZE,
	MOVEBACKANDFORTH
};

class VertexObject
{
public:
	VertexObject() noexcept = default;
	~VertexObject() noexcept = default;

	void Initialize(
		std::shared_ptr<Model> objectModel,
		DirectX::XMVECTOR pos,
		DirectX::XMVECTOR rot,
		float scale,
		UpdateType updateType,
		DirectX::XMFLOAT4 color
	);

	void Update(float deltaTime);

	const DirectX::XMFLOAT4X4& GetTransform() const { return m_Transform; }
	const std::shared_ptr<Model>& GetModel() const { return m_pModel; }
	void SetTransform(const DirectX::XMFLOAT4X4& transform) noexcept { m_Transform = transform; }
	[[nodiscard]] constexpr ConstantBufferView& GetTransformConstantBufferView() { return m_TransformConstantBufferView; }
	[[nodiscard]] constexpr DirectX::XMFLOAT4& GetColor() { return m_Color; }
private:
	void SetConstantBufferView(const ConstantBufferView& transformBuffer) noexcept { m_TransformConstantBufferView = transformBuffer; }
private:
	DirectX::XMFLOAT4 m_Color = {};
	DirectX::XMFLOAT4X4 m_Transform = {};
	ConstantBufferView m_TransformConstantBufferView;
	std::shared_ptr<Model> m_pModel = nullptr;
	UpdateType m_UpdateType = SPIN;
	bool resizeFlag = false;

	float m_Scale = 1.0f;
	float m_ScaleAngle = 0.0f;
	float m_OriginalX = 0.0f;
};