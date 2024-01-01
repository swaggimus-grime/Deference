#include "RasterPass.h"

RasterPass::RasterPass(Graphics& g, const std::string& name, FrameGraph* parent)
	:Pass(std::move(name), parent), m_DepthHeap(g), m_Depth(MakeShared<DepthStencil>(g))
{
	m_DepthHeap.Add(g, m_Depth);
	m_Viewport = MakeShared<Viewport>(g);
	AddBindable(m_Viewport);
}

void RasterPass::Run(Graphics& g)
{
	BindBindables(g);
	auto& outs = GetOutTargets();
	for (auto& out : outs)
		out.second->Clear(g);
	m_Depth->Clear(g);
	m_TargetHeap->BindWithDepth(g, *m_Depth);
	m_Pipeline->Bind(g);
	g.CL().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RasterPass::OnResize(Graphics& g, UINT w, UINT h)
{
	Pass::OnResize(g, w, h);
	m_Depth->Resize(g, w, h);
	m_Viewport->Resize(w, h);
}