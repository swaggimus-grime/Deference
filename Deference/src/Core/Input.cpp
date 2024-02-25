#include "Input.h"
#include "Debug/Exception.h"

namespace Def
{
	Input::Input()
		:m_LPressed(false), m_RPressed(false)
	{
		RAWINPUTDEVICE mouse{};
		mouse.usUsagePage = 1;
		mouse.usUsage = 2;
		BR(RegisterRawInputDevices(&mouse, 1, sizeof(mouse)));

		SetCursor(true);
	}

	void Input::OnKeyPressed(unsigned int key)
	{
		m_Keys[key] = true;
		m_KeyBuffer.push(key);
	}

	void Input::OnKeyReleased(unsigned int key)
	{
		m_Keys[key] = false;
	}

	void Input::OnMouseMoved(float x, float y)
	{
		m_MousePos.x = x;
		m_MousePos.y = y;
	}

	void Input::OnMouseDelta(float dx, float dy)
	{
		m_MouseDeltas.emplace(dx, dy);
	}

	void Input::OnWheelDelta(float dx, float dy)
	{

	}

	void Input::OnMouseLPressed()
	{
		m_LPressed = true;
		m_MouseClicks.push(m_MousePos);
	}

	void Input::OnMouseLReleased()
	{
		m_LPressed = false;
	}

	bool Input::IsPressed(unsigned int key)
	{
		return m_Keys[key] & 1;
	}

	bool Input::IsReleased(unsigned int key)
	{
		return !(m_Keys[key] & 1);
	}

	std::optional<XMFLOAT2> Input::ReadMouseDelta()
	{
		if (m_MouseDeltas.empty())
			return {};

		const auto delta = m_MouseDeltas.front();
		m_MouseDeltas.pop();
		return delta;
	}

	std::optional<UINT> Input::ReadKey()
	{
		if (m_KeyBuffer.empty())
			return {};

		UINT key = m_KeyBuffer.front();
		m_KeyBuffer.pop();
		return key;
	}

	std::optional<XMFLOAT2> Input::ReadMouseLPress()
	{
		if (m_MouseClicks.empty())
			return {};

		XMFLOAT2 pos = m_MouseClicks.front();
		m_MouseClicks.pop();
		return pos;
	}

	void Input::SetCursor(BOOL enabled)
	{
		SetRawDeltaEnabled(enabled);
		ShowCursor(!enabled);
		//if (!enabled)
		//	Gui::EnableMouse();
		//else
		//	Gui::DisableMouse();
	}
}