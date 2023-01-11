#pragma once

struct DescriptorRange
{
	D3D12_CPU_DESCRIPTOR_HANDLE CPUHeadAdress;
	D3D12_CPU_DESCRIPTOR_HANDLE CPUTailAdress;
	D3D12_GPU_DESCRIPTOR_HANDLE GPUHeadAdress;
	D3D12_GPU_DESCRIPTOR_HANDLE GPUTailAdress;
	uint32_t TotalNrOfDescriptors;
	uint32_t DescriptorsPerinterval;
	uint32_t NrOfDescriptorsInUse;
};

struct RetVal
{
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, NR_OF_FRAMES> DstHandles;
	std::array<D3D12_GPU_DESCRIPTOR_HANDLE, NR_OF_FRAMES> GpuHandles;
};

class DescriptorHeapShaderVisible
{
public:
	DescriptorHeapShaderVisible() = default;
	DescriptorHeapShaderVisible(uint32_t capacity, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) noexcept;
	~DescriptorHeapShaderVisible() noexcept = default;
	void AddRange(std::string rangeName, uint32_t nrOfDescriptorsInRange) noexcept;
	[[nodiscard]] RetVal AddDescriptorToRange(const std::string& rangeName) noexcept;
	[[nodiscard]] constexpr Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetInterface() noexcept { return m_pDescriptorHeap; }
private:
	uint32_t m_Capacity;
	uint32_t m_DescriptorsInUse;
	uint32_t m_ReservedDescriptors;
	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
	uint32_t m_IncrementSize;
	std::unordered_map<std::string, DescriptorRange> m_DescriptorRanges;
};