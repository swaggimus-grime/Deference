#pragma once

#define NOMINMAX
#include <Windows.h>
#include "Input.h"
#include "Graphics/Graphics.h"
#include <functional>

class Window {
public:
	Window(const std::string& name, unsigned int width, unsigned int height);
	Window() = delete;
	Window(const Window&) = delete;
	~Window();

	inline void SetOnResize(std::function<void(UINT, UINT)> callback) { m_OnResize = callback; }

	std::optional<INT> Update();
	inline HWND GetHandle() const { return m_Handle; }
	inline Input& GetInput() { return m_Input; }
	inline UINT GetWidth() const { return m_Width; }
	inline UINT GetHeight() const { return m_Height; }
	static std::string OpenDialogBox();
	static std::wstring OpenDialogBoxW();
	static std::string FolderDialogBox();
	static std::wstring FolderDialogBoxW();
	void ClipCursor(BOOL clip);

private:
	static LRESULT CALLBACK SetupMessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	std::function<void(UINT, UINT)> m_OnResize;
	HWND m_Handle;
	HINSTANCE m_Inst;
	std::string m_Name;
	Input m_Input;
	UINT m_Width;
	UINT m_Height;

	RECT m_OrigClipRect;
};
