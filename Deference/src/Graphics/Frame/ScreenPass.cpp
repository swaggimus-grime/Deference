#include "ScreenPass.h"

ScreenPass::ScreenPass(Graphics& g)
	:RasterPass(g)
{
	InputLayout layout(INPUT_LAYOUT_CONFIG::SCREEN);
	VertexStream stream(std::move(layout), 4);
	stream.Pos(0) = { -1, -1, 0 };
	stream.Pos(1) = { -1, 1, 0 };
	stream.Pos(2) = { 1, 1, 0 };
	stream.Pos(3) = { 1, -1, 0 };
	stream.Tex(0) = { 0, 0 };
	stream.Tex(1) = { 0, 1 };
	stream.Tex(2) = { 1, 1 };
	stream.Tex(3) = { 1, 0 };

	std::vector<UINT> indices = { 0, 1, 2, 2, 3, 0 };
	m_NumIndices = indices.size();

	AddBindable(MakeShared<VertexBuffer>(g, std::move(stream)));
	AddBindable(MakeShared<IndexBuffer>(g, indices.size(), indices.data()));
}

void ScreenPass::Rasterize(Graphics& g)
{
	g.CL().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g.CL().DrawIndexedInstanced(m_NumIndices, 1, 0, 0, 0);
}
