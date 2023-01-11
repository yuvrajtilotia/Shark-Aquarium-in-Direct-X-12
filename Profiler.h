#pragma once

struct ProfilerData
{
	std::string ContextName = "";
	double Duration = 0.0f;
};

template<typename LambdaFunction>
class Profiler
{
public:
	Profiler(const std::string& contextName, const LambdaFunction&& func) noexcept
		: m_ContextName{ contextName },
		  m_FunctionToCall{ func },
		  m_StartTime{ std::chrono::high_resolution_clock::now() }
	{
	}
	~Profiler()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto dif = std::chrono::duration_cast<std::chrono::microseconds>(end - m_StartTime);
		auto ms = static_cast<double>(dif.count());
		ms *= 0.001;

		ProfilerData profilerData;
		profilerData.ContextName = m_ContextName;
		profilerData.Duration = ms;

		m_FunctionToCall(profilerData);
	}
private:
	std::string m_ContextName;
	LambdaFunction m_FunctionToCall;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;
};

struct ProfilerManager
{
	static std::vector<ProfilerData> ProfilerDatas;
	static std::vector<ProfilerData> AllProfilerDatas;
	static double m_SummedDurationOverFrames;
	static double m_CurrentAverageDuration;
	static double m_AverageDurationSinceStart;
	static uint32_t m_MaxMeasurements;
	static uint32_t m_CurrentMeasureNr;
	static void Clear() noexcept
	{
		ProfilerDatas.clear();
		AllProfilerDatas.clear();
	}
	static std::pair<double, double> Report() noexcept
	{
		m_CurrentAverageDuration = 0.0f;
		m_AverageDurationSinceStart = 0.0f;
		m_SummedDurationOverFrames = 0u;
		for (uint32_t i{ 0u }; i < ProfilerDatas.size(); ++i)
		{
			m_CurrentAverageDuration += ProfilerDatas[i].Duration;
			AllProfilerDatas.emplace_back(ProfilerDatas[i]);
		}
		for (uint32_t i{ 0u }; i < AllProfilerDatas.size(); ++i)
		{
			m_AverageDurationSinceStart += AllProfilerDatas[i].Duration;
		}
		m_SummedDurationOverFrames = m_CurrentAverageDuration;
		m_CurrentAverageDuration /= ProfilerDatas.size();
		m_AverageDurationSinceStart /= AllProfilerDatas.size();
		m_CurrentMeasureNr++;
		ProfilerDatas.clear();
		return std::make_pair(m_CurrentAverageDuration, m_AverageDurationSinceStart);
	}
	[[nodiscard]] static constexpr bool IsValid() noexcept { return !ProfilerDatas.empty() && (m_CurrentMeasureNr != m_MaxMeasurements); }
};