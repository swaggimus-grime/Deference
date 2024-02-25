#pragma once

#include "Window.h"
#include "Graphics.h"
#include "Graphics/Scene/Camera.h"
#include "Graphics/Frame/FrameGraph.h"
#include "Graphics/Scene/Model.h"
#include <semaphore>

namespace Def
{
	class ModelLoadEvent;
	class WindowResizeEvent;

	class App {
	public:
		App(const std::string& name, UINT32 width, UINT32 height);
		~App();
		App(App&) = delete;
		App& operator=(const App&) = delete;
		INT Run();

	private:
		void RenderLoop(std::binary_semaphore& sem);
		void UpdateLoop(std::binary_semaphore& sem);
		void OnModelLoad(ModelLoadEvent& e);
		void OnResize(WindowResizeEvent& e);

	private:
		Unique<Window> m_Wnd;
		Unique<Graphics> m_Gfx;
		Unique<FrameGraph> m_Graph;

		Shared<Scene> m_Scene;

		UINT m_Width;
		UINT m_Height;
		XMFLOAT3 m_CamSpeed;
		float m_Diag;
	};
}