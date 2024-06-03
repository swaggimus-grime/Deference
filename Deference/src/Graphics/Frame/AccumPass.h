#pragma once

#include "ScreenPass.h"

namespace Def
{
	class AccumPass : public ScreenPass
	{
	public:
		AccumPass(Graphics& g, const std::string& name, FrameGraph* parent);
		virtual void Run(Graphics& g) override;
		virtual void ShowGUI() override;
		virtual void OnResize(Graphics& g, UINT w, UINT h) override;

	private:
		UINT m_NumPassedFrames;
		Shared<RenderTarget> m_PrevFrame;
	};
}