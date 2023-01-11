#include "pch.h"
#include "ImGuiManager.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "DXCore.h"

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> ImGuiManager::m_pDescriptorHeap{ nullptr };

void ImGuiManager::Initialize() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	HR(DXCore::GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeap)));

	ImGui_ImplWin32_Init(::GetActiveWindow());
	ImGui_ImplDX12_Init(DXCore::GetDevice().Get(), NR_OF_FRAMES, DXGI_FORMAT_R8G8B8A8_UNORM, m_pDescriptorHeap.Get(),
		m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiManager::OnShutDown() noexcept
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::Begin() noexcept
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End() noexcept
{
	ImGui::Render();
	STDCALL(DXCore::GetCommandList()->SetDescriptorHeaps(1, m_pDescriptorHeap.GetAddressOf()));
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DXCore::GetCommandList().Get());
}
