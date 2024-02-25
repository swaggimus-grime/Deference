//#pragma once
//
//#include "Pass.h"
//#include "Resource/Viewport.h"
//
//namespace Def
//{
//	class RasterPass : public Pass
//	{
//	public:
//		virtual void Run(Graphics& g) override;
//		virtual void OnResize(Graphics& g, UINT w, UINT h) override;
//
//	protected:
//		RasterPass(Graphics& g, const std::string& name, FrameGraph* parent);
//		Unique<Pipeline> m_Pipeline;
//
//	private:
//		DepthStencilHeap m_DepthHeap;
//		Shared<DepthStencil> m_Depth;
//		Viewport m_Viewport;
//	};
//}