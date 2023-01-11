#pragma once
#include "DXCore.h"
#include "RenderCommand.h"

#include "Vertex.h"

class Mesh
{
public:
	Mesh() = delete;
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) noexcept;
	~Mesh() noexcept = default;

	const D3D12_GPU_VIRTUAL_ADDRESS GetVertexBufferGPUAddress() const noexcept {
		DBG_ASSERT(m_pVertexBuffer, "Error! Trying to get a vertex buffer's GPU address while it has not been set.");
		return m_pVertexBuffer->GetGPUVirtualAddress();
	}
	const D3D12_GPU_VIRTUAL_ADDRESS GetIndexBufferGPUAddress() const noexcept {
		DBG_ASSERT(m_pVertexBuffer, "Error! Trying to get an index buffer's GPU address while it has not been set.");
		return m_pIndexBuffer->GetGPUVirtualAddress();
	}
	const uint32_t GetVertexCount() const noexcept { return m_VertexCount; }
	const uint32_t GetIndexCount() const noexcept { return m_IndexCount; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBuffer = nullptr;

	uint32_t m_VertexCount = 0u;
	uint32_t m_IndexCount = 0u;
};