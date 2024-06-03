#pragma once

#include "RasterPass.h"

namespace Def
{
	class VertexBuffer;
	class IndexBuffer;

	class ScreenPass : public RasterPass
	{
	public:
		ScreenPass(Graphics& g, const std::string& name, FrameGraph* parent);

	protected:
		void Rasterize(Graphics& g);
		InputLayout m_Layout;
	private:
		Unique<VertexBuffer> m_VB;
		Unique<IndexBuffer> m_IB;
	};
}