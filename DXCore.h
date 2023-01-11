#pragma once
class DXCore
{
public:
	DXCore() noexcept = default;
	~DXCore() noexcept = default;
	static void Initialize() noexcept;
	[[nodiscard]] static Microsoft::WRL::ComPtr<IDXGIFactory6> CreateFactory() noexcept;
	[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12Device8>& GetDevice() noexcept { return m_pDevice; }
	[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12CommandAllocator>* GetCommandAllocators() noexcept { return m_pCommandAllocators; }
	[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueue() noexcept { return m_pCommandQueue; }
	[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>& GetCommandList() noexcept { return m_pCommandList; }
	[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12Fence1>& GetFence() noexcept { return m_pFence; }
	[[nodiscard]] static constexpr HANDLE& GetFenceEvent() noexcept { return m_FenceEvent; }
	[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12Resource>& GetUploadBuffer() noexcept { return m_pUploadBuffer; }
private:
	static void CreateDebugAndGPUValidationLayer() noexcept;
	static void InitializeDevice() noexcept;
	static void InitializeCommandInterfaces() noexcept;
	static void InitializeFence() noexcept;
	static void InitializeFenceEvent() noexcept;
	static Microsoft::WRL::ComPtr<IDXGIAdapter1> CreateAdapter(const Microsoft::WRL::ComPtr<IDXGIFactory6> pFactory) noexcept;
	static void CheckSupportForDXR(Microsoft::WRL::ComPtr<ID3D12Device> pDevice) noexcept;
	static void CreateUploadHeapAndBuffer(uint32_t bufferSize) noexcept;
private:
	static Microsoft::WRL::ComPtr<ID3D12Device8> m_pDevice;
	static Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue;
	static Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCommandAllocators[NR_OF_FRAMES];
	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_pCommandList;
	static Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter;
	static Microsoft::WRL::ComPtr<ID3D12Fence1> m_pFence;
	static HANDLE m_FenceEvent;
	static Microsoft::WRL::ComPtr<ID3D12Heap> m_pUploadHeap;
	static Microsoft::WRL::ComPtr<ID3D12Resource> m_pUploadBuffer;
};

