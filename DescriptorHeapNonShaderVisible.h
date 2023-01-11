#pragma once
class DescriptorHeapNonShaderVisible
{
public:
	DescriptorHeapNonShaderVisible() noexcept = default;
	DescriptorHeapNonShaderVisible(uint32_t capacity, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) noexcept;
	~DescriptorHeapNonShaderVisible() noexcept = default;
	[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource> CreateConstantBuffer(uint32_t byteWidth) noexcept;
	[[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetMostRecentHeapHandle() noexcept { return m_MostRecentHandle; }
private:
	uint32_t m_Capacity;
	uint32_t m_NrOfDescriptors;
	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
	uint32_t m_IncrementSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE m_HeadAdress;
	D3D12_CPU_DESCRIPTOR_HANDLE m_TailAdress;
	D3D12_CPU_DESCRIPTOR_HANDLE m_MostRecentHandle;
};