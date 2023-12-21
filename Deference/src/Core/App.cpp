#include "App.h"
#include <thread>
#include "UI/UI.h"
#include <semaphore>

App::App(const std::string& name, UINT32 width, UINT32 height)
{
	UI::Init();

	m_Wnd = MakeUnique<Window>(name, width, height);
	m_Gfx = MakeUnique<Graphics>(m_Wnd->GetHandle(), width, height);

	m_Cam = MakeShared<Camera>(*m_Gfx);
	m_Cup = MakeShared<Model>(*m_Gfx, "models\\hangar\\scene.gltf");

	Scene scene;
	scene.m_Camera = m_Cam->Share();
	scene.m_Models.push_back(m_Cup);
	scene.m_BBox = m_Cup->GetBBox();

	m_CamSpeed = scene.m_BBox.Dim();
	m_Diag = scene.m_BBox.DiagonalLength();

	//m_Cam->SetMoveSpeed(m_Diag);

	m_Graph = MakeUnique<PathTraceGraph>(*m_Gfx, scene);

	m_Wnd->SetOnResize([&](UINT w, UINT h) {
		w = std::max(w, 1u);
		h = std::max(h, 1u);
		m_Gfx->OnResize(w, h);
		m_Cam->OnResize(w, h);
		m_Graph->OnResize(*m_Gfx, w, h);
	});

	m_Gfx->Flush();
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

	std::binary_semaphore sem(0);

	std::thread renderer([&]() {
		auto& g = *m_Gfx;
		while (running) {
			sem.acquire();
			g.BeginFrame();


			m_Cam->Update();
			auto target = m_Graph->Run(g);
			target->Bind(g);
			UI::BeginFrame(g);
			m_Cam->ShowUI();
			m_Graph->ShowUI(g);
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
		auto& cam = m_Cam;
		if (input.IsPressed('W'))
			cam->Move(XMFLOAT3(0.f, 0.f, deltaTime * m_Diag));
		if (input.IsPressed('S'))
			cam->Move(XMFLOAT3(0.f, 0.f, -deltaTime * m_Diag));
		if (input.IsPressed('A'))
			cam->Move(XMFLOAT3(-deltaTime * m_Diag, 0.f, 0.f));
		if (input.IsPressed('D'))
			cam->Move(XMFLOAT3(deltaTime * m_Diag, 0.f, 0.f));
		if (input.IsPressed('Q'))
			cam->Move(XMFLOAT3(0.f, deltaTime * m_Diag, 0.f));
		if (input.IsPressed('E'))
			cam->Move(XMFLOAT3(0.f, -deltaTime * m_Diag, 0.f));
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
			if(input.IsMouseLPressed())
				cam->Rotate((float)delta->x, (float)delta->y);
		}
	}

	running = false;
	renderer.join();

	return *ret;
}
