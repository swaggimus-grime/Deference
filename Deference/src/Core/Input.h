#pragma once

#include <bitset>
#include <queue>

class Input {
public:
	Input();
	bool IsPressed(unsigned int key);
	bool IsReleased(unsigned int key);
	std::optional<XMFLOAT2> ReadMouseDelta();
	std::optional<UINT> ReadKey();
	std::optional<XMFLOAT2> ReadMouseLPress();

	void SetCursor(BOOL enabled);
	inline void SetRawDeltaEnabled(BOOL enabled) { m_RawDeltaEnabled = enabled; }
	inline BOOL RawDeltaEnabled() const { return m_RawDeltaEnabled; }
	inline BOOL IsMouseLPressed() const { return m_LPressed; }
	inline XMFLOAT2 GetMousePos() const { return m_MousePos; }
private:
	friend class Window;
	void OnKeyPressed(unsigned int key);
	void OnKeyReleased(unsigned int key);

	void OnMouseMoved(float x, float y);
	void OnMouseDelta(float dx, float dy);
	void OnWheelDelta(float dx, float dy);
	void OnMouseLPressed();
	void OnMouseLReleased();
private:
	static constexpr UINT numKeys = 256;
	std::bitset<numKeys> m_Keys;
	std::queue<UINT> m_KeyBuffer;

	XMFLOAT2 m_MousePos;
	XMFLOAT2 m_MouseDelta;
	std::queue<XMFLOAT2> m_MouseClicks;
	std::queue<XMFLOAT2> m_MouseDeltas;
	BOOL m_RawDeltaEnabled = true;
	BOOL m_LPressed;
	BOOL m_RPressed;
};
