﻿#include "App.h"
#include <thread>

App::App(const std::string& name, UINT32 width, UINT32 height)
	:m_Wnd(name, width, height), 
	m_Gfx(m_Wnd.GetHandle(), width, height)
{
	m_Cam = MakeShared<Camera>(m_Gfx);
	m_Cup = MakeUnique<Model>(m_Gfx, "models\\temple\\scene.gltf");

	m_Graph = MakeUnique<GeometryGraph>();
	m_Graph->AddGeometry(*m_Cup);
	m_Graph->SetCamera(m_Cam);

	{
		auto pass = MakeShared<GeometryPass>(m_Gfx);
		m_Graph->AddPass(m_Gfx, pass);
	}
	{
		auto pass = MakeShared<DiffusePass>(m_Gfx);
		m_Graph->AddPass(m_Gfx, pass);
	}
	{
		auto pass = MakeShared<AOPass>(m_Gfx);
		m_Graph->AddPass(m_Gfx, pass);
	}
	{
		auto pass = MakeShared<AccumPass>(m_Gfx);
		m_Graph->AddPass(m_Gfx, pass);
	}
	{
		auto pass = MakeShared<HybridOutPass>(m_Gfx);
		m_Graph->AddPass(m_Gfx, pass);
	}
	
	m_Gfx.Flush();
}

App::~App()
{
}

INT App::Run()
{
	std::optional<INT> ret;
	using namespace std::chrono;
	const auto startTime = steady_clock::now();
	float prevTime = 0.f;
	bool running = true;

	std::thread renderer([&]() {
		auto& g = m_Gfx;
		while (running) {
			g.BeginFrame();
			m_Graph->Run(g);
			g.CopyToCurrentBB(m_Graph->Run(g));
			g.EndFrame();
		}
	});

	while (!(ret = m_Wnd.Update())) {
		float currentTime = duration<float>(steady_clock::now() - startTime).count();
		float deltaTime = currentTime - prevTime;
		prevTime = currentTime;

		auto& input = m_Wnd.GetInput();
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
				if (m_Wnd.GetInput().RawDeltaEnabled()) {
					m_Wnd.ClipCursor(false);
					m_Wnd.GetInput().SetCursor(false);
				}
				else {
					m_Wnd.ClipCursor(true);
					m_Wnd.GetInput().SetCursor(true);
				}
				break;
			}
		}

		while (const auto delta = input.ReadMouseDelta())
			cam->Rotate((float)delta->x, (float)delta->y);
	}

	running = false;
	renderer.join();

	return *ret;
}
