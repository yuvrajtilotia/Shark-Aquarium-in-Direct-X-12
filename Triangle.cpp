#include "pch.h"
#include "Triangle.h"
#include "DXCore.h"
#include "Vertex.h"

Triangle::Triangle() noexcept
	: m_NrOfIndices{3u}, m_NrOfVertices{3u}
{
	DirectX::XMStoreFloat4x4(&m_WorldMatrix, DirectX::XMMatrixIdentity());

	Microsoft::WRL::ComPtr<ID3D12Heap> pVBHeap{nullptr};
	Microsoft::WRL::ComPtr<ID3D12Heap> pIBHeap{ nullptr };
	SimpleVertex triangle[3];
	triangle[0].Position = DirectX::XMFLOAT3{ -0.5f, -0.5f, 0.0f };
	triangle[1].Position = DirectX::XMFLOAT3{ 0.0f, 0.5f, 0.0f };
	triangle[2].Position = DirectX::XMFLOAT3{ 0.5f, -0.5f, 0.0f };
	unsigned int indices[] = { 0, 1, 2 };

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0u;
	heapProperties.VisibleNodeMask = 0u;
	
	D3D12_HEAP_DESC heapDescriptor = {};
	heapDescriptor.SizeInBytes = sizeof(SimpleVertex) * 3u;
	heapDescriptor.Properties = heapProperties;
	heapDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	heapDescriptor.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;
	HR(DXCore::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pVBHeap)));
	
	D3D12_RESOURCE_DESC resourceDescriptor = {};
	resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	resourceDescriptor.Width = sizeof(SimpleVertex) * 3u;
	resourceDescriptor.Height = 1u;
	resourceDescriptor.DepthOrArraySize = 1u;
	resourceDescriptor.MipLevels = 1u;
	resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescriptor.SampleDesc = { 1u, 0u };
	resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
	HR(DXCore::GetDevice()->CreatePlacedResource
	(
		pVBHeap.Get(),
		0u, 
		&resourceDescriptor, 
		D3D12_RESOURCE_STATE_COPY_DEST, 
		nullptr, 
		IID_PPV_ARGS(&m_pVertexBuffer)
	));
	
	auto pUploadBuffer = DXCore::GetUploadBuffer();
	
	D3D12_RANGE nullRange = { 0,0 };
	unsigned char* mappedPtr = nullptr;
	
	HR(pUploadBuffer->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
	std::memcpy(mappedPtr, reinterpret_cast<unsigned char*>(triangle), resourceDescriptor.Width);
	STDCALL(DXCore::GetCommandList()->CopyBufferRegion(m_pVertexBuffer.Get(), 0u, pUploadBuffer.Get(), 0u, resourceDescriptor.Width));
	STDCALL(pUploadBuffer->Unmap(0u, nullptr));
	mappedPtr = nullptr;
	
	heapDescriptor.SizeInBytes = sizeof(unsigned int) * ARRAYSIZE(indices);
	HR(DXCore::GetDevice()->CreateHeap(&heapDescriptor, IID_PPV_ARGS(&pIBHeap)));
	
	resourceDescriptor.Width = sizeof(unsigned int) * ARRAYSIZE(indices);
	HR(DXCore::GetDevice()->CreatePlacedResource
	(
		pIBHeap.Get(),
		0u,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_pIndexBuffer)
	));
	
	HR(pUploadBuffer->Map(0u, &nullRange, reinterpret_cast<void**>(&mappedPtr)));
	std::memcpy(static_cast<unsigned char*>(mappedPtr) + sizeof(triangle), reinterpret_cast<unsigned char*>(indices), resourceDescriptor.Width);
	STDCALL(DXCore::GetCommandList()->CopyBufferRegion(m_pIndexBuffer.Get(), 0u, pUploadBuffer.Get(), sizeof(triangle), resourceDescriptor.Width));
	STDCALL(pUploadBuffer->Unmap(0u, nullptr));
	mappedPtr = nullptr;
}
