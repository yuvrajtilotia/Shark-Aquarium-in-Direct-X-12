#include "pch.h"
#include "DescriptorHeapShaderVisible.h"
#include "DXCore.h"

DescriptorHeapShaderVisible::DescriptorHeapShaderVisible(uint32_t capacity, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) noexcept
	: m_Capacity{ capacity * NR_OF_FRAMES }, 
	  m_DescriptorsInUse{0u},
	  m_ReservedDescriptors{0u},
	  m_Type{descriptorHeapType}
{
	DBG_ASSERT(m_Capacity <= 1'000'000, "Capacity is too high for D3D12 Hardware Tier 1.");

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = m_Type;
	descriptorHeapDesc.NumDescriptors = m_Capacity;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0u;

	HR(DXCore::GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap)));
	m_IncrementSize = DXCore::GetDevice()->GetDescriptorHandleIncrementSize(descriptorHeapType);
}

//m_ReservedDescriptors take all ranges into account, frames in flight as well
void DescriptorHeapShaderVisible::AddRange(std::string rangeName, uint32_t nrOfDescriptorsInRange) noexcept
{
	DBG_ASSERT((nrOfDescriptorsInRange * NR_OF_FRAMES) <= (m_Capacity - m_ReservedDescriptors), "Unable to create descriptor range: not enough free space.");
	auto cpuHeapHandle = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto gpuHeapHandle = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	cpuHeapHandle.ptr += m_ReservedDescriptors * m_IncrementSize;

	DescriptorRange descriptorRange = {};
	descriptorRange.CPUHeadAdress = descriptorRange.CPUTailAdress = cpuHeapHandle;
	descriptorRange.GPUHeadAdress = descriptorRange.GPUTailAdress = gpuHeapHandle;
	descriptorRange.TotalNrOfDescriptors = nrOfDescriptorsInRange * NR_OF_FRAMES;
	descriptorRange.DescriptorsPerinterval = nrOfDescriptorsInRange;
	descriptorRange.NrOfDescriptorsInUse = 0u;

	m_DescriptorRanges[rangeName] = descriptorRange;
	m_ReservedDescriptors += nrOfDescriptorsInRange * NR_OF_FRAMES;
}

RetVal DescriptorHeapShaderVisible::AddDescriptorToRange(const std::string& rangeName) noexcept
{
	DBG_ASSERT(m_DescriptorRanges.find(rangeName) != m_DescriptorRanges.end(), "Descriptor range does not exist");
	auto& range = m_DescriptorRanges[rangeName];
	DBG_ASSERT(range.NrOfDescriptorsInUse != range.DescriptorsPerinterval, "Range is full.");

	RetVal retVal = {};

	D3D12_CPU_DESCRIPTOR_HANDLE tempcpuHandle = range.CPUTailAdress;
	D3D12_GPU_DESCRIPTOR_HANDLE tempgpuHandle = range.GPUTailAdress;

	for (uint32_t i{ 0u }; i < NR_OF_FRAMES; ++i)
	{
		retVal.DstHandles[i] = tempcpuHandle;
		tempcpuHandle.ptr += range.DescriptorsPerinterval * m_IncrementSize;

		retVal.GpuHandles[i] = tempgpuHandle;
		tempgpuHandle.ptr += range.DescriptorsPerinterval * m_IncrementSize;

	}
	range.NrOfDescriptorsInUse++;
	range.CPUTailAdress.ptr += m_IncrementSize;
	range.GPUTailAdress.ptr += m_IncrementSize;

	m_DescriptorsInUse++;


	return retVal;
}
