#pragma once
class Triangle
{
public:
	Triangle() noexcept;
	~Triangle() noexcept = default;
	[[nodiscard]] constexpr uint32_t GetNrOfIndices() noexcept { return m_NrOfIndices; }
	[[nodiscard]] constexpr uint32_t GetNrOfVertices() noexcept { return m_NrOfVertices; }
	[[nodiscard]] constexpr Microsoft::WRL::ComPtr<ID3D12Resource>& GetVertexBuffer() noexcept { return m_pVertexBuffer; }
	[[nodiscard]] constexpr Microsoft::WRL::ComPtr<ID3D12Resource>& GetIndexBuffer() noexcept { return m_pIndexBuffer; }
	void SetConstantBufferView(const ConstantBufferView& transformBuffer) noexcept { m_TransformConstantBufferView =  transformBuffer; }
	[[nodiscard]] constexpr ConstantBufferView& GetTransformConstantBufferView() noexcept { return m_TransformConstantBufferView; }
	[[nodiscard]] constexpr DirectX::XMFLOAT4X4& GetWorldMatrix() noexcept { return m_WorldMatrix; }
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBuffer;
	ConstantBufferView m_TransformConstantBufferView;
	uint32_t m_NrOfIndices;
	uint32_t m_NrOfVertices;
	DirectX::XMFLOAT4X4 m_WorldMatrix;
};