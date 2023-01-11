#pragma once
class DescriptorHeap
{
public:
	DescriptorHeap() = default;
	DescriptorHeap(uint32_t capacity, D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool shaderVisible) noexcept;
	~DescriptorHeap() noexcept = default;
	[[nodiscard]] constexpr Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetInterface() noexcept { return m_pDescriptorHeap; }
	[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCPUStartHandle() noexcept { return m_CPUAdressStart; }
	[[nodiscard]] constexpr D3D12_GPU_DESCRIPTOR_HANDLE GetGPUStartHandle() noexcept { return m_GPUAdressStart; }
	[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentCPUOffsetHandle() noexcept { return m_CPUCurrentAdressOffset; }
	[[nodiscard]] constexpr uint32_t GetDescriptorTypeSize() noexcept { return m_IncrementSize; }
	void OffsetCPUAddressPointerBy(uint32_t offset) noexcept;
private:
	uint32_t m_Capacity;
	uint32_t m_Size;
	D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
	bool m_IsShaderVisible;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
	uint32_t m_IncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUAdressStart;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUCurrentAdressOffset;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUAdressStart;
};