#include "pch.h"
#include "App.h"

#include <thread>

App::App(const std::string& name, UINT32 width, UINT32 height)
	:m_Wnd(name, width, height)
{
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
		auto& g = m_Wnd.GetGraphics();
		g.Render();
	});

	while (!(ret = m_Wnd.Update())) {
		float currentTime = duration<float>(steady_clock::now() - startTime).count();
		float deltaTime = currentTime - prevTime;
		prevTime = currentTime;
	}

	return *ret;
}
