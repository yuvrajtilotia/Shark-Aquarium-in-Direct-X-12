#pragma once
class Mouse
{
public:
	static void OnRawDelta(const int dx, const int dy) noexcept;
	static const std::pair<int, int> GetMovementDelta() noexcept;
	static void OnRightButtonPressed() noexcept;
	static void OnRightButtonReleased() noexcept;
	[[nodiscard]] static constexpr bool IsRightButtonPressed() noexcept { return m_sRightButtonPressed; }
private:
	static int m_sDeltaX;
	static int m_sDeltaY;
	static bool m_sRightButtonPressed;
};