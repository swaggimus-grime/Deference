﻿#include "App.h"
#include <thread>
#include "UI/UI.h"
#include <semaphore>

App::App(const std::string& name, UINT32 width, UINT32 height)
{
	UI::Init();

	m_Wnd = MakeUnique<Window>(name, width, height);
	m_Gfx = MakeUnique<Graphics>(m_Wnd->GetHandle(), width, height);

	m_Cam = MakeShared<Camera>(*m_Gfx);
	m_Cup = MakeShared<Model>(*m_Gfx, "models\\void\\scene.gltf");

	m_Graph = MakeUnique<FrameGraph>();
	m_Graph->AddModel(m_Cup);
	m_Graph->SetCamera(m_Cam);
	m_Graph->FinishScene(*m_Gfx);

	{
		auto pass = MakeShared<GeometryPass>(*m_Gfx);
		m_Graph->AddPass(*m_Gfx, pass);
	}
	{
		auto pass = MakeShared<DiffusePass>(*m_Gfx);
		m_Graph->AddPass(*m_Gfx, pass);
	}
	{
		auto pass = MakeShared<AOPass>(*m_Gfx);
		m_Graph->AddPass(*m_Gfx, pass);
	}
	{
		auto pass = MakeShared<AccumPass>(*m_Gfx);
		m_Graph->AddPass(*m_Gfx, pass);
	}
	{
		auto pass = MakeShared<HybridOutPass>(*m_Gfx);
		m_Graph->AddPass(*m_Gfx, pass);
	}

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

			auto target = m_Graph->Run(g);
			target->Bind(g);
			UI::BeginFrame(g);
			m_Graph->ShowUI(g);
			UI::EndFrame(g);

			g.CopyToCurrentBB(target);
			g.EndFrame();
		}
	});

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
			cam->Move(XMFLOAT3(0.f, 0.f, deltaTime));
		if (input.IsPressed('S'))
			cam->Move(XMFLOAT3(0.f, 0.f, -deltaTime));
		if (input.IsPressed('A'))
			cam->Move(XMFLOAT3(-deltaTime, 0.f, 0.f));
		if (input.IsPressed('D'))
			cam->Move(XMFLOAT3(deltaTime, 0.f, 0.f));
		if (input.IsPressed('Q'))
			cam->Move(XMFLOAT3(0.f, deltaTime, 0.f));
		if (input.IsPressed('E'))
			cam->Move(XMFLOAT3(0.f, -deltaTime, 0.f));
		if (input.IsPressed(VK_ESCAPE))
			PostQuitMessage(0);

		while (const auto key = input.ReadKey()) {
			switch (*key) {
			case VK_TAB:
				if (m_Wnd->GetInput().RawDeltaEnabled()) {
					m_Wnd->ClipCursor(false);
					m_Wnd->GetInput().SetCursor(false);
				}
				else {
					m_Wnd->ClipCursor(true);
					m_Wnd->GetInput().SetCursor(true);
				}
				break;
			}
		}

		while (const auto delta = input.ReadMouseDelta())
			cam->Rotate((float)delta->x, (float)delta->y);

		cam->Update();
	}

	running = false;
	renderer.join();

	return *ret;
}
