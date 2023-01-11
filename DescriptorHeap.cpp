#include "pch.h"
#include "DescriptorHeap.h"
#include "DXCore.h"

DescriptorHeap::DescriptorHeap(uint32_t capacity, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool shaderVisible) noexcept
	 :m_Capacity{capacity}, 
	  m_Size{0u}, 
	  m_HeapType{heapType}, 
	  m_IsShaderVisible{ shaderVisible }
{
	DBG_ASSERT(capacity > 0, "Invalid descriptor size.");

	D3D12_DESCRIPTOR_HEAP_DESC descriptor = {};
	descriptor.Type = heapType;
	descriptor.NumDescriptors = capacity;
	descriptor.Flags = shaderVisible == true ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptor.NodeMask = 0u;

	HR(DXCore::GetDevice()->CreateDescriptorHeap(&descriptor, IID_PPV_ARGS(&m_pDescriptorHeap)));

	m_IncrementSize = DXCore::GetDevice()->GetDescriptorHandleIncrementSize(heapType);
	m_CPUAdressStart = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_CPUCurrentAdressOffset = m_CPUAdressStart;
	if (shaderVisible == true)
		m_GPUAdressStart = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	else
		m_GPUAdressStart = {};
}

void DescriptorHeap::OffsetCPUAddressPointerBy(uint32_t offset) noexcept
{
	DBG_ASSERT(!(m_Size + 1 > m_Capacity), "Descriptor Heap is full.");
	m_CPUCurrentAdressOffset.ptr += (offset * m_IncrementSize);
	m_Size++;
}
