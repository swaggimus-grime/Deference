#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Graphics/Scene/Camera.h"
#include "Graphics/Frame/FrameGraph.h"
#include "Graphics/Scene/Model.h"

class App {
public:
	App(const std::string& name, UINT32 width, UINT32 height);
	~App();
	App(App&) = delete;
	App& operator=(const App&) = delete;
	INT Run();

private:
	Unique<Window> m_Wnd;
	Unique<Graphics> m_Gfx;
	Unique<FrameGraph> m_Graph;

	Shared<Scene> m_Scene;

	std::function<void(Model&)> m_OnModelLoad;

	XMFLOAT3 m_CamSpeed;
	float m_Diag;
};