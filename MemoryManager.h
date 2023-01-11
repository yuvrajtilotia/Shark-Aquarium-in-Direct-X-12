#pragma once
#include "DescriptorHeapShaderVisible.h"
#include "DescriptorHeapNonShaderVisible.h"
class MemoryManager
{
public:
	[[nodiscard]] static MemoryManager& Get() noexcept;
	void CreateShaderVisibleDescriptorHeap(const std::string& name, uint32_t nrOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, bool setAsActive) noexcept;
	void CreateNonShaderVisibleDescriptorHeap(const std::string& name, uint32_t nrOfDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) noexcept;
	void AddRangeForDescriptor(const std::string& descriptorHeapName, const std::string& rangeName, uint32_t range) noexcept;
	[[nodiscard]] ConstantBufferView CreateConstantBuffer(const std::string& descriptorHeapName, const std::string& visibleDescriptorHeapName, const std::string& rangeName, uint32_t byteWidth) noexcept;
	void UpdateConstantBuffer(const ConstantBufferView& constantBufferView, void* pData, uint32_t sizeOfData) noexcept;
	[[nodiscard]] constexpr DescriptorHeapShaderVisible* GetActiveSRVCBVUAVDescriptorHeap() noexcept { return m_pActiveShaderVisibleSRVCBVUAVDescriptorHeap; }
private:
	MemoryManager() noexcept = default;
	~MemoryManager() noexcept = default;
private:
	static MemoryManager s_Instance;
	DescriptorHeapShaderVisible* m_pActiveShaderVisibleSRVCBVUAVDescriptorHeap{ nullptr };
	DescriptorHeapShaderVisible* m_pActiveShaderVisibleRTVDescriptorHeap{ nullptr };
	DescriptorHeapShaderVisible* m_pActiveShaderVisibleDSVDescriptorHeap{ nullptr };
	std::unordered_map<std::string, DescriptorHeapShaderVisible> m_ShaderVisibleDescriptorHeaps;
	std::unordered_map<std::string, std::array<DescriptorHeapNonShaderVisible, NR_OF_FRAMES>> m_NonShaderVisibleDescriptorHeaps;
};