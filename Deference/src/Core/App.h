#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Graphics/Entity/Camera.h"
#include "Graphics/Frame/FrameGraph.h"
#include "Graphics/Entity/Model.h"

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
	Shared<Camera> m_Cam;
	Unique<FrameGraph> m_Graph;
	Shared<Model> m_Cup;
};