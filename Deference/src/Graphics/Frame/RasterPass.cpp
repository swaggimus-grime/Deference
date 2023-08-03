#include "RasterPass.h"

RasterPass::RasterPass(Graphics& g)
	:m_DepthHeap(g), m_Depth(g)
{
	m_Depth.CreateView(g, m_DepthHeap.Next());
}

void RasterPass::Run(Graphics& g)
{
	BindBindables(g);

	for (auto& out : GetOutTargets())
		out.second->Clear(g);
	m_Depth.Clear(g);

	m_RTs->BindWithDepth(g, m_Depth);

	m_Pipeline->Bind(g);

	g.CL().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RasterPass::OnResize(Graphics& g, UINT w, UINT h)
{
	Pass::OnResize(g, w, h);
	m_Depth.Resize(g, w, h);
}