#include "App.h"
#include <thread>
#include "UI/UI.h"
#include <imgui.h>
#include "Graphics/Frame/PathTraceGraph.h"
#include <filesystem>
#include "Event/ModelEvent.h"
#include "Event/WindowEvent.h"

namespace Def
{
	App::App(const std::string& name, UINT32 width, UINT32 height)
	{
		UI::Init();

		m_Wnd = MakeUnique<Window>(name, width, height);
		m_Gfx = MakeUnique<Graphics>(m_Wnd->GetHandle(), width, height);
		m_Graph = MakeUnique<PathTraceGraph>(*m_Gfx);

		m_Scene = MakeShared<Scene>();
		m_Scene->m_Camera = MakeShared<Camera>(*m_Gfx);
		m_Scene->m_Model = MakeShared<Model>(*m_Gfx, "res\\models\\DamagedHelmet2.glb");
		m_Graph->Compile(*m_Gfx);

		EventManager::Bind<ModelLoadEvent>(EVENT_BIND(App::OnModelLoad));
		EventManager::Bind<WindowResizeEvent>(EVENT_BIND(App::OnResize));

		EventManager::Execute<ModelLoadEvent>();
	}

	void App::OnModelLoad(ModelLoadEvent& e)
	{
		m_Graph->LoadScene(*m_Gfx, m_Scene);
		auto bbox = m_Scene->m_Model->GetBBox();
		m_CamSpeed = bbox.Dim();
		m_Diag = bbox.DiagonalLength();
	}

	void App::OnResize(WindowResizeEvent& e)
	{
		m_Gfx->OnResize(m_Width, m_Height);
		m_Scene->m_Camera->OnResize(m_Width, m_Height);
		m_Graph->OnResize(*m_Gfx, m_Width, m_Height);
		UI::OnResize(*m_Gfx, m_Width, m_Height);
	}

	App::~App()
	{
	}

	INT App::Run()
	{
		std::binary_semaphore sem(1);
		std::thread renderer(&App::RenderLoop, this, std::ref(sem));
		
		using namespace std::chrono;
		const auto startTime = steady_clock::now();
		float prevTime = 0.f;

		while (m_Wnd->Update()) {
			sem.release();
			float currentTime = duration<float>(steady_clock::now() - startTime).count();
			float deltaTime = currentTime - prevTime;
			prevTime = currentTime;

			auto& cam = *m_Scene->m_Camera;
			auto& input = m_Wnd->GetInput();
			if (input.IsPressed('W'))
				cam.Move(XMFLOAT3(0.f, 0.f, deltaTime * m_Diag));
			if (input.IsPressed('S'))
				cam.Move(XMFLOAT3(0.f, 0.f, -deltaTime * m_Diag));
			if (input.IsPressed('A'))
				cam.Move(XMFLOAT3(-deltaTime * m_Diag, 0.f, 0.f));
			if (input.IsPressed('D'))
				cam.Move(XMFLOAT3(deltaTime * m_Diag, 0.f, 0.f));
			if (input.IsPressed('Q'))
				cam.Move(XMFLOAT3(0.f, deltaTime * m_Diag, 0.f));
			if (input.IsPressed('E'))
				cam.Move(XMFLOAT3(0.f, -deltaTime * m_Diag, 0.f));

			while (const auto key = input.ReadKey()) {
				switch (*key) {
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;
				}
			}

			while (const auto delta = input.ReadMouseDelta())
			{
				if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && UI::IsViewportActive())
					cam.Rotate((float)delta->x, (float)delta->y);
			}
		}
		
		renderer.join();

		return m_Wnd->GetExitCode();
	}

	void App::RenderLoop(std::binary_semaphore& sem)
	{
		auto& g = *m_Gfx;
		auto& cam = *m_Scene->m_Camera;
		while (!m_Wnd->HasQuit()) {
			sem.acquire();
			g.BeginFrame();
			cam.Update();
			auto target = m_Graph->Run(g);
			target->Bind(g);

			UI::BeginFrame(g);

			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("Open"))
					{
						auto path = Window::OpenDialogBox();
						if (!path.empty())
						{
							m_Scene->m_Model = MakeShared<Model>(*m_Gfx, path);
							EventManager::Execute<ModelLoadEvent>();
						}
					}

					if (ImGui::MenuItem("Exit"))
						PostQuitMessage(0);

					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}

			m_Graph->ShowUI(g);
			cam.ShowUI();

			if (ImGui::Begin("Viewport"))
			{
				UI::SetViewportActive(ImGui::IsItemActive());
				UI::DrawTarget(g, target);
			}
			ImGui::End();

			UI::EndFrame(g);

			g.CopyToCurrentBB(target);
			g.EndFrame();
		}
	}

	void App::UpdateLoop(std::binary_semaphore& sem)
	{
		
	}

}