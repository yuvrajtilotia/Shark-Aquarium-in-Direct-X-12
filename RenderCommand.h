#pragma once
class Renderer;
class RenderCommand
{
public:
	static void TransitionResource(const Microsoft::WRL::ComPtr<ID3D12Resource> pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter) noexcept;
	static void ClearRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorhandle, const DirectX::XMVECTORF32& clearColor) noexcept;
	static void ClearDepth(D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorhandle, FLOAT depthValue) noexcept;
	static void WaitForFenceValue(uint64_t fenceValue) noexcept;
	static void Flush() noexcept;
	static Renderer* s_Renderer;
private:
	RenderCommand() noexcept = default;
	~RenderCommand() noexcept = default;
private:
	static uint64_t s_FenceValue;
};