#include "pch.h"
#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) noexcept
{
	m_VertexCount = static_cast<uint32_t>(vertices.size());
	m_IndexCount = static_cast<uint32_t>(indices.size());

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0u;
	heapProperties.VisibleNodeMask = 0u;

	D3D12_RESOURCE_DESC resourceDescriptor = {};
	resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	resourceDescriptor.Width = sizeof(Vertex) * vertices.size();
	resourceDescriptor.Height = 1u;
	resourceDescriptor.DepthOrArraySize = 1u;
	resourceDescriptor.MipLevels = 1u;
	resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescriptor.SampleDesc = { 1u, 0u };
	resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

	HR(DXCore::GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_pVertexBuffer)
	));

	auto pUploadBuffer = DXCore::GetUploadBuffer();

	D3D12_RANGE nullRange = { 0,0 };
	unsigned char* mappedPtr = nullptr;

	HR(pUploadBuffer->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
	std::memcpy(mappedPtr, reinterpret_cast<unsigned char*>(vertices.data()), resourceDescriptor.Width);
	STDCALL(DXCore::GetCommandList()->CopyBufferRegion(m_pVertexBuffer.Get(), 0u, pUploadBuffer.Get(), 0u, resourceDescriptor.Width));
	STDCALL(pUploadBuffer->Unmap(0u, nullptr));
	mappedPtr = nullptr;

	resourceDescriptor.Width = sizeof(uint32_t) * indices.size();

	HR(DXCore::GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_pIndexBuffer)
	));

	HR(pUploadBuffer->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
	std::memcpy(static_cast<unsigned char*>(mappedPtr) + (sizeof(Vertex) * vertices.size()), reinterpret_cast<unsigned char*>(indices.data()), resourceDescriptor.Width);
	STDCALL(DXCore::GetCommandList()->CopyBufferRegion(m_pIndexBuffer.Get(), 0u, pUploadBuffer.Get(), sizeof(Vertex) * vertices.size(), resourceDescriptor.Width));
	STDCALL(pUploadBuffer->Unmap(0u, nullptr));
	mappedPtr = nullptr;

	m_pVertexBuffer->SetName(L"Vertex Buffer");
	m_pIndexBuffer->SetName(L"Index Buffer");
	RenderCommand::TransitionResource(m_pVertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	RenderCommand::TransitionResource(m_pIndexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	//Upload the vertex & index data.
	auto pCommandAllocator = DXCore::GetCommandAllocators()[0];
	auto pCommandList = DXCore::GetCommandList();
	auto pDevice = DXCore::GetDevice();

	HR(pCommandList->Close());
	ID3D12CommandList* commandLists[] = { pCommandList.Get() };
	STDCALL(DXCore::GetCommandQueue()->ExecuteCommandLists(ARRAYSIZE(commandLists), commandLists));
	RenderCommand::Flush();

	HR(pCommandAllocator->Reset());
	HR(pCommandList->Reset(pCommandAllocator.Get(), nullptr));
}
