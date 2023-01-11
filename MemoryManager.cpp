#include "pch.h"
#include "MemoryManager.h"
#include "DXCore.h"
#include "Window.h"

MemoryManager MemoryManager::s_Instance;

MemoryManager& MemoryManager::Get() noexcept
{
	return s_Instance;
}

void MemoryManager::CreateShaderVisibleDescriptorHeap(const std::string& name, uint32_t nrOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, bool setAsActive) noexcept
{
	DBG_ASSERT(m_ShaderVisibleDescriptorHeaps.find(name) == m_ShaderVisibleDescriptorHeaps.end(), "Descriptor heap name already in use.");

	m_ShaderVisibleDescriptorHeaps[name] = DescriptorHeapShaderVisible{nrOfDescriptors, descriptorHeapType};
	if (setAsActive)
	{
		switch (descriptorHeapType)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			m_pActiveShaderVisibleSRVCBVUAVDescriptorHeap = &m_ShaderVisibleDescriptorHeaps[name];
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			m_pActiveShaderVisibleRTVDescriptorHeap = &m_ShaderVisibleDescriptorHeaps[name];
			break;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			m_pActiveShaderVisibleDSVDescriptorHeap = &m_ShaderVisibleDescriptorHeaps[name];
			break;
		default:
			DBG_ASSERT(false, "Illegal Descriptor Heap Type");
			break;
		}
	}
}

void MemoryManager::CreateNonShaderVisibleDescriptorHeap(const std::string& name, uint32_t nrOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) noexcept
{
	DBG_ASSERT(m_NonShaderVisibleDescriptorHeaps.find(name) == m_NonShaderVisibleDescriptorHeaps.end(), "Descriptor heap name already in use.");
	std::array<DescriptorHeapNonShaderVisible, NR_OF_FRAMES> descriptorHeapArray;
	for (uint8_t i{ 0u }; i < NR_OF_FRAMES; ++i)
	{
		descriptorHeapArray[i] = DescriptorHeapNonShaderVisible{ nrOfDescriptors, descriptorHeapType};
	}
	m_NonShaderVisibleDescriptorHeaps[name] = std::move(descriptorHeapArray);
}

void MemoryManager::AddRangeForDescriptor(const std::string& descriptorHeapName, const std::string& rangeName, uint32_t range) noexcept
{
	DBG_ASSERT(m_ShaderVisibleDescriptorHeaps.find(descriptorHeapName) != m_ShaderVisibleDescriptorHeaps.end(), "Descriptor heap does not exist.");
	m_ShaderVisibleDescriptorHeaps[descriptorHeapName].AddRange(rangeName, range);
}

ConstantBufferView MemoryManager::CreateConstantBuffer(const std::string& descriptorHeapName, const std::string& visibleDescriptorHeapName, const std::string& rangeName, uint32_t byteWidth) noexcept
{
	DBG_ASSERT(m_NonShaderVisibleDescriptorHeaps.find(descriptorHeapName) != m_NonShaderVisibleDescriptorHeaps.end(), "Descriptor heap does not exist.");
	DBG_ASSERT(m_ShaderVisibleDescriptorHeaps.find(visibleDescriptorHeapName) != m_ShaderVisibleDescriptorHeaps.end(), "Descriptor heap does not exist.");

	ConstantBufferView constantBufferView = {};
	for (uint8_t i{ 0u }; i < NR_OF_FRAMES; ++i)
	{
		constantBufferView.pResources[i] = std::move(m_NonShaderVisibleDescriptorHeaps[descriptorHeapName][i].CreateConstantBuffer(byteWidth));
		constantBufferView.SrcHandles[i] = m_NonShaderVisibleDescriptorHeaps[descriptorHeapName][i].GetMostRecentHeapHandle();
	}

	//We now need to connect it with the visible descriptor heap:
	auto retVal = std::move(m_ShaderVisibleDescriptorHeaps[visibleDescriptorHeapName].AddDescriptorToRange(rangeName));

	for (uint8_t i{ 0u }; i < NR_OF_FRAMES; ++i)
	{
		constantBufferView.DstHandles[i] = retVal.DstHandles[i];
		constantBufferView.GpuHandles[i] = retVal.GpuHandles[i];
	}

	return constantBufferView;
}

void MemoryManager::UpdateConstantBuffer(const ConstantBufferView& constantBufferView, void* pData, uint32_t sizeOfData) noexcept
{
	auto index = Window::Get().GetCurrentFrameInFlightIndex();

	D3D12_RANGE range = { 0,0 };
	auto address = constantBufferView.pResources[index]->GetGPUVirtualAddress();

	HR(constantBufferView.pResources[index]->Map(0u, &range, reinterpret_cast<void**>(&address)));
	std::memcpy(reinterpret_cast<void*>(address), reinterpret_cast<unsigned char*>(pData), sizeOfData);
	STDCALL(constantBufferView.pResources[index]->Unmap(0u, nullptr));

	auto dstHandle = constantBufferView.DstHandles[index];

	STDCALL(DXCore::GetDevice()->CopyDescriptorsSimple(1u, dstHandle, constantBufferView.SrcHandles[index], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
}