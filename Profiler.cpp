#include "pch.h"
#include "Profiler.h"

std::vector<ProfilerData> ProfilerManager::ProfilerDatas;
std::vector<ProfilerData> ProfilerManager::AllProfilerDatas;
double ProfilerManager::m_CurrentAverageDuration{ 0.0f };
double ProfilerManager::m_AverageDurationSinceStart{ 0.0f };
double ProfilerManager::m_SummedDurationOverFrames{ 0.0f };
uint32_t ProfilerManager::m_MaxMeasurements{ 1u };
uint32_t ProfilerManager::m_CurrentMeasureNr{ 0u };