#pragma once

#include "Window.h"

class App {
public:
	App(const std::string& name, UINT32 width, UINT32 height);
	~App();
	App(App&) = delete;
	App& operator=(const App&) = delete;
	INT Run();

private:
	Window m_Wnd;
};