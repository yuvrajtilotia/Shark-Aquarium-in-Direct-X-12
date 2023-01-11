#include "pch.h"
#include "Window.h"
#include "DXCore.h"
#include "Mouse.h"
#include "imgui_impl_win32.h"

Window Window::s_Instance{};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(windowHandle, message, wParam, lParam))
		return true;

	switch (message)
	{
	case WM_CLOSE:
	case WM_QUIT:
	{
		::PostQuitMessage(0);
		break;
	}
	case WM_KEYDOWN:
	{
		BOOL repeatFlag = (HIWORD(lParam) & KF_REPEAT) == KF_REPEAT;

		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		default:
		{
			if (repeatFlag == FALSE)
			{
				Keyboard::OnKeyDown(wParam);
			}
			break;
		}
		}
		break;
	}
	case WM_KEYUP:
		Keyboard::OnKeyRelease(wParam);
		break;
	case WM_INPUT:
	{
		UINT size = 0u;
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
			break;

		std::vector<char> rawBuffer;
		rawBuffer.resize(size);
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
			break;

		auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
		if (ri.header.dwType == RIM_TYPEMOUSE && (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
		{
			Mouse::OnRawDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		Mouse::OnRightButtonPressed();
		while (::ShowCursor(false) >= 0);
		break;
	}
	case WM_RBUTTONUP:
	{
		Mouse::OnRightButtonReleased();
		while (::ShowCursor(true) < 0);
		break;
	}
	}
	return ::DefWindowProc(windowHandle, message, wParam, lParam);
}

Window::Window() noexcept
	: m_WindowHandle{nullptr},
	  m_IsRunning{true},
	  m_Message{},
	  m_pSwapChain{nullptr},
	  m_FrameInFlight{ 0u }
{}

void Window::CreateWindow(const std::wstring& applicationName) noexcept
{
	std::wstring className = applicationName + L"Class";
	HINSTANCE appInstance{ ::GetModuleHandle(nullptr) };

	WNDCLASSEXW windowClass{};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbClsExtra = 0;
	windowClass.hInstance = appInstance;
	windowClass.hIcon = ::LoadIcon(appInstance, L"Icon");
	windowClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = nullptr;
	windowClass.lpszClassName = className.c_str();
	windowClass.hIconSm = ::LoadIcon(appInstance, L"Icon");
	static ATOM atom = ::RegisterClassExW(&windowClass);
	DBG_ASSERT(atom > 0, "Failed to register window class.");

	int screenWidth{ ::GetSystemMetrics(SM_CXSCREEN) };
	int screenHeight{ ::GetSystemMetrics(SM_CYSCREEN) };

	RECT windowRect{ 0, 0, static_cast<LONG>(screenWidth), static_cast<LONG>(screenHeight) };
	::AdjustWindowRect(&windowRect, WS_POPUP, FALSE);

	int windowWidth{ windowRect.right - windowRect.left };
	int windowHeight{ windowRect.bottom - windowRect.top };

	int windowCenterX{ std::max<int>(0, ((screenWidth - windowWidth) / 2)) };
	int windowCenterY{ std::max<int>(0, ((screenHeight - windowHeight) / 2)) };

	m_WindowHandle = ::CreateWindowEx
	(
		0u,
		className.c_str(),
		applicationName.c_str(),
		WS_POPUP,
		windowCenterX,
		windowCenterY,
		windowWidth,
		windowHeight,
		nullptr,
		0u,
		windowClass.hInstance,
		nullptr
	);
	DBG_ASSERT(m_WindowHandle, "Failed to create render window.");

	m_Width = windowWidth;
	m_Height = windowHeight;

	RAWINPUTDEVICE rawInputDevice;
	rawInputDevice.usUsagePage = 0x01;
	rawInputDevice.usUsage = 0x02;
	rawInputDevice.dwFlags = 0;
	rawInputDevice.hwndTarget = nullptr;
	if (RegisterRawInputDevices(&rawInputDevice, 1u, sizeof(rawInputDevice)) == FALSE)
		DBG_ASSERT(false, "Failed to register mouse device for raw input.")

	while (::ShowCursor(false) >= 0);

	::ShowWindow(m_WindowHandle, SW_SHOWNORMAL);
}

void Window::CreateSwapChain() noexcept
{
	auto pFactory{ std::move(DXCore::CreateFactory()) };
	auto pCommandQueue{ DXCore::GetCommandQueue() };
	Microsoft::WRL::ComPtr<IDXGISwapChain1> pTempSwapChain{ nullptr };

	DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
	swapChainDescriptor.Width = 0u;
	swapChainDescriptor.Height = 0u;
	swapChainDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDescriptor.Stereo = FALSE;
	swapChainDescriptor.SampleDesc = { 1u, 0u };
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.BufferCount = NR_OF_BACKBUFFERS;
	swapChainDescriptor.Scaling = DXGI_SCALING_STRETCH;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescriptor.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	HR(pFactory->CreateSwapChainForHwnd
	(
		pCommandQueue.Get(), 
		m_WindowHandle, 
		&swapChainDescriptor, 
		nullptr, 
		nullptr, 
		&pTempSwapChain
	));
	HR(pTempSwapChain->QueryInterface(__uuidof(IDXGISwapChain4), reinterpret_cast<void**>(m_pSwapChain.GetAddressOf())));
}

void Window::CreateBackBufferRTVs()
{
	for (uint32_t i{ 0u }; i < NR_OF_BACKBUFFERS; ++i)
	{
		auto cpuHandle{ m_pBackBufferRTVHeap->GetCurrentCPUOffsetHandle() };
		Microsoft::WRL::ComPtr<ID3D12Resource> pBackBuffer{ nullptr };
		HR(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer)));

		STDCALL(DXCore::GetDevice()->CreateRenderTargetView(pBackBuffer.Get(), nullptr, cpuHandle));
		m_pBackBuffers[i] = std::move(pBackBuffer);
		std::wstring bufferName{ L"BackBuffer #" + std::to_wstring(i) };
		HR(m_pBackBuffers[i]->SetName(bufferName.c_str()));
		m_pBackBufferRTVHeap->OffsetCPUAddressPointerBy(1);
	}
}

void Window::Initialize(const std::wstring& applicationName) noexcept
{
	CreateWindow(applicationName);
	m_pBackBufferRTVHeap = std::make_unique<DescriptorHeap>(NR_OF_BACKBUFFERS, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
	CreateSwapChain();
	CreateBackBufferRTVs();
}

void Window::OnUpdate() noexcept
{
	while (::PeekMessageA(&m_Message, nullptr, 0u, 0u, PM_REMOVE))
	{
		if (m_Message.message == WM_QUIT || m_Message.message == WM_CLOSE)
			m_IsRunning = false;

		::TranslateMessage(&m_Message);
		::DispatchMessageA(&m_Message);
	}
}

void Window::Present() noexcept
{
	HR(m_pSwapChain->Present(0u, DXGI_PRESENT_ALLOW_TEARING));
	//m_pSwapChain->Present(0u, DXGI_PRESENT_ALLOW_TEARING);
	m_FrameInFlight = (m_FrameInFlight + 1) % NR_OF_FRAMES;
}
