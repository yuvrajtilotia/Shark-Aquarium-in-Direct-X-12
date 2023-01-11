#include "pch.h"
#include "DescriptorHeapNonShaderVisible.h"
#include "DXCore.h"

DescriptorHeapNonShaderVisible::DescriptorHeapNonShaderVisible(uint32_t capacity, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) noexcept
	: m_Capacity{capacity},
	  m_NrOfDescriptors{ 0u },
	  m_Type{ descriptorHeapType }
{
	DBG_ASSERT(m_Capacity <= 1'000'000, "Capacity is too high for D3D12 Hardware Tier 1.");

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = m_Type;
	descriptorHeapDesc.NumDescriptors = m_Capacity;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NodeMask = 0u;

	HR(DXCore::GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap)));
	m_IncrementSize = DXCore::GetDevice()->GetDescriptorHandleIncrementSize(descriptorHeapType);

	m_HeadAdress = m_TailAdress = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

Microsoft::WRL::ComPtr<ID3D12Resource> DescriptorHeapNonShaderVisible::CreateConstantBuffer(uint32_t byteWidth) noexcept
{
	DBG_ASSERT(m_NrOfDescriptors != m_Capacity, "Descriptor Heap is full.");

	D3D12_HEAP_PROPERTIES bufferHeapProperties = {};
	bufferHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	bufferHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	bufferHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	bufferHeapProperties.CreationNodeMask = 0u;
	bufferHeapProperties.VisibleNodeMask = 0u;

	byteWidth = (byteWidth + 255) & ~255;

	D3D12_RESOURCE_DESC bufferDescriptor = {};
	bufferDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDescriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	bufferDescriptor.Width = byteWidth;
	bufferDescriptor.Height = 1u;
	bufferDescriptor.DepthOrArraySize = 1u;
	bufferDescriptor.MipLevels = 1u;
	bufferDescriptor.Format = DXGI_FORMAT_UNKNOWN;
	bufferDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
	bufferDescriptor.SampleDesc.Count = 1u;
	bufferDescriptor.SampleDesc.Quality = 0u;
	bufferDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> pResource{ nullptr };
	HR(DXCore::GetDevice()->CreateCommittedResource(&bufferHeapProperties, D3D12_HEAP_FLAG_NONE,
		&bufferDescriptor, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pResource)));

	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferDescriptor = {};
	constantBufferDescriptor.BufferLocation = pResource->GetGPUVirtualAddress();
	constantBufferDescriptor.SizeInBytes = byteWidth;

	STDCALL(DXCore::GetDevice()->CreateConstantBufferView(&constantBufferDescriptor, m_TailAdress));

	m_MostRecentHandle = m_TailAdress;
	m_TailAdress.ptr += m_IncrementSize;
	m_NrOfDescriptors++;

	return pResource;
}
