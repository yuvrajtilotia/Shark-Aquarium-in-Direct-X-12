#pragma once
class ImGuiManager
{
public:
	ImGuiManager() noexcept = default;
	~ImGuiManager() noexcept = default;
	static void Initialize() noexcept;
	static void OnShutDown() noexcept;
	static void Begin() noexcept;
	static void End() noexcept;
private:
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
};