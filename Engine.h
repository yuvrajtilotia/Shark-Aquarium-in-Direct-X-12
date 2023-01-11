#pragma once
#include "Renderer.h"

#include "Camera.h"
#include "Profiler.h"
class Engine
{
public:
	Engine() noexcept = default;
	~Engine() noexcept = default;
	void Initialize(const std::wstring& applicationName) noexcept;
	void Run() noexcept;

private:
	void CreateConsole() noexcept;
	void RenderMiscWindow(uint64_t currentFramesPerSecond, float currentFrameTime) noexcept;
private:
	std::wstring m_AppName;
	std::unique_ptr<Renderer> m_pRenderer;
	std::unique_ptr<Scene> m_pScene;
	std::unique_ptr<Camera> m_pCamera;

	double m_CurrentAverageRenderTime = 0.0f;
	double m_AverageRenderTimeSinceStart = 0.0f;
	double m_SummedDurationOverFrames = 0.0f;
};