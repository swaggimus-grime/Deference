#include "RasterPass.h"

RasterPass::RasterPass(Graphics& g)
	:m_DepthHeap(g), m_Depth(g)
{
	m_Depth.CreateView(g, m_DepthHeap.Next());
}

void RasterPass::OnResize(Graphics& g, UINT w, UINT h)
{
	Pass::OnResize(g, w, h);
	m_Depth.Resize(g, w, h);
}