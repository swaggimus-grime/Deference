//#include "ScreenPass.h"
//
//namespace Def
//{
//	ScreenPass::ScreenPass(Graphics& g, const std::string& name, FrameGraph* parent)
//		:RasterPass(g, std::move(name), parent)
//	{
//		InputLayout layout;
//		layout.AddElement<VERTEX_ATTRIBUTES::POS>(DXGI_FORMAT_R32G32B32_FLOAT);
//		layout.AddElement<VERTEX_ATTRIBUTES::TEX>(DXGI_FORMAT_R32G32B32_FLOAT);
//		VertexStream stream(std::move(layout), 4);
//		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 0) = XMFLOAT3{ -1, -1, 0 };
//		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 1) = XMFLOAT3{ -1, 1, 0 };
//		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 2) = XMFLOAT3{ 1, 1, 0 };
//		stream(Map<VERTEX_ATTRIBUTES::POS>::name, 3) = XMFLOAT3{ 1, -1, 0 };
//		stream(Map<VERTEX_ATTRIBUTES::TEX>::name, 0) = XMFLOAT2{ 0, 0 };
//		stream(Map<VERTEX_ATTRIBUTES::TEX>::name, 1) = XMFLOAT2{ 0, 1 };
//		stream(Map<VERTEX_ATTRIBUTES::TEX>::name, 2) = XMFLOAT2{ 1, 1 };
//		stream(Map<VERTEX_ATTRIBUTES::TEX>::name, 3) = XMFLOAT2{ 1, 0 };
//
//		std::vector<UINT> indices = { 0, 1, 2, 2, 3, 0 };
//		m_NumIndices = indices.size();
//
//		AddBindable(MakeShared<VertexBuffer>(g, std::move(stream)));
//		AddBindable(MakeShared<IndexBuffer>(g, indices.size(), indices.data()));
//	}
//
//	void ScreenPass::Rasterize(Graphics& g)
//	{
//		g.CL().DrawIndexedInstanced(m_NumIndices, 1, 0, 0, 0);
//	}
//
//}