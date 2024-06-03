#include "RasterPass.h"

namespace Def
{
	RasterPass::RasterPass(Graphics& g, const std::string& name, FrameGraph* parent)
		:Pass(std::move(name), parent), 
		m_DepthHeap(g, 1),
		m_Depth(MakeShared<DepthStencil>(g)),
		m_Viewport(g.Width(), g.Height())
	{
		m_DepthHeap.Add(g, m_Depth);
	}

	void RasterPass::Run(Graphics& g)
	{
		m_Viewport.Bind(g);
		for (auto& rt : GetOutTargets())
			rt.second->Clear(g);
		m_Depth->Clear(g);
		m_TargetHeap->BindWithDepth(g, m_Depth.get());
		m_Pipeline->Bind(g);
	}

	void RasterPass::OnResize(Graphics& g, UINT w, UINT h)
	{
		Pass::OnResize(g, w, h);
		//m_Depth->Resize(g, w, h);
		m_Viewport.Resize(g, w, h);
	}
}