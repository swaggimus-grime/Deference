#pragma once

#include "Graphics.h"

namespace Def
{

	namespace UI
	{
		void Init();
		void InitGraphics(Graphics& g);
		void Shutdown();

		void OnResize(Graphics& g, UINT width, UINT height);
		void BeginFrame(Graphics& g);
		void EndFrame(Graphics& g);
		void DrawTarget(Graphics& g, Shared<Target> target);

		void SetViewportActive(bool active);
		bool IsViewportActive();
	}
}