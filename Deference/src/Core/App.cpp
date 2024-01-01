#include "App.h"
#include <thread>
#include "UI/UI.h"
#include <semaphore>
#include <imgui.h>

App::App(const std::string& name, UINT32 width, UINT32 height)
{
	UI::Init();

	m_Wnd = MakeUnique<Window>(name, width, height);
	m_Gfx = MakeUnique<Graphics>(m_Wnd->GetHandle(), width, height);
	m_Graph = MakeUnique<PathTraceGraph>(*m_Gfx);
	m_Scene = MakeShared<Scene>();
	m_Scene->SetCamera(MakeShared<Camera>(*m_Gfx));
	m_Scene->SetModel(MakeShared<Model>(*m_Gfx, "models\\hangar\\scene.gltf"));
	m_Graph->Compile(*m_Gfx);

	m_Wnd->SetOnResize([&](UINT w, UINT h) {
		w = std::max(w, 1u);
		h = std::max(h, 1u);
		m_Gfx->OnResize(w, h);
		m_Scene->GetCamera().OnResize(w, h);
		m_Graph->OnResize(*m_Gfx, w, h);
		UI::OnResize(*m_Gfx, w, h);
	});

	m_OnModelLoad = [&](Model& m) {
		m_Graph->LoadScene(*m_Gfx, m_Scene);
		auto bbox = m.GetBBox();
		m_CamSpeed = bbox.Dim();
		m_Diag = bbox.DiagonalLength();
	};

	m_OnModelLoad(m_Scene->GetModel());
}

App::~App()
{
}

INT App::Run()
{
	using namespace std::chrono;
	const auto startTime = steady_clock::now();
	float prevTime = 0.f;
	bool running = true;
	bool viewportActive = false;
	bool modelDirty = false;
	auto& cam = m_Scene->GetCamera();
	std::string path = "";
	std::binary_semaphore sem(1);

	std::thread renderer([&]() {
		auto& g = *m_Gfx;
		while (running) {
			sem.acquire();

			if (modelDirty)
			{
				modelDirty = false;
				m_Scene->SetModel(MakeShared<Model>(g, path));
				m_OnModelLoad(m_Scene->GetModel());
			}

			g.BeginFrame();
			cam.Update();
			auto target = m_Graph->Run(g);
			target->Bind(g);

			UI::BeginFrame(g);

			if (ImGui::BeginMainMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("Open"))
					{
						path = Window::OpenDialogBox();
						modelDirty = true;
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
				viewportActive = ImGui::IsItemActive();
				UI::DrawTarget(g, target);
			}
			ImGui::End();

			UI::EndFrame(g);

			g.CopyToCurrentBB(target);
			g.EndFrame();
		}
	});

	const float camSpeed = length(m_CamSpeed) / m_Diag;

	std::optional<INT> ret;
	while (!ret) {
		sem.release();
		ret = m_Wnd->Update();
		float currentTime = duration<float>(steady_clock::now() - startTime).count();
		float deltaTime = currentTime - prevTime;
		prevTime = currentTime;

		auto& input = m_Wnd->GetInput();
		auto& g = m_Gfx;
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
		if (input.IsPressed(VK_ESCAPE))
			PostQuitMessage(0);

		while (const auto key = input.ReadKey()) {
			switch (*key) {
			/*case VK_TAB:
				if (m_Wnd->GetInput().RawDeltaEnabled()) {
					m_Wnd->ClipCursor(false);
					m_Wnd->GetInput().SetCursor(false);
				}
				else {
					m_Wnd->ClipCursor(true);
					m_Wnd->GetInput().SetCursor(true);
				}
				break;*/
			}
		}

		while (const auto delta = input.ReadMouseDelta())
		{
			if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && viewportActive)
				cam.Rotate((float)delta->x, (float)delta->y);
		}
	}

	running = false;
	renderer.join();

	return *ret;
}
