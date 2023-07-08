#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Graphics/Entity/Camera.h"
#include "Graphics/Frame/GeometryGraph.h"
#include "Graphics/Entity/Model.h"

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
	Unique<GeometryGraph> m_Graph;
	Unique<Model> m_Cup; 
};