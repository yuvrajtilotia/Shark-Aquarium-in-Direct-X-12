#include "pch.h"
#include "Keyboard.h"

std::bitset<256> Keyboard::m_sKeyStates{ false };

void Keyboard::OnKeyDown(const WPARAM key) noexcept
{
	m_sKeyStates[static_cast<uint32_t>(key)] = true;
}

void Keyboard::OnKeyRelease(const WPARAM key) noexcept
{
	m_sKeyStates[static_cast<uint32_t>(key)] = false;
}

const bool Keyboard::IsKeyDown(const KEY key) noexcept
{
	return m_sKeyStates[static_cast<uint32_t>(key)];
}
