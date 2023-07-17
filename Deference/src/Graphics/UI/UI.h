#pragma once

#include "Graphics.h"

namespace UI
{
	void Init();
	void InitGraphics(Graphics& g);
	void Shutdown();

	void BeginFrame(Graphics& g);
	void EndFrame(Graphics& g);

}