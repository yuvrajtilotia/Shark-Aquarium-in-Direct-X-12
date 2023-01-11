#include "pch.h"
#include "Engine.h"
#include "DXCore.h"
#include "Window.h"
#include "ImGuiManager.h"
#define USE_PIX
#include "pix3.h"

void Engine::Initialize(const std::wstring& applicationName) noexcept
{
	CreateConsole();
	DXCore::Initialize();
	Window::Get().Initialize(applicationName);
	ImGuiManager::Initialize();

	auto& memoryManager = MemoryManager::Get();
	memoryManager.CreateShaderVisibleDescriptorHeap("ShaderBindables", 100'000, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
	memoryManager.AddRangeForDescriptor("ShaderBindables", "TransformsRange", 100'000);
	memoryManager.CreateNonShaderVisibleDescriptorHeap("Transforms", 100'000, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	m_pRenderer = std::make_unique<Renderer>();
	m_pRenderer->Initialize();
	m_pScene = std::make_unique<Scene>();
	m_pScene->Initialize();
	DirectX::XMFLOAT3 cameraStartPosition = DirectX::XMFLOAT3(0.0f, 0.0f, -5.0f);
	auto [width, height] = Window::Get().GetDimensions();
	m_pCamera = std::make_unique<Camera>(cameraStartPosition, width, height);
}

void Engine::Run() noexcept
{
	static Window& s_Window{ Window::Get() };

	float deltaTime = 0.0f;
	auto lastFrameEnd = std::chrono::system_clock::now();
	uint64_t frameCount = 0u;
	uint32_t framesPerSecond = 0u;
	uint32_t currentFramesPerSecond = 0u;
	float currentFrameTime = 0.0f;
	float secondTracker = 0.0f;
	bool startProfiling = false;
	m_pRenderer->WaitForGpu();
	HR(DXCore::GetCommandList()->Close());
	while (s_Window.IsRunning())
	{
		{
			Profiler profiler("Render loop", [&](ProfilerData profilerData) {ProfilerManager::ProfilerDatas.emplace_back(profilerData); });

			m_pScene->Update(m_pCamera->GetRayTraceBool(), deltaTime);
			m_pRenderer->Begin(m_pCamera.get(), m_pScene->GetAccelerationStructureGPUAddress());
			m_pRenderer->Submit(m_pScene->GetCulledVertexObjects());

			{
				//ImGuiManager::Begin();
				//
				//RenderMiscWindow(currentFramesPerSecond, currentFrameTime);
				//
				//ImGuiManager::End();
			}
			m_pRenderer->End();
		}

		m_pCamera->Update(deltaTime);
		s_Window.OnUpdate();

		auto currentFrameEnd = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentFrameEnd - lastFrameEnd).count();
		deltaTime = static_cast<float>(elapsed) / 1'000'000.0f;
		currentFrameTime = static_cast<float>(elapsed) / 1'000;
		lastFrameEnd = currentFrameEnd;

		secondTracker += deltaTime;
		if (secondTracker >= 1.0f)
		{
			secondTracker = 0.0f;
			currentFramesPerSecond = framesPerSecond;
			framesPerSecond = 0;
		}


		frameCount++;
		if (frameCount == 2'000 && startProfiling == false)
		{
			startProfiling = true;
			frameCount = 0u;
		}
		else if (frameCount == 500 && startProfiling == true)
		{
			if (ProfilerManager::IsValid())
			{
				auto [currentAverage, averageSinceStart] = ProfilerManager::Report();
				m_CurrentAverageRenderTime = currentAverage;
				m_AverageRenderTimeSinceStart = averageSinceStart;
				m_SummedDurationOverFrames = ProfilerManager::m_SummedDurationOverFrames;
			}
			frameCount = 0u;
		}

		framesPerSecond++;
	}
	m_pRenderer->OnShutDown();
	ImGuiManager::OnShutDown();
}

void Engine::CreateConsole() noexcept
{
	AllocConsole() && "Unable to allocate console";
	DBG_ASSERT(freopen("CONIN$", "r", stdin), "Could not freopen stdin.");
	DBG_ASSERT(freopen("CONOUT$", "w", stdout), "Could not freopen stdout.");
	DBG_ASSERT(freopen("CONOUT$", "w", stderr), "Could not freopen stderr.");
}

void Engine::RenderMiscWindow(uint64_t currentFramesPerSecond, float currentFrameTime) noexcept
{
	ImGui::Begin("Miscellaneous");
	static float cameraSpeed = 40.0f;
	if (ImGui::DragFloat("Camera Speed", &cameraSpeed, 1.0f, 1.0f, 100.0f))
		m_pCamera->SetCameraSpeed(cameraSpeed);
	ImGui::Text("Frame rate: %d", currentFramesPerSecond);
	ImGui::Text("Frame time: %.5f ms", currentFrameTime);
	ImGui::Text("Render pass time (Average): %.5f ms", m_CurrentAverageRenderTime);
	ImGui::Text("Render pass time (Total summed average): %.5f ms", m_AverageRenderTimeSinceStart);
	ImGui::Text("Summed duration over test: %.5f ms", m_SummedDurationOverFrames);
	ImGui::Text("Mesh Count: %d", m_pScene->GetTotalNrOfMeshes());
	ImGui::Text("Vertex Count: %d", m_pScene->GetTotalNrOfVertices());
	ImGui::Text("Index Count: %d", m_pScene->GetTotalNrOfIndices());
	ImGui::End();
}
