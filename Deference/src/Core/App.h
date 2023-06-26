#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Graphics/Entity/Camera.h"
#include "Graphics/Frame/LambertianGraph.h"
#include "Graphics/Entity/Model.h"
#include "Graphics/Bindable/VertexBuffer.h"

class App {
public:
	App(const std::string& name, UINT32 width, UINT32 height);
	~App();
	App(App&) = delete;
	App& operator=(const App&) = delete;
	INT Run();

private:
	Window m_Wnd;
	Graphics m_Gfx;
	Shared<Camera> m_Cam;
	Unique<LambertianGraph> m_Graph;
	Model m_Cup;
};