#pragma once

enum class KEY
{
	A = 65, D = 68, E = 69, Q = 81, S = 83, W = 87, R = 82
};

class Keyboard
{
public:
	static void OnKeyDown(const WPARAM key) noexcept;
	static void OnKeyRelease(const WPARAM key) noexcept;
	static const bool IsKeyDown(const KEY key) noexcept;
private:
	static std::bitset<256> m_sKeyStates;
	static const Keyboard m_sInstance;
};