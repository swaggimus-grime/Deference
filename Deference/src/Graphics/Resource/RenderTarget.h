#pragma once

#include "Target.h"
#include "DepthStencil.h"
#include "Swapchain.h"

namespace Def
{
	class RenderTarget : public Target, public RTV
	{
	public:
		RenderTarget(Graphics& g, DXGI_FORMAT fmt = Swapchain::s_Format, XMFLOAT4 clearColor = {0.f, 0.f, 0.f, 1.f});
		RenderTarget(const ComPtr<ID3D12Resource>& res);
		virtual const D3D12_RENDER_TARGET_VIEW_DESC& RTVDesc() const override;
		virtual void Clear(Graphics& g) const override;
		virtual void Resize(Graphics& g, UINT w, UINT h) override;
		virtual void Bind(Graphics& g) override;
		void BindWithDepth(Graphics& g, DepthStencil* ds);

	private:
		XMFLOAT4 m_ClearColor;
	};
}