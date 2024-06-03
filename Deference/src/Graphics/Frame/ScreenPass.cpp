#include "ScreenPass.h"

namespace Def
{
	ScreenPass::ScreenPass(Graphics& g, const std::string& name, FrameGraph* parent)
		:RasterPass(g, std::move(name), parent)
	{
		m_Layout.AddElement<VERTEX_ATTRIBUTES::POS>(DXGI_FORMAT_R32G32B32_FLOAT);
		m_Layout.AddElement<VERTEX_ATTRIBUTES::TEX_0>(DXGI_FORMAT_R32G32B32_FLOAT);
		VertexStream stream(m_Layout, 4);
		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 0) = XMFLOAT3{ -1, -1, 0 };
		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 1) = XMFLOAT3{ -1, 1, 0 };
		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 2) = XMFLOAT3{ 1, 1, 0 };
		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 3) = XMFLOAT3{ 1, -1, 0 };
		stream(Map<VERTEX_ATTRIBUTES::TEX_0>::name, 0) = XMFLOAT2{ 0, 0 };
		stream(Map<VERTEX_ATTRIBUTES::TEX_0>::name, 1) = XMFLOAT2{ 0, 1 };
		stream(Map<VERTEX_ATTRIBUTES::TEX_0>::name, 2) = XMFLOAT2{ 1, 1 };
		stream(Map<VERTEX_ATTRIBUTES::TEX_0>::name, 3) = XMFLOAT2{ 1, 0 };

		std::vector<UINT> indices = { 0, 1, 2, 2, 3, 0 };

		m_VB = MakeUnique<VertexBuffer>(g, std::move(stream));
		m_IB = MakeUnique<IndexBuffer>(g, indices.size(), indices.data());
	}

	void ScreenPass::Rasterize(Graphics& g)
	{
		m_VB->Bind(g);
		m_IB->Bind(g);
		g.CL().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g.CL().DrawIndexedInstanced(m_IB->NumIndices(), 1, 0, 0, 0);
	}

}